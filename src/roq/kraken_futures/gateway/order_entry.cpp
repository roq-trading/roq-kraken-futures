/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kraken_futures/gateway/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/server/oms/exceptions.hpp"

#include "roq/kraken_futures/json/cancel_all_after_ack.hpp"
#include "roq/kraken_futures/json/cancel_all_orders.hpp"
#include "roq/kraken_futures/json/cancel_order.hpp"
#include "roq/kraken_futures/json/edit_order.hpp"
#include "roq/kraken_futures/json/rest_error.hpp"
#include "roq/kraken_futures/json/result.hpp"
#include "roq/kraken_futures/json/send_order.hpp"

#include "roq/kraken_futures/json/encoder.hpp"
#include "roq/kraken_futures/json/map.hpp"
#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace gateway {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
    SupportType::ORDER,
};

size_t const MAX_DECODE_BUFFER_DEPTH = 1;
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.rest.uri;
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .proxy = settings.rest.proxy,
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // request
      .allow_pipelining = true,
      .request_timeout = settings.rest.request_timeout,
      // response
      .suspend_on_retry_after = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = settings.rest.ping_freq,
      .ping_path = settings.rest.ping_path,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::rest::Client::create(handler, context, config);
}

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

// following is used from several places

auto get_quality_of_service(auto &settings) {
  return settings.rest.allow_order_request_pipeline ? io::QualityOfService::IMMEDIATE : io::QualityOfService::CRITICAL;
}

auto compute_order_event_type(auto &order_events) {
  if (std::empty(order_events)) {
    throw RuntimeError{"Unexpected"sv};
  }
  json::OrderEventType result = json::OrderEventType::type_t{};
  for (auto &item : order_events) {
    if (result == json::OrderEventType::type_t{}) {
      result = item.type;
    } else if (item.type != result) {
      throw RuntimeError{R"(Unexpected: type="{}")"sv, item};
    }
  }
  if (result == json::OrderEventType::type_t{}) {
    throw RuntimeError{"Unexpected"sv};
  }
  return result;
}

auto compute_order_status(auto status, auto remaining_quantity) {
  if (utils::is_zero(remaining_quantity)) {
    return OrderStatus::COMPLETED;
  }
  return map(status).template get<OrderStatus>();
}
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_{shared.settings.misc.decode_buffer_size, MAX_DECODE_BUFFER_DEPTH},
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .create_order = create_metrics(shared.settings, name_, "create_order"sv),
          .create_order_ack = create_metrics(shared.settings, name_, "create_order_ack"sv),
          .modify_order = create_metrics(shared.settings, name_, "modify_order"sv),
          .modify_order_ack = create_metrics(shared.settings, name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(shared.settings, name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(shared.settings, name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(shared.settings, name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(shared.settings, name_, "cancel_all_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.rest.request_timeout, [this](auto state) { return download(state); }} {
}

void OrderEntry::operator()(Event<Start> const &) {
  (*connection_).start();
}

void OrderEntry::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void OrderEntry::operator()(Event<Timer> const &event) {
  auto now = event.value.now;
  (*connection_).refresh(now);
  if (shared_.settings.rest.cancel_on_disconnect && shared_.settings.rest.cancel_all_after.count() != 0 && ready() && next_cancel_all_timer_ < now) {
    next_cancel_all_timer_ = now + shared_.settings.rest.cancel_all_after / 4;  // note! update 4x per period
    cancel_all_orders_after(shared_.settings.rest.cancel_all_after);
  }
}

void OrderEntry::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.create_order, metrics::Type::PROFILE)
      .write(profile_.create_order_ack, metrics::Type::PROFILE)
      .write(profile_.modify_order, metrics::Type::PROFILE)
      .write(profile_.modify_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_order, metrics::Type::PROFILE)
      .write(profile_.cancel_order_ack, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders, metrics::Type::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY);
}

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  create_order(event, order, ref_data, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  modify_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, ref_data, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void OrderEntry::operator()(ConnectionStatus connection_status, std::string_view const &reason) {
  connection_status_ = connection_status;
  TraceInfo trace_info;
  auto stream_status = StreamStatus{
      .stream_id = stream_id_,
      .account = account_.name,
      .supports = SUPPORTS,
      .transport = Transport::TCP,
      .protocol = Protocol::HTTP,
      .encoding = {Encoding::JSON},
      .priority = Priority::PRIMARY,
      .connection_status = connection_status_,
      .reason = reason,
      .interface = (*connection_).get_interface(),
      .authority = (*connection_).get_current_authority(),
      .path = (*connection_).get_current_path(),
      .proxy = (*connection_).get_proxy(),
  };
  log::info("stream_status={}"sv, stream_status);
  create_trace_and_dispatch(handler_, trace_info, stream_status);
}

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading()) {
    download_.reset();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = account_.name,
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

// create-order

void OrderEntry::create_order(
    Event<CreateOrder> const &event, server::oms::Order const &order, server::oms::RefData const &ref_data, std::string_view const &request_id) {
  profile_.create_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, create_order] = event;
    auto query = json::Encoder::send_order(encode_buffer_, create_order, order, ref_data, request_id);
    auto path = shared_.api.order_management.send_order;
    auto headers = account_.create_headers(path, query);
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(shared_.settings),
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      auto version = 1;
      create_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::create_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.create_order_ack([&]() {
    auto dispatch = [&](auto origin, auto request_status, Error error, std::string_view const &text, std::string_view const &request_id, auto... args) {
      auto response = server::oms::Response{
          .request_type = RequestType::CREATE_ORDER,
          .origin = origin,
          .request_status = request_status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = request_id,
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      // log::warn("DEBUG response={}"sv, response);
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id, args...);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      dispatch(origin, status, error, text, {});
    };
    auto handle_success = [&](auto &body) {
      // log::warn(R"(DEBUG body="{}")"sv, body);
      json::SendOrder send_order{body, decode_buffer_};
      // log::warn(R"(DEBUG send_order={})"sv, send_order);
      switch (send_order.result) {
        using enum json::Result::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          log::warn(R"(response="{}")"sv, body);
          log::warn("send_order={}"sv, send_order);
          log::fatal("Unexpected"sv);
          break;
        case ERROR:
          dispatch(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, send_order.error, {});
          break;
        case SUCCESS: {
          auto request_id = send_order.send_status.cli_ord_id;  // note! could be missing
          auto accept_handler = [&](auto &order_update) {
            // log::warn("DEBUG order_update={}"sv, order_update);
            dispatch(Origin::EXCHANGE, RequestStatus::ACCEPTED, {}, {}, request_id, order_update);
          };
          auto reject_handler = [&](auto error, auto const &text) { dispatch(Origin::EXCHANGE, RequestStatus::REJECTED, error, text, request_id); };
          process_send_order(send_order.send_status, accept_handler, reject_handler);
          break;
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// modify-order

void OrderEntry::modify_order(
    Event<ModifyOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, modify_order] = event;
    auto query = json::Encoder::edit_order(encode_buffer_, modify_order, order, ref_data, request_id, previous_request_id);
    // log::warn("DEBUG query={}"sv, query);
    auto path = shared_.api.order_management.edit_order;
    auto headers = account_.create_headers(path, query);
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(shared_.settings),
    };
    auto callback = [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      modify_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::modify_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.modify_order_ack([&]() {
    auto dispatch = [&](auto origin, auto status, Error error, std::string_view const &text, std::string_view const &request_id, auto... args) {
      auto response = server::oms::Response{
          .request_type = RequestType::MODIFY_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = request_id,
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      // log::warn("DEBUG response={}"sv, response);
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id, args...);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      dispatch(origin, status, error, text, {});
    };
    auto handle_success = [&](auto &body) {
      // log::warn(R"(DEBUG body="{}")"sv, body);
      json::EditOrder edit_order{body, decode_buffer_};
      // log::warn(R"(DEBUG edit_order={})"sv, edit_order);
      switch (edit_order.result) {
        using enum json::Result::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          log::warn(R"(response="{}")"sv, body);
          log::warn("edit_order={}"sv, edit_order);
          log::fatal("Unexpected"sv);
          break;
        case ERROR:
          dispatch(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, edit_order.error, {});
          break;
        case SUCCESS: {
          auto request_id = edit_order.edit_status.cli_ord_id;
          auto accept_handler = [&](auto &order_update) {
            // log::warn("DEBUG order_update={}"sv, order_update);
            dispatch(Origin::EXCHANGE, RequestStatus::ACCEPTED, {}, {}, request_id, order_update);
          };
          auto reject_handler = [&](auto error, auto const &text) { dispatch(Origin::EXCHANGE, RequestStatus::REJECTED, error, text, request_id); };
          process_edit_order(edit_order.edit_status, accept_handler, reject_handler);
          break;
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// cancel-order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    server::oms::Order const &order,
    server::oms::RefData const &ref_data,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready()) {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &[message_info, cancel_order] = event;
    auto query = json::Encoder::cancel_order(encode_buffer_, cancel_order, order, ref_data, request_id, previous_request_id);
    auto path = shared_.api.order_management.cancel_order;
    auto headers = account_.create_headers(path, query);
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(shared_.settings),
    };
    auto callback = [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      cancel_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::cancel_order_ack(Trace<web::rest::Response> const &event, uint8_t user_id, uint64_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto dispatch = [&](auto origin, auto status, Error error, std::string_view const &text, std::string_view const &request_id, auto... args) {
      auto response = server::oms::Response{
          .request_type = RequestType::CANCEL_ORDER,
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = request_id,
          .external_order_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      // log::warn("DEBUG response={}"sv, response);
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id, args...);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      dispatch(origin, status, error, text, {});
    };
    auto handle_success = [&](auto &body) {
      // log::warn(R"(DEBUG body="{}")"sv, body);
      json::CancelOrder cancel_order{body, decode_buffer_};
      // log::warn(R"(DEBUG cancel_order={})"sv, cancel_order);
      switch (cancel_order.result) {
        using enum json::Result::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          log::warn(R"(response="{}")"sv, body);
          log::warn("cancel_order={}"sv, cancel_order);
          log::fatal("Unexpected"sv);
          break;
        case ERROR:
          dispatch(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, cancel_order.error, {});
          break;
        case SUCCESS: {
          auto request_id = cancel_order.cancel_status.cli_ord_id;
          auto accept_handler = [&](auto &order_update) {
            // log::warn("DEBUG order_update={}"sv, order_update);
            dispatch(Origin::EXCHANGE, RequestStatus::ACCEPTED, {}, {}, request_id, order_update);
          };
          auto reject_handler = [&](auto error, auto const &text) { dispatch(Origin::EXCHANGE, RequestStatus::REJECTED, error, text, request_id); };
          process_cancel_order(cancel_order.cancel_status, accept_handler, reject_handler);
          break;
        }
      }
    };
    process_response(event, handle_error, handle_success);
  });
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready()) [[unlikely]] {
      throw server::oms::NotReady{"not ready"sv};
    }
    auto &cancel_all_orders = event.value;
    auto send_ack = [&]() {
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = cancel_all_orders.order_id,
          .exchange = cancel_all_orders.exchange,
          .symbol = cancel_all_orders.symbol,
          .side = cancel_all_orders.side,
          .origin = Origin::GATEWAY,
          .request_status = RequestStatus::FORWARDED,
          .error = {},
          .text = {},
          .request_id = request_id,
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = cancel_all_orders.strategy_id,
      };
      TraceInfo trace_info{event};
      Trace event_2{trace_info, cancel_all_orders_ack};
      shared_(event_2);
    };
    auto path = shared_.api.order_management.cancel_all_orders;
    auto headers = account_.create_headers(path, {});
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(shared_.settings),
    };
    auto callback = [this](auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      cancel_all_orders_ack(event, request_id);
    };
    (*connection_)(request_id, request, callback);
    send_ack();
  });
}

void OrderEntry::cancel_all_orders_ack(Trace<web::rest::Response> const &event, std::string_view const &request_id) {
  profile_.cancel_all_orders_ack([&]() {
    auto send_ack = [&](auto origin, auto status, Error error, std::string_view const &text) {
      auto cancel_all_orders_ack = CancelAllOrdersAck{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = {},
          .symbol = {},
          .side = {},
          .origin = origin,
          .request_status = status,
          .error = error,
          .text = text,
          .request_id = request_id,
          .external_account = {},
          .number_of_affected_orders = {},
          .round_trip_latency = {},
          .user = {},
          .strategy_id = {},
      };
      Trace event_2{event, cancel_all_orders_ack};
      shared_(event_2);
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto const &text) {
      log::warn(R"(DEBUG origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      send_ack(origin, status, error, text);
    };
    auto handle_success = [&](auto &body) {
      // log::warn(R"(DEBUG body="{}")"sv, body);
      json::CancelAllOrders cancel_all_orders{body, decode_buffer_};
      log::info("*** CANCELED {} ORDER(S) ***"sv, std::size(cancel_all_orders.cancel_status.order_events));
      send_ack(Origin::EXCHANGE, RequestStatus::ACCEPTED, {}, {});
    };
    process_response(event, handle_error, handle_success);
  });
}

// cancel-all-orders-after

void OrderEntry::cancel_all_orders_after(std::chrono::nanoseconds timeout) {
  auto value = std::chrono::duration_cast<std::chrono::seconds>(timeout);
  auto query = fmt::format("?timeout={}"sv, value.count());
  auto path = shared_.api.order_management.cancel_all_orders_after;
  auto headers = account_.create_headers(path, query);
  auto request = web::rest::Request{
      .method = web::http::Method::POST,
      .path = path,
      .query = query,
      .accept = web::http::Accept::APPLICATION_JSON,
      .content_type = {},
      .headers = headers,
      .body = {},
      .quality_of_service = get_quality_of_service(shared_.settings),
  };
  auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
    TraceInfo trace_info;
    Trace event{trace_info, response};
    cancel_all_orders_after_ack(event);
  };
  (*connection_)("cancel-all-orders-after"sv, request, callback);
}

void OrderEntry::cancel_all_orders_after_ack(Trace<web::rest::Response> const &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto const &text) {
      log::warn(R"(origin={}, error={}, status={}, text="{}")"sv, origin, error, status, text);
      // note! no response required
    };
    auto handle_success = [&](auto &body) {
      json::CancelAllAfterAck cancel_all_after_ack{body, decode_buffer_};
      log::info<2>("cancel_all_after_ack={}"sv, cancel_all_after_ack);
    };
    process_response(event, handle_error, handle_success);
  });
}

uint32_t OrderEntry::download(State state) {
  switch (state) {
    using enum State;
    case UNDEFINED:
      assert(false);
      break;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return 0;
  }
  assert(false);
  return 0;
}

void OrderEntry::process_response(web::rest::Response const &response, auto error_handler, auto success_handler) {
  try {
    auto [status, category, body] = response.result();
    switch (category) {
      using enum web::http::Category;
      case UNKNOWN:
      case INFORMATIONAL_RESPONSE:
        response.expect(web::http::Status::OK);  // throws
        break;
      case SUCCESS:
        success_handler(body);
        break;
      case REDIRECTION:
        log::fatal("Unexpected: URL is being redirected"sv);
      case CLIENT_ERROR: {
        json::RestError rest_error{body, decode_buffer_};
        log::warn("error={}"sv, rest_error);
        auto message = [&]() {
          if (!std::empty(rest_error.errors)) {
            return rest_error.errors[0].message;
          }
          if (!std::empty(rest_error.error)) {
            return rest_error.error;
          }
          return rest_error.message;
        }();
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
      case SERVER_ERROR: {
        auto message = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, message);
        break;
      }
    }
  } catch (server::oms::Exception &e) {
    std::string_view const what{e.what()};
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), what);
    error_handler(e.origin, e.status, e.error, what);
  } catch (NetworkError &e) {
    std::string_view const what{e.what()};
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), what);
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), what);
  } catch (std::exception &e) {
    std::string_view const what{e.what()};
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), what);
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, what);
  }
}

template <typename... Args>
void OrderEntry::operator()(Trace<server::oms::Response> const &event, uint8_t user_id, uint64_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(user_id, order_id, stream_id_, trace_info, response, std::forward<Args>(args)..., []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

void OrderEntry::operator()(Trace<server::oms::OrderUpdate> const &event, std::string_view const &client_order_id) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

// helpers

template <typename Accept, typename Reject>
void OrderEntry::process_send_order(auto &request_status, Accept accept, Reject reject) {
  switch (request_status.status) {
    using enum json::Status::type_t;
    case PLACED:
    case PARTIALLY_FILLED:  // note! have not seen this during testing, but found this enum in the docs
    case FILLED: {
      auto order_event_type = compute_order_event_type(request_status.order_events);
      switch (order_event_type) {
        using enum json::OrderEventType::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          throw RuntimeError{"Unexpected"sv};
        case PLACE:
          process_place(request_status, accept);
          break;
        case EDIT:
          throw RuntimeError{R"(Unexpected: type="{}")"sv, order_event_type};
        case CANCEL:
          throw RuntimeError{R"(Unexpected: type="{}")"sv, order_event_type};  // XXX FIXME TODO possible ???
        case EXECUTION:
          process_execution(request_status, accept);
          break;
        case REJECT:
          reject(map(request_status.status), request_status.status.as_raw_text());
          break;
      }
      break;
    }
    default:
      reject(map(request_status.status), request_status.status.as_raw_text());
  }
}

template <typename Accept, typename Reject>
void OrderEntry::process_edit_order(auto &request_status, Accept accept, Reject reject) {
  switch (request_status.status) {
    using enum json::Status::type_t;
    case EDITED:
    case FILLED:
    case PARTIALLY_FILLED: {
      auto order_event_type = compute_order_event_type(request_status.order_events);
      switch (order_event_type) {
        using enum json::OrderEventType::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          throw RuntimeError{"Unexpected"sv};
        case PLACE:
          throw RuntimeError{R"(Unexpected: type="{}")"sv, order_event_type};
          break;
        case EDIT:
          process_edit(request_status, accept);
          break;
        case CANCEL:
          throw RuntimeError{R"(Unexpected: type="{}")"sv, order_event_type};  // XXX FIXME TODO possible ???
        case EXECUTION:
          process_execution(request_status, accept);
          break;
        case REJECT:
          reject(map(request_status.status), request_status.status.as_raw_text());
          break;
      }
      break;
    }
    default:
      reject(map(request_status.status), request_status.status.as_raw_text());
  }
}

template <typename Accept, typename Reject>
void OrderEntry::process_cancel_order(auto &request_status, Accept accept, Reject reject) {
  switch (request_status.status) {
    using enum json::Status::type_t;
    case CANCELLED:
    case PARTIALLY_FILLED:
    case FILLED: {
      auto order_event_type = compute_order_event_type(request_status.order_events);
      switch (order_event_type) {
        using enum json::OrderEventType::type_t;
        case UNDEFINED_INTERNAL:
        case UNKNOWN_INTERNAL:
          throw RuntimeError{"Unexpected"sv};
        case PLACE:
          throw RuntimeError{R"(Unexpected: type="{}")"sv, order_event_type};
        case EDIT:
          throw RuntimeError{R"(Unexpected: type="{}")"sv, order_event_type};
        case CANCEL:
          process_edit(request_status, accept);
          break;
        case EXECUTION:
          process_execution(request_status, accept);
          break;
        case REJECT:
          reject(map(request_status.status), request_status.status.as_raw_text());
          break;
      }
      break;
    }
    default:
      reject(map(request_status.status), request_status.status.as_raw_text());
      break;
  }
}

// helpers 2

template <typename Callback>
void OrderEntry::process_place(auto &request_status, Callback callback) {
  if (std::size(request_status.order_events) != 1) {
    throw RuntimeError{"Unexpected: size={}"sv, std::size(request_status.order_events)};
  }
  auto &order_event = request_status.order_events[0];
  auto &order = order_event.order;
  auto remaining_quantity = order.quantity;
  auto traded_quantity = order.filled;
  auto quantity = remaining_quantity + traded_quantity;
  auto order_status = compute_order_status(request_status.status, remaining_quantity);
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = {},
      .symbol = order.symbol,
      .side = map(order.side),
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order.type),
      .time_in_force = {},
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = request_status.received_time,
      .external_account = {},
      .external_order_id = request_status.order_id,
      .client_order_id = {},
      .order_status = order_status,
      .error = {},
      .text = {},
      .quantity = quantity,        // note!
      .price = order.limit_price,  // note!
      .stop_price = NaN,
      .leverage = NaN,
      .remaining_quantity = remaining_quantity,  // note!
      .traded_quantity = traded_quantity,        // note!
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = {},
      .sending_time_utc = {},
  };
  callback(std::as_const(order_update));
}

template <typename Callback>
void OrderEntry::process_edit(auto &request_status, Callback callback) {
  if (std::size(request_status.order_events) != 1) {
    throw RuntimeError{"Unexpected: size={}"sv, std::size(request_status.order_events)};
  }
  auto &order_event = request_status.order_events[0];
  auto &order = order_event.order;
  auto remaining_quantity = order.quantity;
  auto traded_quantity = order.filled;
  auto quantity = remaining_quantity + traded_quantity;
  auto order_status = compute_order_status(request_status.status, remaining_quantity);
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = {},
      .symbol = order.symbol,
      .side = map(order.side),
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order.type),
      .time_in_force = {},
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = request_status.received_time,
      .external_account = {},
      .external_order_id = request_status.order_id,
      .client_order_id = {},
      .order_status = order_status,
      .error = {},
      .text = {},
      .quantity = quantity,        // note!
      .price = order.limit_price,  // note!
      .stop_price = NaN,
      .leverage = NaN,
      .remaining_quantity = remaining_quantity,  // note!
      .traded_quantity = traded_quantity,        // note!
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = {},
      .sending_time_utc = {},
  };
  callback(std::as_const(order_update));
}

template <typename Callback>
void OrderEntry::process_cancel(auto &request_status, Callback callback) {
  if (std::size(request_status.order_events) != 1) {
    throw RuntimeError{"Unexpected: size={}"sv, std::size(request_status.order_events)};
  }
  auto &order_event = request_status.order_events[0];
  auto &order = order_event.order;
  auto remaining_quantity = order.quantity;
  auto traded_quantity = order.filled;
  auto quantity = remaining_quantity + traded_quantity;
  auto order_status = compute_order_status(request_status.status, remaining_quantity);
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = {},
      .symbol = order.symbol,
      .side = map(order.side),
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order.type),
      .time_in_force = {},
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = request_status.received_time,
      .external_account = {},
      .external_order_id = request_status.order_id,
      .client_order_id = {},
      .order_status = order_status,
      .error = {},
      .text = {},
      .quantity = quantity,        // note!
      .price = order.limit_price,  // note!
      .stop_price = NaN,
      .leverage = NaN,
      .remaining_quantity = remaining_quantity,  // note!
      .traded_quantity = traded_quantity,        // note!
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = {},
      .sending_time_utc = {},
  };
  callback(std::as_const(order_update));
}

template <typename Callback>
void OrderEntry::process_execution(auto &request_status, Callback callback) {
  if (std::empty(request_status.order_events)) {
    throw RuntimeError{"Unexpected: size={}"sv, std::size(request_status.order_events)};
  }
  std::string_view symbol;
  Side side = {};
  double price = NaN;
  double prior_quantity = NaN;
  double prior_filled = NaN;
  double last_traded_quantity = 0.0;
  double sum_product = 0.0;
  for (auto &order_event : request_status.order_events) {
    if (order_event.type != json::OrderEventType::EXECUTION) {
      throw RuntimeError{"Unexpected: type={}"sv, order_event.type};
    }
    auto &order_prior_execution = order_event.order_prior_execution;
    if (std::empty(symbol)) {
      symbol = order_prior_execution.symbol;
    }
    side = map(order_prior_execution.side);
    price = order_prior_execution.limit_price;
    prior_quantity = order_prior_execution.quantity;
    prior_filled = order_prior_execution.filled;
    last_traded_quantity += order_event.amount;
    sum_product += order_event.amount * order_event.price;
  }
  auto last_traded_price = utils::is_zero(last_traded_quantity) ? NaN : (sum_product / last_traded_quantity);
  auto remaining_quantity = prior_quantity - last_traded_quantity;  // ???
  auto traded_quantity = prior_filled + last_traded_quantity;
  auto order_status = compute_order_status(request_status.status, remaining_quantity);
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = {},
      .symbol = symbol,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = {},  // note! we can't infer order_type because they convert market orders into IOC limit orders
      .time_in_force = {},
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = request_status.received_time,
      .external_account = {},
      .external_order_id = request_status.order_id,
      .client_order_id = {},
      .order_status = order_status,
      .error = {},
      .text = {},
      .quantity = NaN,  // note!
      .price = price,   // note!
      .stop_price = NaN,
      .leverage = NaN,
      .remaining_quantity = remaining_quantity,  // note!
      .traded_quantity = traded_quantity,        // note!
      .average_traded_price = NaN,
      .last_traded_quantity = last_traded_quantity,  // note!
      .last_traded_price = last_traded_price,        // note!
      .last_liquidity = Liquidity::TAKER,
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = {},
      .sending_time_utc = {},
  };
  callback(std::as_const(order_update));  // XXX FIXME TODO include the fills
}

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

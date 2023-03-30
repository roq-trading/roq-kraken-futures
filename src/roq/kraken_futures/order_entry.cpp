/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/order_entry.hpp"

#include <utility>

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/rest/client_factory.hpp"

#include "roq/kraken_futures/flags.hpp"
#include "roq/kraken_futures/order_update.hpp"

#include "roq/kraken_futures/json/cancel_all_after_ack.hpp"
#include "roq/kraken_futures/json/cancel_all_orders.hpp"
#include "roq/kraken_futures/json/cancel_order.hpp"
#include "roq/kraken_futures/json/edit_order.hpp"
#include "roq/kraken_futures/json/rest_error.hpp"
#include "roq/kraken_futures/json/result.hpp"
#include "roq/kraken_futures/json/send_order.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "om"sv;

auto const SUPPORTS = Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  auto config = web::rest::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      // connection manager
      .connection_timeout = {},
      .disconnect_on_idle_timeout = {},
      .connection = web::http::Connection::KEEP_ALIVE,
      // proxy
      .proxy = Flags::rest_proxy(),
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
      // implementation
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .allow_pipelining = true,
  };
  return web::rest::ClientFactory::create(handler, context, config);
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

// following is used from several places

auto get_quality_of_service() {
  return Flags::rest_allow_order_request_pipeline() ? io::QualityOfService::IMMEDIATE : io::QualityOfService::CRITICAL;
}
}  // namespace

// === IMPLEMENTATION ===

OrderEntry::OrderEntry(
    Handler &handler, io::Context &context, uint16_t stream_id, Authenticator &authenticator, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, authenticator.get_account())},
      connection_{create_connection(*this, context)}, decode_buffer_{Flags::decode_buffer_size()},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .create_order = create_metrics(name_, "create_order"sv),
          .create_order_ack = create_metrics(name_, "create_order_ack"sv),
          .modify_order = create_metrics(name_, "modify_order"sv),
          .modify_order_ack = create_metrics(name_, "modify_order_ack"sv),
          .cancel_order = create_metrics(name_, "cancel_order"sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"sv),
          .cancel_all_orders_ack = create_metrics(name_, "cancel_all_orders_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
      },
      authenticator_{authenticator}, shared_{shared},
      download_{Flags::rest_request_timeout(), [this](auto state) { return download(state); }} {
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
  if (Flags::rest_cancel_on_disconnect() && Flags::rest_cancel_all_after().count() && ready() &&
      next_cancel_all_timer_ < now) {
    next_cancel_all_timer_ = now + Flags::rest_cancel_all_after() / 4;
    cancel_all_orders_after(Flags::rest_cancel_all_after());
  }
}

void OrderEntry::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.create_order, metrics::PROFILE)
      .write(profile_.create_order_ack, metrics::PROFILE)
      .write(profile_.modify_order, metrics::PROFILE)
      .write(profile_.modify_order_ack, metrics::PROFILE)
      .write(profile_.cancel_order, metrics::PROFILE)
      .write(profile_.cancel_order_ack, metrics::PROFILE)
      .write(profile_.cancel_all_orders, metrics::PROFILE)
      .write(profile_.cancel_all_orders_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

namespace {
json::OrderEventOrderType compute_order_type(
    auto const &order_type, auto const &time_in_force, auto const &execution_instructions, auto const &stop_price) {
  if (time_in_force == TimeInForce::IOC)
    return json::OrderEventOrderType::IOC;
  switch (order_type) {
    using enum roq::OrderType;
    case UNDEFINED:
      break;
    case MARKET:
      return json::OrderEventOrderType::MKT;
    case LIMIT:
      if (std::isnan(stop_price))
        return json::OrderEventOrderType::LMT;
      else
        return json::OrderEventOrderType::STP;
      break;
  }
  throw RuntimeError{
      "Unexpected combination of order_type={}, time_in_force={}, execution_instructions={}, stop_price={}"sv,
      order_type,
      time_in_force,
      execution_instructions,
      stop_price};
}
}  // namespace

uint16_t OrderEntry::operator()(
    Event<CreateOrder> const &event, oms::Order const &order, std::string_view const &request_id) {
  create_order(event, order, request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  modify_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    std::string_view const &previous_request_id) {
  cancel_order(event, order, request_id, previous_request_id);
  return stream_id_;
}

uint16_t OrderEntry::operator()(Event<CancelAllOrders> const &event, std::string_view const &request_id) {
  cancel_all_orders(event, request_id);
  return stream_id_;
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = authenticator_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::HTTP,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Connected> const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(Trace<web::rest::Client::Disconnected> const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void OrderEntry::operator()(Trace<web::rest::Client::Latency> const &event) {
  auto &[trace_info, latency] = event;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = authenticator_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void OrderEntry::operator()(
    Trace<web::rest::Response> const &, [[maybe_unused]] uint64_t request_id, [[maybe_unused]] uint64_t opaque) {
}

// create-order

void OrderEntry::create_order(Event<CreateOrder> const &event, oms::Order const &, std::string_view const &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
    auto &[message_info, create_order] = event;
    auto order_type = compute_order_type(
        create_order.order_type,
        create_order.time_in_force,
        create_order.execution_instructions,
        create_order.stop_price);
    auto side = json::map(create_order.side);
    auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
    std::string query;
    if (!std::isnan(create_order.price)) {
      if (std::isnan(create_order.stop_price)) {
        query = fmt::format(  // limit
            "?orderType={}"
            "&symbol={}"
            "&side={}"
            "&size={}"
            "&limitPrice={}"
            "&cliOrdId={}"
            "&reduceOnly={}"sv,
            order_type.as_raw_text(),
            create_order.symbol,
            side.as_raw_text(),
            create_order.quantity,
            create_order.price,
            request_id,
            reduce_only);
      } else {
        query = fmt::format(  // limit + stop
            "?orderType={}"
            "&symbol={}"
            "&side={}"
            "&size={}"
            "&limitPrice={}"
            "&stopPrice={}"
            "&cliOrdId={}"
            "&reduceOnly={}"sv,
            order_type.as_raw_text(),
            create_order.symbol,
            side.as_raw_text(),
            create_order.quantity,
            create_order.price,
            create_order.stop_price,
            request_id,
            reduce_only);
      }
    } else {
      query = fmt::format(  // market
          "?orderType={}"
          "&symbol={}"
          "&side={}"
          "&size={}"
          "&cliOrdId={}"
          "&reduceOnly={}"sv,
          order_type.as_raw_text(),
          create_order.symbol,
          side.as_raw_text(),
          create_order.quantity,
          request_id,
          reduce_only);
    }
    log::debug(R"(query="{}")"sv, query);
    auto path = "/api/v3/sendorder"sv;
    auto headers = authenticator_.create_headers(path, query);
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = web::http::ContentType::APPLICATION_JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    auto callback = [this, user_id = message_info.source, order_id = create_order.order_id](
                        [[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      auto version = uint32_t{1};
      create_order_ack(event, user_id, order_id, version);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::create_order_ack(
    Trace<web::rest::Response> const &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.create_order_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::SendOrder send_order{body, decode_buffer_};
      switch (send_order.result) {
        using enum json::Result::type_t;
        case UNDEFINED:
        case UNKNOWN:
          log::warn(R"(response="{}")"sv, body);
          log::warn("send_order={}"sv, send_order);
          log::fatal("Unexpected"sv);
          break;
        case ERROR:
          log::warn("send_order={}"sv, send_order);
          throw oms::Rejected{Origin::EXCHANGE, Error::UNKNOWN, "{}"sv, send_order.error};
          break;
        case SUCCESS: {
          auto request_id = send_order.send_status.cli_ord_id;
          OrderUpdate{shared_, stream_id_, authenticator_.get_account()}(
              order_id,
              send_order,
              [&](auto &order_update) {
                log::debug("order_update={}"sv, order_update);
                oms::Response response{
                    .type = RequestType::CREATE_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::ACCEPTED,
                    .error = {},
                    .text = {},
                    .version = version,
                    .request_id = request_id,
                    .quantity = NaN,
                    .price = NaN,
                };
                Trace event_2{event, response};
                (*this)(event_2, user_id, order_id, order_update);
              },
              [&](auto error, auto text) {
                throw oms::Rejected{Origin::EXCHANGE, error, "{}"sv, text};
              });
          break;
        }
      }
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto text) {
      auto response = oms::Response{
          .type = RequestType::CREATE_ORDER,
          .origin = origin,
          .status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    process_response(event, handle_success, handle_error);
  });
}

// modify-order

void OrderEntry::modify_order(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
    auto &[message_info, modify_order] = event;
    // note! price has max 2 decimals, size is integer
    auto query = fmt::format(
        "?orderId={}"
        "&size={}"
        "&limitPrice={}"sv,
        order.external_order_id,
        modify_order.quantity,
        modify_order.price);
    log::debug(R"(query="{}")"sv, query);
    auto path = "/api/v3/editorder"sv;
    auto headers = authenticator_.create_headers(path, query);
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    auto callback =
        [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
            [[maybe_unused]] auto &request_id, auto &response) {
          TraceInfo trace_info;
          Trace event{trace_info, response};
          modify_order_ack(event, user_id, order_id, version);
        };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::modify_order_ack(
    Trace<web::rest::Response> const &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.modify_order_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::EditOrder edit_order{body, decode_buffer_};
      switch (edit_order.result) {
        using enum json::Result::type_t;
        case UNDEFINED:
        case UNKNOWN:
          log::warn(R"(response="{}")"sv, body);
          log::warn("edit_order={}"sv, edit_order);
          log::fatal("Unexpected"sv);
          break;
        case ERROR:
          log::warn("edit_order={}"sv, edit_order);
          throw oms::Rejected{Origin::EXCHANGE, Error::UNKNOWN, "{}"sv, edit_order.error};
          break;
        case SUCCESS: {
          auto request_id = edit_order.edit_status.cli_ord_id;
          OrderUpdate{shared_, stream_id_, authenticator_.get_account()}(
              order_id,
              edit_order,
              [&](auto &order_update) {
                log::debug("order_update={}"sv, order_update);
                auto response = oms::Response{
                    .type = RequestType::MODIFY_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::ACCEPTED,
                    .error = {},
                    .text = {},
                    .version = version,
                    .request_id = request_id,
                    .quantity = NaN,
                    .price = NaN,
                };
                Trace event_2{event, response};
                (*this)(event_2, user_id, order_id, order_update);
              },
              [&](auto error, auto text) {
                throw oms::Rejected{Origin::EXCHANGE, error, "{}"sv, text};
              });
          break;
        }
      }
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto text) {
      auto response = oms::Response{
          .type = RequestType::MODIFY_ORDER,
          .origin = origin,
          .status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    process_response(event, handle_success, handle_error);
  });
}

// cancel-order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
    auto &[message_info, cancel_order] = event;
    auto query = fmt::format("?order_id={}"sv, order.external_order_id);
    log::debug(R"(query="{}")"sv, query);
    auto path = "/api/v3/cancelorder"sv;
    auto headers = authenticator_.create_headers(path, query);
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = query,
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    auto callback =
        [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
            [[maybe_unused]] auto &request_id, auto &response) {
          TraceInfo trace_info;
          Trace event{trace_info, response};
          cancel_order_ack(event, user_id, order_id, version);
        };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::cancel_order_ack(
    Trace<web::rest::Response> const &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::CancelOrder cancel_order{body, decode_buffer_};
      switch (cancel_order.result) {
        using enum json::Result::type_t;
        case UNDEFINED:
        case UNKNOWN:
          log::warn(R"(response="{}")"sv, body);
          log::warn("cancel_order={}"sv, cancel_order);
          log::fatal("Unexpected"sv);
          break;
        case ERROR:
          log::warn("cancel_order={}"sv, cancel_order);
          throw oms::Rejected{Origin::EXCHANGE, Error::UNKNOWN, "{}"sv, cancel_order.error};
          break;
        case SUCCESS: {
          auto request_id = cancel_order.cancel_status.cli_ord_id;
          OrderUpdate{shared_, stream_id_, authenticator_.get_account()}(
              order_id,
              cancel_order,
              [&](auto &order_update) {
                log::debug("order_update={}"sv, order_update);
                auto response = oms::Response{
                    .type = RequestType::CANCEL_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::ACCEPTED,
                    .error = {},
                    .text = {},
                    .version = version,
                    .request_id = request_id,
                    .quantity = NaN,
                    .price = NaN,
                };
                Trace event_2{event, response};
                (*this)(event_2, user_id, order_id, order_update);
              },
              [&](auto error, auto text) {
                throw oms::Rejected{Origin::EXCHANGE, error, "{}"sv, text};
              });
          break;
        }
      }
    };
    auto handle_error = [&](auto origin, auto status, auto error, auto text) {
      auto response = oms::Response{
          .type = RequestType::CANCEL_ORDER,
          .origin = origin,
          .status = status,
          .error = error,
          .text = text,
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      Trace event_2{event, response};
      (*this)(event_2, user_id, order_id);
    };
    process_response(event, handle_success, handle_error);
  });
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready())
      throw oms::NotReady{"not ready"sv};
    auto path = "/api/v3/cancelallorders"sv;
    auto headers = authenticator_.create_headers(path, {});
    auto request = web::rest::Request{
        .method = web::http::Method::POST,
        .path = path,
        .query = {},
        .accept = web::http::Accept::APPLICATION_JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
      TraceInfo trace_info;
      Trace event{trace_info, response};
      cancel_all_orders_ack(event);
    };
    (*connection_)(request_id, request, callback);
  });
}

void OrderEntry::cancel_all_orders_ack(Trace<web::rest::Response> const &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::CancelAllOrders cancel_all_orders{body, decode_buffer_};
      log::info("*** CANCELED {} ORDER(S) ***"sv, std::size(cancel_all_orders.cancel_status.order_events));
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      // note! no response required
    };
    process_response(event, handle_success, handle_error);
  });
}

// cancel-all-orders-after

void OrderEntry::cancel_all_orders_after(std::chrono::nanoseconds timeout) {
  auto value = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
  auto query = fmt::format("?timeout={}"sv, value.count());
  auto path = "/api/v3/cancelallordersafter"sv;
  auto headers = authenticator_.create_headers(path, query);
  auto request = web::rest::Request{
      .method = web::http::Method::POST,
      .path = path,
      .query = query,
      .accept = web::http::Accept::APPLICATION_JSON,
      .content_type = {},
      .headers = headers,
      .body = {},
      .quality_of_service = get_quality_of_service(),
  };
  auto callback = [this]([[maybe_unused]] auto &request_id, auto &response) {
    TraceInfo trace_info;
    Trace event{trace_info, response};
    cancel_all_orders_after_ack(event);
  };
  (*connection_)("cancel_all_orders_after"sv, request, callback);
}

void OrderEntry::cancel_all_orders_after_ack(Trace<web::rest::Response> const &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto handle_success = [&](auto &body) {
      json::CancelAllAfterAck cancel_all_after_ack{body, decode_buffer_};
      log::debug("cancel_all_after_ack={}"sv, cancel_all_after_ack);
      log::info<2>("cancel_all_after_ack={}"sv, cancel_all_after_ack);
    };
    auto handle_error = [&]([[maybe_unused]] auto origin, [[maybe_unused]] auto status, auto error, auto text) {
      log::warn(R"(error={}, text="{}")"sv, error, text);
      // note! no response required
    };
    process_response(event, handle_success, handle_error);
  });
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    using enum OrderEntryState;
    case UNDEFINED:
      assert(false);
      break;
    case DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

template <typename SuccessHandler, typename ErrorHandler>
void OrderEntry::process_response(
    web::rest::Response const &response, SuccessHandler success_handler, ErrorHandler error_handler) {
  try {
    auto [status, category, body] = response.result();
    log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
    switch (category) {
      using enum web::http::Category;
      case SUCCESS:  // 2xx
        success_handler(body);
        break;
      case CLIENT_ERROR: {  // 4xx
        json::RestError rest_error{body, decode_buffer_};
        log::warn("error={}"sv, rest_error);
        auto text = std::size(rest_error.errors) > 0 ? rest_error.errors[0].message : rest_error.message;
        error_handler(Origin::EXCHANGE, RequestStatus::REJECTED, Error::UNKNOWN, text);
        break;
      }
      case SERVER_ERROR: {  // 5xx
        auto text = fmt::format("{}"sv, status);
        error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, text);
        break;
      }
      default:
        response.expect(web::http::Status::OK);  // throws
    }
  } catch (oms::Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(e.origin, e.status, e.error, e.what());
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::GATEWAY, e.request_status(), e.error(), e.what());
  } catch (std::exception &e) {
    log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
    error_handler(Origin::EXCHANGE, RequestStatus::ERROR, Error::UNKNOWN, e.what());
  }
}

template <typename... Args>
void OrderEntry::operator()(Trace<oms::Response> const &event, uint8_t user_id, uint32_t order_id, Args &&...args) {
  auto &[trace_info, response] = event;
  if (shared_.update_order(
          user_id,
          order_id,
          stream_id_,
          trace_info,
          response,
          std::forward<Args>(args)...,
          []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("Did not find order: user_id={}, order_id={}"sv, user_id, order_id);
  }
}

template <typename... Args>
void OrderEntry::operator()(
    Trace<oms::OrderUpdate> const &event, std::string_view const &client_order_id, Args &&...args) {
  auto &[trace_info, order_update] = event;
  if (shared_.update_order(
          client_order_id, stream_id_, trace_info, order_update, [&]([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
  }
}

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2022, Hans Erik Thrane */

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

namespace {
auto const NAME = "om"sv;

const Mask SUPPORTS{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(std::string_view const &group, std::string_view const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  web::rest::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = web::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return web::rest::ClientFactory::create(handler, context, config);
}

auto get_quality_of_service() {
  return Flags::rest_allow_order_request_pipeline() ? io::QualityOfService::IMMEDIATE : io::QualityOfService::CRITICAL;
}
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler, io::Context &context, uint16_t stream_id, Security &security, Shared &shared, bool master)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"sv, stream_id_, NAME, security.get_account())), master_(master),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
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
      security_(security), shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
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
  throw RuntimeError(
      "Unexpected combination of order_type={}, time_in_force={}, execution_instructions={}, stop_price={}"sv,
      order_type,
      time_in_force,
      execution_instructions,
      stop_price);
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
    auto trace_info = server::create_trace_info();
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
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

void OrderEntry::operator()(web::rest::Client::Connected const &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(web::rest::Client::Disconnected const &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void OrderEntry::operator()(web::rest::Client::Latency const &latency) {
  auto trace_info = server::create_trace_info();
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = security_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

// create-order

void OrderEntry::create_order(Event<CreateOrder> const &event, oms::Order const &, std::string_view const &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, create_order] = event;
    auto method = web::http::Method::POST;
    auto path = "/api/v3/sendorder"sv;
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
    auto headers = security_.create_headers(path, query);
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::JSON,
        .content_type = web::http::ContentType::JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    (*connection_)(
        request_id,
        request,
        [this, user_id = message_info.source, order_id = create_order.order_id](
            [[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          uint32_t version = 1;
          create_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::create_order_ack(
    Trace<web::rest::Response const> const &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.create_order_ack([&]() {
    // auto &[trace_info, response] = event; // XXX clang13
    auto &trace_info = event.trace_info;
    auto &response = event.value;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        using enum web::http::Category;
        case SUCCESS: {
          core::json::Buffer buffer(decode_buffer_);
          auto send_order = core::json::Parser::create<json::SendOrder>(body, buffer);
          switch (send_order.result) {
            using enum json::Result::type_t;
            case UNDEFINED:
            case UNKNOWN:
              log::warn(R"(response="{}")"sv, body);
              log::fatal("Unexpected: send_order={}"sv, send_order);
              break;
            case ERROR: {
              log::warn("send_order={}"sv, send_order);
              oms::Response response{
                  .type = RequestType::CREATE_ORDER,
                  .origin = Origin::EXCHANGE,
                  .status = RequestStatus::REJECTED,
                  .error = Error::UNKNOWN,
                  .text = send_order.error,
                  .version = version,
                  .request_id = {},
                  .quantity = NaN,
                  .price = NaN,
              };
              shared_.update_order(
                  user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
            } break;
            case SUCCESS: {
              auto request_id = send_order.send_status.cli_ord_id;
              OrderUpdate{shared_, stream_id_, security_.get_account()}(
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
                    shared_.update_order(
                        user_id,
                        order_id,
                        stream_id_,
                        trace_info,
                        response,
                        order_update,
                        []([[maybe_unused]] auto &order) {});
                  },
                  [&](auto error, auto text) {
                    oms::Response response{
                        .type = RequestType::CREATE_ORDER,
                        .origin = Origin::EXCHANGE,
                        .status = RequestStatus::REJECTED,
                        .error = error,
                        .text = text,
                        .version = version,
                        .request_id = request_id,
                        .quantity = NaN,
                        .price = NaN,
                    };
                    shared_.update_order(
                        user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
                  });
              break;
            }
          }
          break;
        }
        case CLIENT_ERROR: {
          core::json::Buffer buffer(decode_buffer_);
          auto error = core::json::Parser::create<json::RestError>(body, buffer);
          log::warn("error={}"sv, error);
          auto text = std::size(error.errors) > 0 ? error.errors[0].message : error.message;
          oms::Response response{
              .type = RequestType::CREATE_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = Error::UNKNOWN,
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          shared_.update_order(
              user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
          break;
        }
        default:
          response.expect(web::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::CREATE_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn("Did not find order: user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
      }
    }
  });
}

// modify-order

void OrderEntry::modify_order(
    Event<ModifyOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, modify_order] = event;
    auto method = web::http::Method::POST;
    auto path = "/api/v3/editorder"sv;
    // note! price has max 2 decimals, size is integer
    auto query = fmt::format(
        "?orderId={}"
        "&size={}"
        "&limitPrice={}"sv,
        order.external_order_id,
        modify_order.quantity,
        modify_order.price);
    log::debug(R"(query="{}")"sv, query);
    auto headers = security_.create_headers(path, query);
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    (*connection_)(
        request_id,
        request,
        [this, user_id = message_info.source, order_id = modify_order.order_id, version = modify_order.version](
            [[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          modify_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::modify_order_ack(
    Trace<web::rest::Response const> const &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.modify_order_ack([&]() {
    // auto &[trace_info, response] = event; // XXX clang13
    auto &trace_info = event.trace_info;
    auto &response = event.value;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        using enum web::http::Category;
        case SUCCESS: {
          core::json::Buffer buffer(decode_buffer_);
          auto edit_order = core::json::Parser::create<json::EditOrder>(body, buffer);
          switch (edit_order.result) {
            using enum json::Result::type_t;
            case UNDEFINED:
            case UNKNOWN:
              log::warn(R"(response="{}")"sv, body);
              log::fatal("Unexpected: edit_order={}"sv, edit_order);
              break;
            case ERROR: {
              log::warn("edit_order={}"sv, edit_order);
              oms::Response response{
                  .type = RequestType::CREATE_ORDER,
                  .origin = Origin::EXCHANGE,
                  .status = RequestStatus::REJECTED,
                  .error = Error::UNKNOWN,
                  .text = edit_order.error,
                  .version = version,
                  .request_id = {},
                  .quantity = NaN,
                  .price = NaN,
              };
              shared_.update_order(
                  user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
              break;
            }
            case SUCCESS: {
              auto request_id = edit_order.edit_status.cli_ord_id;
              OrderUpdate{shared_, stream_id_, security_.get_account()}(
                  order_id,
                  edit_order,
                  [&](auto &order_update) {
                    log::debug("order_update={}"sv, order_update);
                    oms::Response response{
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
                    shared_.update_order(
                        user_id,
                        order_id,
                        stream_id_,
                        trace_info,
                        response,
                        order_update,
                        []([[maybe_unused]] auto &order) {});
                  },
                  [&](auto error, auto text) {
                    oms::Response response{
                        .type = RequestType::MODIFY_ORDER,
                        .origin = Origin::EXCHANGE,
                        .status = RequestStatus::REJECTED,
                        .error = error,
                        .text = text,
                        .version = version,
                        .request_id = request_id,
                        .quantity = NaN,
                        .price = NaN,
                    };
                    shared_.update_order(
                        user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
                  });
              break;
            }
          }
          break;
        }
        case CLIENT_ERROR: {
          core::json::Buffer buffer(decode_buffer_);
          auto error = core::json::Parser::create<json::RestError>(body, buffer);
          log::warn("error={}"sv, error);
          auto text = std::size(error.errors) > 0 ? error.errors[0].message : error.message;
          oms::Response response{
              .type = RequestType::MODIFY_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = Error::UNKNOWN,
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          shared_.update_order(
              user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
          break;
        }
        default:
          response.expect(web::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::MODIFY_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn("Did not find order: user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
      }
    }
  });
}

// cancel-order

void OrderEntry::cancel_order(
    Event<CancelOrder> const &event,
    oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto &[message_info, cancel_order] = event;
    auto method = web::http::Method::POST;
    auto path = "/api/v3/cancelorder"sv;
    auto query = fmt::format("?order_id={}"sv, order.external_order_id);
    log::debug(R"(query="{}")"sv, query);
    auto headers = security_.create_headers(path, query);
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = web::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    (*connection_)(
        request_id,
        request,
        [this, user_id = message_info.source, order_id = cancel_order.order_id, version = cancel_order.version](
            [[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          Trace event(trace_info, response);
          cancel_order_ack(event, user_id, order_id, version);
        });
  });
}

void OrderEntry::cancel_order_ack(
    Trace<web::rest::Response const> const &event, uint8_t user_id, uint32_t order_id, uint32_t version) {
  profile_.cancel_order_ack([&]() {
    // auto &[trace_info, response] = event; // XXX clang13
    auto &trace_info = event.trace_info;
    auto &response = event.value;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (category) {
        using enum web::http::Category;
        case SUCCESS: {
          core::json::Buffer buffer(decode_buffer_);
          auto cancel_order = core::json::Parser::create<json::CancelOrder>(body, buffer);
          switch (cancel_order.result) {
            using enum json::Result::type_t;
            case UNDEFINED:
            case UNKNOWN:
              log::warn(R"(response="{}")"sv, body);
              log::fatal("Unexpected: cancel_order={}"sv, cancel_order);
              break;
            case ERROR: {
              log::warn("cancel_order={}"sv, cancel_order);
              oms::Response response{
                  .type = RequestType::CREATE_ORDER,
                  .origin = Origin::EXCHANGE,
                  .status = RequestStatus::REJECTED,
                  .error = Error::UNKNOWN,
                  .text = cancel_order.error,
                  .version = version,
                  .request_id = {},
                  .quantity = NaN,
                  .price = NaN,
              };
              shared_.update_order(
                  user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
              break;
            }
            case SUCCESS: {
              auto request_id = cancel_order.cancel_status.cli_ord_id;
              OrderUpdate{shared_, stream_id_, security_.get_account()}(
                  order_id,
                  cancel_order,
                  [&](auto &order_update) {
                    log::debug("order_update={}"sv, order_update);
                    oms::Response response{
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
                    shared_.update_order(
                        user_id,
                        order_id,
                        stream_id_,
                        trace_info,
                        response,
                        order_update,
                        []([[maybe_unused]] auto &order) {});
                  },
                  [&](auto error, auto text) {
                    oms::Response response{
                        .type = RequestType::CANCEL_ORDER,
                        .origin = Origin::EXCHANGE,
                        .status = RequestStatus::REJECTED,
                        .error = error,
                        .text = text,
                        .version = version,
                        .request_id = request_id,
                        .quantity = NaN,
                        .price = NaN,
                    };
                    shared_.update_order(
                        user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
                  });
              break;
            }
          }
          break;
        }
        case CLIENT_ERROR: {
          core::json::Buffer buffer(decode_buffer_);
          auto error = core::json::Parser::create<json::RestError>(body, buffer);
          log::warn("error={}"sv, error);
          auto text = std::size(error.errors) > 0 ? error.errors[0].message : error.message;
          oms::Response response{
              .type = RequestType::CANCEL_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = Error::UNKNOWN,
              .text = text,
              .version = version,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          shared_.update_order(
              user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
          break;
        }
        default:
          response.expect(web::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      oms::Response response{
          .type = RequestType::CANCEL_ORDER,
          .origin = Origin::GATEWAY,
          .status = e.request_status(),
          .error = e.error(),
          .text = e.what(),
          .version = version,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      if (shared_.update_order(
              user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {})) {
      } else {
        log::warn("Did not find order: user_id={}, order_id={}, version={}"sv, user_id, order_id, version);
      }
    }
  });
}

// cancel-all-orders

void OrderEntry::cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id) {
  profile_.cancel_all_orders([&]() {
    if (!ready())
      throw oms::NotReady("not ready"sv);
    auto method = web::http::Method::POST;
    auto path = "/api/v3/cancelallorders"sv;
    auto headers = security_.create_headers(path, {});
    web::rest::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = web::http::Accept::JSON,
        .content_type = {},
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
    };
    (*connection_)(request_id, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
      auto trace_info = server::create_trace_info();
      Trace event(trace_info, response);
      cancel_all_orders_ack(event);
    });
  });
}

void OrderEntry::cancel_all_orders_ack(Trace<web::rest::Response const> const &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (status) {
        using enum web::http::Status;
        case OK: {  // 200
          core::json::Buffer buffer(decode_buffer_);
          auto cancel_all_orders = core::json::Parser::create<json::CancelAllOrders>(body, buffer);
          log::info("*** CANCELED {} ORDER(S) ***"sv, std::size(cancel_all_orders.cancel_status.order_events));
          break;
        }
        case BAD_REQUEST:   // 400
        case UNAUTHORIZED:  // 401
        case FORBIDDEN:     // 403
        case NOT_FOUND: {   // 404
          core::json::Buffer buffer(decode_buffer_);
          auto rest_error = core::json::Parser::create<json::RestError>(body, buffer);
          log::warn("error={}"sv, rest_error);
          // note! this event does not require an ack
          break;
        }
        default:
          response.expect(web::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      // note! this event does not require an ack
    }
  });
}

// cancel-all-orders-after

void OrderEntry::cancel_all_orders_after(std::chrono::nanoseconds timeout) {
  auto value = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
  auto method = web::http::Method::POST;
  auto path = "/api/v3/cancelallordersafter"sv;
  auto query = fmt::format("?timeout={}"sv, value.count());
  auto headers = security_.create_headers(path, query);
  web::rest::Request request{
      .method = method,
      .path = path,
      .query = query,
      .accept = web::http::Accept::JSON,
      .content_type = {},
      .headers = headers,
      .body = {},
      .quality_of_service = get_quality_of_service(),
  };
  (*connection_)("cancel_all_orders_after"sv, request, [this]([[maybe_unused]] auto &request_id, auto &response) {
    auto trace_info = server::create_trace_info();
    Trace event(trace_info, response);
    cancel_all_orders_after_ack(event);
  });
}

void OrderEntry::cancel_all_orders_after_ack(Trace<web::rest::Response const> const &event) {
  profile_.cancel_all_orders_ack([&]() {
    auto &[trace_info, response] = event;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      switch (status) {
        using enum web::http::Status;
        case OK: {  // 200
          core::json::Buffer buffer(decode_buffer_);
          auto cancel_all_orders_after_ack = core::json::Parser::create<json::CancelAllAfterAck>(body, buffer);
          log::debug("cancel_all_orders_after_ack={}"sv, cancel_all_orders_after_ack);
          log::info<2>("cancel_all_orders_after_ack={}"sv, cancel_all_orders_after_ack);
          break;
        }
        case BAD_REQUEST:   // 400
        case UNAUTHORIZED:  // 401
        case FORBIDDEN:     // 403
        case NOT_FOUND: {   // 404
          core::json::Buffer buffer(decode_buffer_);
          auto cancel_all_orders_after_ack = core::json::Parser::create<json::CancelAllAfterAck>(body, buffer);
          log::debug("cancel_all_orders_after_ack={}"sv, cancel_all_orders_after_ack);
          log::warn<2>("cancel_all_orders_after_ack={}"sv, cancel_all_orders_after_ack);
          break;
        }
        default:
          response.expect(web::http::Status::OK);  // throws
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      // note! this event does not require an ack
    }
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

}  // namespace kraken_futures
}  // namespace roq

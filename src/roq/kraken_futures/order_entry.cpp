/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/order_entry.h"

#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"
#include "roq/kraken_futures/order_update.h"

#include "roq/kraken_futures/json/cancel_all_after_ack.h"
#include "roq/kraken_futures/json/cancel_all_orders.h"
#include "roq/kraken_futures/json/cancel_order.h"
#include "roq/kraken_futures/json/edit_order.h"
#include "roq/kraken_futures/json/rest_error.h"
#include "roq/kraken_futures/json/result.h"
#include "roq/kraken_futures/json/send_order.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

namespace {
static const auto NAME = "om"_sv;

static const auto SUPPORTS = utils::Mask{
    SupportType::CREATE_ORDER,
    SupportType::MODIFY_ORDER,
    SupportType::CANCEL_ORDER,
    SupportType::ORDER_ACK,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

static auto get_quality_of_service() {
  return Flags::rest_allow_order_request_pipeline() ? core::web::QualityOfService::IMMEDIATE
                                                    : core::web::QualityOfService::CRITICAL;
}
}  // namespace

OrderEntry::OrderEntry(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared,
    bool master)
    : handler_(handler), stream_id_(stream_id),
      name_(fmt::format("{}:{}:{}"_sv, stream_id_, NAME, security.get_account())), master_(master),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_rate_limit_interval(),
          Flags::rest_rate_limit_max_requests(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .create_order = create_metrics(name_, "create_order"_sv),
          .create_order_ack = create_metrics(name_, "create_order_ack"_sv),
          .modify_order = create_metrics(name_, "modify_order"_sv),
          .modify_order_ack = create_metrics(name_, "modify_order_ack"_sv),
          .cancel_order = create_metrics(name_, "cancel_order"_sv),
          .cancel_order_ack = create_metrics(name_, "cancel_order_ack"_sv),
          .cancel_all_orders = create_metrics(name_, "cancel_all_orders"_sv),
          .cancel_all_orders_ack = create_metrics(name_, "cancel_all_orders_ack"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
      },
      security_(security), shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void OrderEntry::operator()(const Event<Start> &) {
  connection_.start();
}

void OrderEntry::operator()(const Event<Stop> &) {
  connection_.stop();
}

void OrderEntry::operator()(const Event<Timer> &event) {
  auto now = event.value.now;
  connection_.refresh(now);
  if (Flags::rest_cancel_on_disconnect() && Flags::rest_cancel_all_after().count() && ready() &&
      next_cancel_all_timer_ < now) {
    next_cancel_all_timer_ = now + Flags::rest_cancel_all_after() / 4;
    cancel_all_after(Flags::rest_cancel_all_after());
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
    OrderType order_type,
    TimeInForce time_in_force,
    ExecutionInstruction execution_instruction,
    double stop_price) {
  if (time_in_force == TimeInForce::IOC)
    return json::OrderEventOrderType::IOC;
  switch (order_type) {
    case roq::OrderType::UNDEFINED:
      break;
    case roq::OrderType::MARKET:
      return json::OrderEventOrderType::MKT;
    case roq::OrderType::LIMIT:
      if (std::isnan(stop_price))
        return json::OrderEventOrderType::LMT;
      else
        return json::OrderEventOrderType::STP;
      break;
  }
  throw RuntimeErrorException(
      "Unexpected combination of order_type={}, time_in_force={}, execution_instruction={}, stop_price={}"_sv,
      order_type,
      time_in_force,
      execution_instruction,
      stop_price);
}
}  // namespace

uint16_t OrderEntry::operator()(
    const Event<CreateOrder> &event, const std::string_view &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw oms::NotReadyException();
    auto &[message_info, create_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v3/sendorder"_sv;
    auto order_type = compute_order_type(
        create_order.order_type,
        create_order.time_in_force,
        create_order.execution_instruction,
        create_order.stop_price);
    auto side = json::map(create_order.side);
    auto reduce_only = create_order.execution_instruction == ExecutionInstruction::DO_NOT_INCREASE;
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
            "&reduceOnly={}"_sv,
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
            "&reduceOnly={}"_sv,
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
          "&reduceOnly={}"_sv,
          order_type.as_raw_text(),
          create_order.symbol,
          side.as_raw_text(),
          create_order.quantity,
          request_id,
          reduce_only);
    }
    log::debug(R"(query="{}")"_sv, query);
    auto headers = security_.create_headers(path, query);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
        .rate_limit_weight = 1,
    };
    connection_(
        request,
        [this, user_id = message_info.source, order_id = create_order.order_id](auto &response) {
          profile_.create_order_ack([&]() { create_order_ack(response, user_id, order_id); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<ModifyOrder> &event,
    const oms::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw oms::NotReadyException();
    auto &[message_info, modify_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v3/editorder"_sv;
    // XXX HANS price has max 2 decimals, size is integer
    auto query = fmt::format(
        "?orderId={}"
        "&size={}"
        "&limitPrice={}"_sv,
        order.external_order_id,
        modify_order.quantity,
        modify_order.price);
    log::debug(R"(query="{}")"_sv, query);
    auto headers = security_.create_headers(path, query);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
        .rate_limit_weight = 1,
    };
    connection_(
        request,
        [this,
         user_id = message_info.source,
         order_id = modify_order.order_id,
         version = modify_order.version](auto &response) {
          profile_.modify_order_ack(
              [&]() { modify_order_ack(response, user_id, order_id, version); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(
    const Event<CancelOrder> &event,
    const oms::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw oms::NotReadyException();
    auto &[message_info, cancel_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v3/cancelorder"_sv;
    auto query = fmt::format("?order_id={}"_sv, order.external_order_id);
    log::debug(R"(query="{}")"_sv, query);
    auto headers = security_.create_headers(path, query);
    core::web::Request request{
        .method = method,
        .path = path,
        .query = query,
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
        .rate_limit_weight = 1,
    };
    connection_(
        request,
        [this,
         user_id = message_info.source,
         order_id = cancel_order.order_id,
         version = cancel_order.version](auto &response) {
          profile_.cancel_order_ack(
              [&]() { cancel_order_ack(response, user_id, order_id, version); });
        });
  });
  return stream_id_;
}

uint16_t OrderEntry::operator()(const Event<CancelAllOrders> &) {
  profile_.cancel_all_orders([&]() {
    if (!ready())
      throw oms::NotReadyException();
    auto method = core::http::Method::POST;
    auto path = "/api/v3/cancelallorders"_sv;
    auto headers = security_.create_headers(path, {});
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = core::http::ContentType::JSON,
        .headers = headers,
        .body = {},
        .quality_of_service = get_quality_of_service(),
        .rate_limit_weight = 1,
    };
    connection_(request, [this](auto &response) {
      profile_.cancel_all_orders_ack([&]() { cancel_all_orders_ack(response); });
    });
  });
  return stream_id_;
}

void OrderEntry::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

void OrderEntry::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void OrderEntry::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void OrderEntry::operator()(const core::web::Client::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void OrderEntry::create_order_ack(
    const core::web::Response &response, const uint8_t user_id, const uint32_t order_id) {
  server::TraceInfo trace_info;
  switch (response.category()) {
    case core::http::Category::SUCCESS: {
      auto body = response.body();
      core::json::Buffer buffer(decode_buffer_);
      auto send_order = core::json::Parser::create<json::SendOrder>(body, buffer);
      switch (send_order.result) {
        case json::Result::UNDEFINED:
        case json::Result::UNKNOWN:
          log::warn(R"(response="{}")"_sv, body);
          log::fatal("Unexpected: send_order={}"_sv, send_order);
          break;
        case json::Result::ERROR: {
          log::warn("send_order={}"_sv, send_order);
          oms::Response response{
              .type = RequestType::CREATE_ORDER,
              .origin = Origin::EXCHANGE,
              .status = RequestStatus::REJECTED,
              .error = Error::UNKNOWN,
              .text = send_order.error,
              .version = 1,
              .request_id = {},
              .quantity = NaN,
              .price = NaN,
          };
          shared_.update_order(
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {});
        } break;
        case json::Result::SUCCESS: {
          auto request_id = send_order.send_status.cli_ord_id;
          OrderUpdate{shared_, stream_id_, security_.get_account()}(
              order_id,
              send_order,
              [&](auto &order_update) {
                log::debug("order_update={}"_sv, order_update);
                oms::Response response{
                    .type = RequestType::CREATE_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::ACCEPTED,
                    .error = {},
                    .text = {},
                    .version = 1,
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
                    .version = 1,
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
                    []([[maybe_unused]] auto &order) {});
              });
          break;
        }
      }
      break;
    }
    case core::http::Category::CLIENT_ERROR: {
      auto body = response.body();
      core::json::Buffer buffer(decode_buffer_);
      auto error = core::json::Parser::create<json::RestError>(body, buffer);
      log::warn("error={}"_sv, error);
      auto text = std::size(error.errors) > 0 ? error.errors[0].message : error.message;
      oms::Response response{
          .type = RequestType::CREATE_ORDER,
          .origin = Origin::EXCHANGE,
          .status = RequestStatus::REJECTED,
          .error = Error::UNKNOWN,
          .text = text,
          .version = 1,
          .request_id = {},
          .quantity = NaN,
          .price = NaN,
      };
      shared_.update_order(
          user_id, order_id, stream_id_, trace_info, response, []([[maybe_unused]] auto &order) {});
      break;
    }
    default:
      response.expect(core::http::Status::OK);  // throws
  }
}

void OrderEntry::modify_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  server::TraceInfo trace_info;
  switch (response.category()) {
    case core::http::Category::SUCCESS: {
      auto body = response.body();
      core::json::Buffer buffer(decode_buffer_);
      auto edit_order = core::json::Parser::create<json::EditOrder>(body, buffer);
      switch (edit_order.result) {
        case json::Result::UNDEFINED:
        case json::Result::UNKNOWN:
          log::warn(R"(response="{}")"_sv, body);
          log::fatal("Unexpected: edit_order={}"_sv, edit_order);
          break;
        case json::Result::ERROR: {
          log::warn("edit_order={}"_sv, edit_order);
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
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {});
          break;
        }
        case json::Result::SUCCESS: {
          auto request_id = edit_order.edit_status.cli_ord_id;
          OrderUpdate{shared_, stream_id_, security_.get_account()}(
              order_id,
              edit_order,
              [&](auto &order_update) {
                log::debug("order_update={}"_sv, order_update);
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
                    user_id,
                    order_id,
                    stream_id_,
                    trace_info,
                    response,
                    []([[maybe_unused]] auto &order) {});
              });
          break;
        }
      }
      break;
    }
    case core::http::Category::CLIENT_ERROR: {
      auto body = response.body();
      core::json::Buffer buffer(decode_buffer_);
      auto error = core::json::Parser::create<json::RestError>(body, buffer);
      log::warn("error={}"_sv, error);
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
      response.expect(core::http::Status::OK);  // throws
  }
}

void OrderEntry::cancel_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  server::TraceInfo trace_info;
  switch (response.category()) {
    case core::http::Category::SUCCESS: {
      auto body = response.body();
      core::json::Buffer buffer(decode_buffer_);
      auto cancel_order = core::json::Parser::create<json::CancelOrder>(body, buffer);
      switch (cancel_order.result) {
        case json::Result::UNDEFINED:
        case json::Result::UNKNOWN:
          log::warn(R"(response="{}")"_sv, body);
          log::fatal("Unexpected: cancel_order={}"_sv, cancel_order);
          break;
        case json::Result::ERROR: {
          log::warn("cancel_order={}"_sv, cancel_order);
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
              user_id,
              order_id,
              stream_id_,
              trace_info,
              response,
              []([[maybe_unused]] auto &order) {});
          break;
        }
        case json::Result::SUCCESS: {
          auto request_id = cancel_order.cancel_status.cli_ord_id;
          OrderUpdate{shared_, stream_id_, security_.get_account()}(
              order_id,
              cancel_order,
              [&](auto &order_update) {
                log::debug("order_update={}"_sv, order_update);
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
                    user_id,
                    order_id,
                    stream_id_,
                    trace_info,
                    response,
                    []([[maybe_unused]] auto &order) {});
              });
          break;
        }
      }
      break;
    }
    case core::http::Category::CLIENT_ERROR: {
      auto body = response.body();
      core::json::Buffer buffer(decode_buffer_);
      auto error = core::json::Parser::create<json::RestError>(body, buffer);
      log::warn("error={}"_sv, error);
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
      response.expect(core::http::Status::OK);  // throws
  }
}

void OrderEntry::cancel_all_orders_ack(const core::web::Response &response) {
  server::TraceInfo trace_info;
  try {
    switch (response.raw_status()) {
      case core::http::Status::OK: {  // 200
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto cancel_all_orders = core::json::Parser::create<json::CancelAllOrders>(body, buffer);
        log::info(
            "*** CANCELED {} ORDER(S) ***"_sv,
            std::size(cancel_all_orders.cancel_status.order_events));
        break;
      }
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND: {   // 404
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto rest_error = core::json::Parser::create<json::RestError>(body, buffer);
        log::warn("error={}"_sv, rest_error);
        // note! this event does not require an ack
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    // note! this event does not require an ack
  }
}

void OrderEntry::cancel_all_after(std::chrono::nanoseconds timeout) {
  auto value = std::chrono::duration_cast<std::chrono::milliseconds>(timeout);
  auto method = core::http::Method::POST;
  auto path = "/api/v3/cancelallordersafter"_sv;
  auto query = fmt::format("?timeout={}"_sv, value.count());
  auto headers = security_.create_headers(path, query);
  core::web::Request request{
      .method = method,
      .path = path,
      .query = query,
      .accept = core::http::Accept::JSON,
      .content_type = core::http::ContentType::JSON,
      .headers = headers,
      .body = {},
      .quality_of_service = get_quality_of_service(),
      .rate_limit_weight = 1,
  };
  connection_(request, [this](auto &response) {
    profile_.cancel_all_orders_ack([&]() { cancel_all_after_ack(response); });
  });
}

void OrderEntry::cancel_all_after_ack(const core::web::Response &response) {
  try {
    switch (response.raw_status()) {
      case core::http::Status::OK: {  // 200
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto cancel_all_after_ack =
            core::json::Parser::create<json::CancelAllAfterAck>(body, buffer);
        log::info<3>("cancel_all_after_ack={}"_sv, cancel_all_after_ack);
        log::debug("cancel_all_after_ack={}"_sv, cancel_all_after_ack);
        break;
      }
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND: {   // 404
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto cancel_all_after_ack =
            core::json::Parser::create<json::CancelAllAfterAck>(body, buffer);
        log::warn<3>("cancel_all_after_ack={}"_sv, cancel_all_after_ack);
        log::debug("cancel_all_after_ack={}"_sv, cancel_all_after_ack);
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    // note! this event does not require an ack
  }
}

uint32_t OrderEntry::download(OrderEntryState state) {
  switch (state) {
    case OrderEntryState::UNDEFINED:
      assert(false);
      break;
    case OrderEntryState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

}  // namespace kraken_futures
}  // namespace roq

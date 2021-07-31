/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/order_entry.h"

#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"
#include "roq/kraken_futures/order_update.h"

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
      name_(roq::format("{}:{}:{}"_sv, stream_id_, NAME, security.get_account())), master_(master),
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
  connection_.refresh(event.value.now);
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

uint16_t OrderEntry::operator()(
    const Event<CreateOrder> &event, const std::string_view &request_id) {
  profile_.create_order([&]() {
    if (!ready())
      throw server::OMS_ErrorException(Error::GATEWAY_NOT_READY);
    auto &[message_info, create_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v3/sendorder"_sv;
    auto order_type = "lmt"_sv;
    auto side = "buy"_sv;
    auto reduce_only = false;
    auto query = roq::format(
        "?orderType={}"
        "&symbol={}"
        "&side={}"
        "&size={}"
        "&limitPrice={}"
        "&stopPrice={}"
        "&cliOrdId={}"
        "&reduceOnly={}"_sv,
        order_type,
        create_order.symbol,
        side,
        create_order.quantity,
        create_order.price,
        create_order.stop_price,
        request_id,
        reduce_only);
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
    const server::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.modify_order([&]() {
    if (!ready())
      throw server::OMS_ErrorException(Error::GATEWAY_NOT_READY);
    auto &[message_info, modify_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v3/editorder"_sv;
    // XXX HANS price has max 2 decimals, size is integer
    auto query = roq::format(
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
    const server::Order &order,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const std::string_view &previous_request_id) {
  profile_.cancel_order([&]() {
    if (!ready())
      throw server::OMS_ErrorException(Error::GATEWAY_NOT_READY);
    auto &[message_info, cancel_order] = event;
    auto method = core::http::Method::POST;
    auto path = "/api/v3/cancelorder"_sv;
    auto query = roq::format("?order_id={}"_sv, order.external_order_id);
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

uint16_t OrderEntry::operator()(const Event<CancelAllOrders> &event) {
  profile_.cancel_all_orders([&]() {
    if (!ready())
      throw server::OMS_ErrorException(Error::GATEWAY_NOT_READY);
    auto &[message_info, cancel_all_orders] = event;
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
  try {
    switch (response.raw_status()) {
      case core::http::Status::OK: {  // 200
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto send_order = core::json::Parser::create<json::SendOrder>(body, buffer);
        OrderUpdate{shared_, stream_id_, security_.get_account()}(send_order, trace_info, order_id);
        break;
      }
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND: {   // 404
        std::string_view text;
        auto body = response.body();
        auto rest_error = core::json::Parser::create<json::RestError>(body);
        log::warn("error={}"_sv, rest_error);
        server::Ack ack{
            .stream_id = stream_id_,
            .account = security_.get_account(),
            .order_id = order_id,
            .type = RequestType::CREATE_ORDER,
            .origin = Origin::EXCHANGE,
            .status = RequestStatus::REJECTED,
            .error = Error::UNKNOWN,
            .text = rest_error.message,
            .version = {},
            .request_id = {},
        };
        server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::Ack ack{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .order_id = order_id,
        .type = RequestType::CREATE_ORDER,
        .origin = Origin::GATEWAY,
        .status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,
        .text = e.what(),
        .version = 1,  // XXX HANS allow 0
        .request_id = {},
    };
    server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
  } catch (Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::Ack ack{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .order_id = order_id,
        .type = RequestType::CREATE_ORDER,
        .origin = Origin::EXCHANGE,
        .status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,
        .text = e.what(),
        .version = 1,  // XXX HANS allow 0
        .request_id = {},
    };
    server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
  }
  // XXX HANS what about OMS_Error?
}

void OrderEntry::modify_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  server::TraceInfo trace_info;
  try {
    switch (response.raw_status()) {
      case core::http::Status::OK: {  // 200
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto edit_order = core::json::Parser::create<json::EditOrder>(body, buffer);
        OrderUpdate{shared_, stream_id_, security_.get_account()}(edit_order, trace_info, order_id);
        break;
      }
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND: {   // 404
        auto body = response.body();
        auto rest_error = core::json::Parser::create<json::RestError>(body);
        log::warn("error={}"_sv, rest_error);
        server::Ack ack{
            .stream_id = stream_id_,
            .account = security_.get_account(),
            .order_id = order_id,
            .type = RequestType::MODIFY_ORDER,
            .origin = Origin::EXCHANGE,
            .status = RequestStatus::REJECTED,
            .error = Error::UNKNOWN,
            .text = rest_error.message,
            .version = version,
            .request_id = {},
        };
        server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::Ack ack{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .order_id = order_id,
        .type = RequestType::MODIFY_ORDER,
        .origin = Origin::GATEWAY,
        .status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,
        .text = e.what(),
        .version = version,
        .request_id = {},
    };
    server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
  } catch (Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::Ack ack{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .order_id = order_id,
        .type = RequestType::MODIFY_ORDER,
        .origin = Origin::EXCHANGE,
        .status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,
        .text = e.what(),
        .version = version,
        .request_id = {},
    };
    server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
  }
  // XXX HANS what about OMS_Error?
}

void OrderEntry::cancel_order_ack(
    const core::web::Response &response,
    const uint8_t user_id,
    const uint32_t order_id,
    const uint32_t version) {
  server::TraceInfo trace_info;
  try {
    switch (response.raw_status()) {
      case core::http::Status::OK: {  // 200
        auto body = response.body();
        core::json::Buffer buffer(decode_buffer_);
        auto cancel_order = core::json::Parser::create<json::CancelOrder>(body, buffer);
        OrderUpdate{shared_, stream_id_, security_.get_account()}(
            cancel_order, trace_info, order_id);
        break;
      }
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND: {   // 404
        auto body = response.body();
        auto rest_error = core::json::Parser::create<json::RestError>(body);
        log::warn("error={}"_sv, rest_error);
        server::Ack ack{
            .stream_id = stream_id_,
            .account = security_.get_account(),
            .order_id = order_id,
            .type = RequestType::CANCEL_ORDER,
            .origin = Origin::EXCHANGE,
            .status = RequestStatus::REJECTED,
            .error = Error::UNKNOWN,
            .text = rest_error.message,
            .version = version,
            .request_id = {},
        };
        server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
        break;
      }
      default:
        response.expect(core::http::Status::OK);  // throws
    }
  } catch (NetworkError &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::Ack ack{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .order_id = order_id,
        .type = RequestType::CANCEL_ORDER,
        .origin = Origin::GATEWAY,
        .status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,
        .text = e.what(),
        .version = version,
        .request_id = {},
    };
    server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
  } catch (Exception &e) {
    log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
    server::Ack ack{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .order_id = order_id,
        .type = RequestType::CANCEL_ORDER,
        .origin = Origin::EXCHANGE,
        .status = RequestStatus::REJECTED,
        .error = Error::UNKNOWN,
        .text = e.what(),
        .version = version,
        .request_id = {},
    };
    server::create_trace_and_dispatch(trace_info, ack, shared_, true, user_id);
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
        break;
      }
      case core::http::Status::BAD_REQUEST:   // 400
      case core::http::Status::UNAUTHORIZED:  // 401
      case core::http::Status::FORBIDDEN:     // 403
      case core::http::Status::NOT_FOUND: {   // 404
        auto body = response.body();
        auto rest_error = core::json::Parser::create<json::RestError>(body);
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

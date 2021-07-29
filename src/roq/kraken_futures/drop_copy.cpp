/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/drop_copy.h"

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

namespace {
static const auto NAME = "ex"_sv;
static const auto SUPPORTS = utils::Mask<SupportType>{};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

DropCopy::DropCopy(
    Handler &handler,
    core::io::Context &context,
    uint16_t stream_id,
    Security &security,
    Shared &shared)
    : handler_(handler), stream_id_(stream_id),
      name_(roq::format("{}:{}:{}"_sv, stream_id_, NAME, security.get_account())),
      connection_(
          *this,
          context,
          core::URI(Flags::ws_uri()),
          {},  // query
          Flags::ws_ping_freq(),
          Flags::decode_buffer_size(),  // XXX need read buffer size
          Flags::encode_buffer_size(),
          []() { return std::string(); }),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"_sv),
          .challenge = create_metrics(name_, "challenge"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
          .account_balances_and_margins = create_metrics(name_, "account_balances_and_margins"_sv),
          .open_positions = create_metrics(name_, "open_positions"_sv),
          .open_orders_snapshot = create_metrics(name_, "open_orders_snapshot"_sv),
          .open_orders = create_metrics(name_, "open_orders"_sv),
          .fills_snapshot = create_metrics(name_, "fills_snapshot"_sv),
          .fills = create_metrics(name_, "fills"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
      },
      security_(security), shared_(shared),
      download_(Flags::ws_request_timeout(), [this](auto state) { return download(state); }) {
}

void DropCopy::operator()(const Event<Start> &) {
  connection_.start();
}

void DropCopy::operator()(const Event<Stop> &) {
  connection_.stop();
}

void DropCopy::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void DropCopy::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.challenge, metrics::PROFILE)
      .write(profile_.heartbeat, metrics::PROFILE)
      .write(profile_.account_balances_and_margins, metrics::PROFILE)
      .write(profile_.open_positions, metrics::PROFILE)
      .write(profile_.open_orders_snapshot, metrics::PROFILE)
      .write(profile_.open_orders, metrics::PROFILE)
      .write(profile_.fills_snapshot, metrics::PROFILE)
      .write(profile_.fills, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void DropCopy::get_challenge() {
  assert(original_challenge_.empty());
  assert(signed_challenge_.empty());
  auto message = roq::format(
      R"({{)"
      R"("event":"challenge",)"
      R"("api_key":"{}")"
      R"(}})"_sv,
      security_.get_key());
  log::debug(R"(request="{}")"_sv, message);
  log::info<3>(R"(request="{}")"_sv, message);
  connection_.send_text(message);
}

void DropCopy::subscribe() {
  subscribe("account_balances_and_margins"_sv);
  subscribe("open_positions"_sv);
  subscribe("open_orders"_sv);
  subscribe("fills"_sv);
}

void DropCopy::subscribe(const std::string_view &feed) {
  auto message = roq::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}",)"
      R"("api_key":"{}",)"
      R"("original_challenge":"{}",)"
      R"("signed_challenge":"{}")"
      R"(}})"_sv,
      feed,
      security_.get_key(),
      original_challenge_,
      signed_challenge_);
  log::debug(R"(request="{}")"_sv, message);
  log::info<3>(R"(request="{}")"_sv, message);
  connection_.send_text(message);
}

void DropCopy::operator()(const core::web::Socket::Connected &) {
  // note! wait for upgrade
}

void DropCopy::operator()(const core::web::Socket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  original_challenge_.clear();
  signed_challenge_.clear();
}

void DropCopy::operator()(const core::web::Socket::Ready &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(const core::web::Socket::Close &) {
}

void DropCopy::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    case DropCopyState::UNDEFINED:
      assert(false);
      break;
    case DropCopyState::GET_CHALLENGE:
      get_challenge();
      return 1;
    case DropCopyState::SUBSCRIBE:
      subscribe();
      return {};
    case DropCopyState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::operator()(const server::Trace<json::Info> &event) {
  auto &[trace_info, info] = event;
  log::debug("info={}"_sv, info);
  log::info<1>("info={}"_sv, info);
}

void DropCopy::operator()(const server::Trace<json::Alert> &event) {
  auto &[trace_info, alert] = event;
  log::debug("alert={}"_sv, alert);
  log::warn<1>("alert={}"_sv, alert);
}

void DropCopy::operator()(const server::Trace<json::Error> &event) {
  auto &[trace_info, error] = event;
  log::debug("error={}"_sv, error);
  log::warn("error={}"_sv, error);
}

void DropCopy::operator()(const server::Trace<json::Challenge> &event) {
  profile_.challenge([&]() {
    auto &[trace_info, challenge] = event;
    log::debug("challenge={}"_sv, challenge);
    log::info<3>("challenge={}"_sv, challenge);
    assert(original_challenge_.empty());
    assert(signed_challenge_.empty());
    original_challenge_ = challenge.message;
    signed_challenge_ = security_.signed_challenge(original_challenge_);
    download_.check(DropCopyState::GET_CHALLENGE);  // note!
  });
}

void DropCopy::operator()(const server::Trace<json::Subscribed> &event) {
  auto &[trace_info, subscribed] = event;
  log::debug("subscribed={}"_sv, subscribed);
  log::info<1>("subscribed={}"_sv, subscribed);
}

void DropCopy::operator()(const server::Trace<json::Heartbeat> &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
    log::debug("heartbeat={}"_sv, heartbeat);
    log::info<3>("heartbeat={}"_sv, heartbeat);
  });
}

void DropCopy::operator()(const server::Trace<json::AccountBalancesAndMargins> &event) {
  profile_.account_balances_and_margins([&]() {
    auto &[trace_info, account_balances_and_margins] = event;
    log::debug("account_balances_and_margins={}"_sv, account_balances_and_margins);
    log::info<3>("account_balances_and_margins={}"_sv, account_balances_and_margins);
  });
}

void DropCopy::operator()(const server::Trace<json::OpenPositions> &event) {
  profile_.open_positions([&]() {
    auto &[trace_info, open_positions] = event;
    log::debug("open_positions={}"_sv, open_positions);
    log::info<3>("open_positions={}"_sv, open_positions);
  });
}

void DropCopy::operator()(const server::Trace<json::OpenOrdersSnapshot> &event) {
  profile_.open_orders_snapshot([&]() {
    auto &[trace_info, open_orders_snapshot] = event;
    log::debug("open_orders_snapshot={}"_sv, open_orders_snapshot);
    log::info<3>("open_orders_snapshot={}"_sv, open_orders_snapshot);
  });
}

void DropCopy::operator()(const server::Trace<json::OpenOrders> &event) {
  profile_.open_orders([&]() {
    auto &[trace_info, open_orders] = event;
    log::debug("open_orders={}"_sv, open_orders);
    log::info<3>("open_orders={}"_sv, open_orders);
  });
}

void DropCopy::operator()(const server::Trace<json::FillsSnapshot> &event) {
  profile_.fills_snapshot([&]() {
    auto &[trace_info, fills_snapshot] = event;
    log::debug("fills_snapshot={}"_sv, fills_snapshot);
    log::info<3>("fills_snapshot={}"_sv, fills_snapshot);
  });
}

void DropCopy::operator()(const server::Trace<json::Fills> &event) {
  profile_.fills([&]() {
    auto &[trace_info, fills] = event;
    log::debug("fills={}"_sv, fills);
    log::info<3>("fills={}"_sv, fills);
  });
}

void DropCopy::parse(const std::string_view &message) {
  profile_.parse([&]() {
    server::TraceInfo trace_info;
    core::json::Buffer buffer(decode_buffer_);
    auto result = json::ParserPrivate::dispatch(*this, message, buffer, trace_info);
    if (ROQ_UNLIKELY(!result))
      log::warn(R"(Unexpected: message="{}")"_sv, message);
  });
}

}  // namespace kraken_futures
}  // namespace roq

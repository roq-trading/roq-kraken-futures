/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/drop_copy.h"

#include <algorithm>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"
#include "roq/kraken_futures/order_update.h"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

namespace {
static const auto NAME = "ex"sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
    SupportType::FUNDS,
};

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
      name_(fmt::format("{}:{}:{}"sv, stream_id_, NAME, security.get_account())),
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
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .challenge = create_metrics(name_, "challenge"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
          .account_balances_and_margins = create_metrics(name_, "account_balances_and_margins"sv),
          .open_positions = create_metrics(name_, "open_positions"sv),
          .open_orders_snapshot = create_metrics(name_, "open_orders_snapshot"sv),
          .open_orders = create_metrics(name_, "open_orders"sv),
          .fills_snapshot = create_metrics(name_, "fills_snapshot"sv),
          .fills = create_metrics(name_, "fills"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
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
  assert(std::empty(original_challenge_));
  assert(std::empty(signed_challenge_));
  auto message = fmt::format(
      R"({{)"
      R"("event":"challenge",)"
      R"("api_key":"{}")"
      R"(}})"sv,
      security_.get_key());
  log::debug(R"(request="{}")"sv, message);
  log::info<2>(R"(request="{}")"sv, message);
  connection_.send_text(message);
}

void DropCopy::subscribe() {
  subscribe("account_balances_and_margins"sv);
  subscribe("open_positions"sv);
  subscribe("open_orders"sv);
  subscribe("fills"sv);
}

void DropCopy::subscribe(const std::string_view &feed) {
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}",)"
      R"("api_key":"{}",)"
      R"("original_challenge":"{}",)"
      R"("signed_challenge":"{}")"
      R"(}})"sv,
      feed,
      security_.get_key(),
      original_challenge_,
      signed_challenge_);
  log::debug(R"(request="{}")"sv, message);
  log::info<2>(R"(request="{}")"sv, message);
  connection_.send_text(message);
}

void DropCopy::operator()(const core::web::ClientSocket::Connected &) {
  // note! wait for upgrade
}

void DropCopy::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  original_challenge_.clear();
  signed_challenge_.clear();
}

void DropCopy::operator()(const core::web::ClientSocket::Ready &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(const core::web::ClientSocket::Close &) {
}

void DropCopy::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void DropCopy::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = security_.get_account(),
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
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
  log::debug("info={}"sv, info);
  log::info<2>("info={}"sv, info);
}

void DropCopy::operator()(const server::Trace<json::Alert> &event) {
  auto &[trace_info, alert] = event;
  log::debug("alert={}"sv, alert);
  log::warn<1>("alert={}"sv, alert);
}

void DropCopy::operator()(const server::Trace<json::Error> &event) {
  auto &[trace_info, error] = event;
  log::debug("error={}"sv, error);
  log::warn("error={}"sv, error);
}

void DropCopy::operator()(const server::Trace<json::Challenge> &event) {
  profile_.challenge([&]() {
    auto &[trace_info, challenge] = event;
    log::debug("challenge={}"sv, challenge);
    log::info<2>("challenge={}"sv, challenge);
    assert(std::empty(original_challenge_));
    assert(std::empty(signed_challenge_));
    original_challenge_ = challenge.message;
    signed_challenge_ = security_.signed_challenge(original_challenge_);
    download_.check(DropCopyState::GET_CHALLENGE);  // note!
  });
}

void DropCopy::operator()(const server::Trace<json::Subscribed> &event) {
  auto &[trace_info, subscribed] = event;
  log::debug("subscribed={}"sv, subscribed);
  log::info<2>("subscribed={}"sv, subscribed);
}

void DropCopy::operator()(const server::Trace<json::Heartbeat> &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
    log::debug("heartbeat={}"sv, heartbeat);
    log::info<2>("heartbeat={}"sv, heartbeat);
  });
}

void DropCopy::operator()(const server::Trace<json::AccountBalancesAndMargins> &event) {
  profile_.account_balances_and_margins([&]() {
    auto &[trace_info, account_balances_and_margins] = event;
    log::info<2>("account_balances_and_margins={}"sv, account_balances_and_margins);
    for (auto &item : account_balances_and_margins.margin_accounts) {
      auto currency = std::string{item.name};
      std::transform(std::begin(currency), std::end(currency), std::begin(currency), ::toupper);
      FundsUpdate funds_update{
          .stream_id = stream_id_,
          .account = security_.get_account(),
          .currency = currency,
          .balance = item.balance,
          .hold = NaN,
          .external_account = account_balances_and_margins.account,
      };
      server::create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
  });
}

void DropCopy::operator()(const server::Trace<json::OpenPositions> &event) {
  profile_.open_positions([&]() {
    auto &[trace_info, open_positions] = event;
    log::info<2>("open_positions={}"sv, open_positions);
    for (auto &item : open_positions.positions) {
      auto long_quantity = std::max(0.0, item.balance);
      auto short_quantity = std::max(0.0, -item.balance);
      PositionUpdate position_update{
          .stream_id = stream_id_,
          .account = security_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.instrument,
          .external_account = open_positions.account,
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .long_quantity_begin = NaN,
          .short_quantity_begin = NaN,
      };
      server::create_trace_and_dispatch(handler_, trace_info, position_update, true);
    }
  });
}

void DropCopy::operator()(const server::Trace<json::OpenOrdersSnapshot> &event) {
  profile_.open_orders_snapshot([&]() {
    auto &[trace_info, open_orders_snapshot] = event;
    log::info<2>("open_orders_snapshot={}"sv, open_orders_snapshot);
    OrderUpdate{shared_, stream_id_, security_.get_account()}(open_orders_snapshot, trace_info);
  });
}

void DropCopy::operator()(const server::Trace<json::OpenOrders> &event) {
  profile_.open_orders([&]() {
    auto &[trace_info, open_orders] = event;
    log::info<2>("open_orders={}"sv, open_orders);
    OrderUpdate{shared_, stream_id_, security_.get_account()}(open_orders, trace_info);
  });
}

void DropCopy::operator()(const server::Trace<json::FillsSnapshot> &event) {
  profile_.fills_snapshot([&]() {
    // auto &[trace_info, fills_snapshot] = event;
    auto &trace_info = event.trace_info;
    auto &fills_snapshot = event.value;
    log::info<2>("fills_snapshot={}"sv, fills_snapshot);
    for (auto &item : fills_snapshot.fills) {
      if (shared_.find_order(item.cli_ord_id, [&](auto &order) {
            auto symbol = std::string{item.instrument};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            if (symbol.compare(order.symbol) != 0)
              log::warn(R"(Unexpected: symbol="{}"/"{}")"sv, symbol, order.symbol);
            auto side = item.buy ? Side::BUY : Side::SELL;
            if (side != order.side)
              log::warn("Unexpected: side={}/{})"sv, side, order.side);
            Fill fill{
                .external_trade_id = item.fill_id,
                .quantity = item.qty,
                .price = item.price,
                .liquidity = json::map(item.fill_type),
            };
            TradeUpdate trade_update{
                .stream_id = stream_id_,
                .account = order.account,
                .order_id = order.order_id,
                .exchange = order.exchange,
                .symbol = order.symbol,
                .side = order.side,
                .position_effect = order.position_effect,
                .create_time_utc = item.time,
                .update_time_utc = item.time,
                .external_account = fills_snapshot.account,
                .external_order_id = item.order_id,
                .fills = {&fill, 1},
                .routing_id = order.routing_id,
            };
            server::create_trace_and_dispatch(
                handler_, trace_info, trade_update, true, order.user_id);
          })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
        log::warn<2>("fill={}"sv, item);
      }
    }
  });
}

void DropCopy::operator()(const server::Trace<json::Fills> &event) {
  profile_.fills([&]() {
    // auto &[trace_info, fills] = event;
    auto &trace_info = event.trace_info;
    auto &fills = event.value;
    log::info<2>("fills={}"sv, fills);
    // XXX HANS should emplace_back and try to group by order_id
    for (auto &item : fills.fills) {
      if (shared_.find_order(item.cli_ord_id, [&](auto &order) {
            auto symbol = std::string{item.instrument};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            if (symbol.compare(order.symbol) != 0)
              log::warn(R"(Unexpected: symbol="{}"/"{}")"sv, symbol, order.symbol);
            auto side = item.buy ? Side::BUY : Side::SELL;
            if (side != order.side)
              log::warn("Unexpected: side={}/{})"sv, side, order.side);
            Fill fill{
                .external_trade_id = item.fill_id,
                .quantity = item.qty,
                .price = item.price,
                .liquidity = json::map(item.fill_type),
            };
            TradeUpdate trade_update{
                .stream_id = stream_id_,
                .account = order.account,
                .order_id = order.order_id,
                .exchange = order.exchange,
                .symbol = order.symbol,
                .side = order.side,
                .position_effect = order.position_effect,
                .create_time_utc = item.time,
                .update_time_utc = item.time,
                .external_account = fills.username,  // note! appears to be account
                .external_order_id = item.order_id,
                .fills = {&fill, 1},
                .routing_id = order.routing_id,
            };
            server::create_trace_and_dispatch(
                handler_, trace_info, trade_update, true, order.user_id);
          })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
        log::warn<2>("fill={}"sv, item);
      }
    }
  });
}

void DropCopy::parse(const std::string_view &message) {
  profile_.parse([&]() {
    auto trace_info = server::create_trace_info();
    core::json::Buffer buffer(decode_buffer_);
    auto result = json::ParserPrivate::dispatch(*this, message, buffer, trace_info);
    if (!result) [[unlikely]]
      log::warn(R"(Unexpected: message="{}")"sv, message);
  });
}

}  // namespace kraken_futures
}  // namespace roq

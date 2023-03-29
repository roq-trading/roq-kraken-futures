/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/drop_copy.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/kraken_futures/flags.hpp"
#include "roq/kraken_futures/order_update.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "ex"sv;

auto const SUPPORTS = Mask{
    SupportType::ORDER,
    SupportType::TRADE,
    SupportType::POSITION,
    SupportType::FUNDS,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id, auto const &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_uri();
  auto config = web::socket::Client::Config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = {},
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .interface = {},
      .proxy = {},
      .uris = {&uri, 1},
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return web::socket::ClientFactory::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto const &group, auto const &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(
    Handler &handler, io::Context &context, uint16_t stream_id, Authenticator &authenticator, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, authenticator.get_account())},
      connection_{create_connection(*this, context)}, decode_buffer_{Flags::decode_buffer_size()},
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
      authenticator_{authenticator}, shared_{shared}, download_{Flags::ws_request_timeout(), [this](auto state) {
                                                                  return download(state);
                                                                }} {
}

void DropCopy::operator()(Event<Start> const &) {
  (*connection_).start();
}

void DropCopy::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void DropCopy::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
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
      authenticator_.get_key());
  log::debug(R"(request="{}")"sv, message);
  log::info<2>(R"(request="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::subscribe() {
  subscribe("account_balances_and_margins"sv);
  subscribe("open_positions"sv);
  subscribe("open_orders"sv);
  subscribe("fills"sv);
}

void DropCopy::subscribe(std::string_view const &feed) {
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}",)"
      R"("api_key":"{}",)"
      R"("original_challenge":"{}",)"
      R"("signed_challenge":"{}")"
      R"(}})"sv,
      feed,
      authenticator_.get_key(),
      original_challenge_,
      signed_challenge_);
  log::debug(R"(request="{}")"sv, message);
  log::info<2>(R"(request="{}")"sv, message);
  (*connection_).send_text(message);
}

void DropCopy::operator()(web::socket::Client::Connected const &) {
  // note! wait for upgrade
}

void DropCopy::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  ready_ = false;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
  original_challenge_.clear();
  signed_challenge_.clear();
}

void DropCopy::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void DropCopy::operator()(web::socket::Client::Close const &) {
}

void DropCopy::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  auto external_latency = ExternalLatency{
      .stream_id = stream_id_,
      .account = authenticator_.get_account(),
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void DropCopy::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void DropCopy::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void DropCopy::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = authenticator_.get_account(),
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
    };
    log::info("stream_status={}"sv, stream_status);
    create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
}

uint32_t DropCopy::download(DropCopyState state) {
  switch (state) {
    using enum DropCopyState;
    case UNDEFINED:
      assert(false);
      break;
    case GET_CHALLENGE:
      get_challenge();
      return 1;
    case SUBSCRIBE:
      subscribe();
      return {};
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void DropCopy::operator()(Trace<json::Info> const &event) {
  auto &[trace_info, info] = event;
  log::debug("info={}"sv, info);
  log::info<2>("info={}"sv, info);
}

void DropCopy::operator()(Trace<json::Alert> const &event) {
  auto &[trace_info, alert] = event;
  log::debug("alert={}"sv, alert);
  log::warn<1>("alert={}"sv, alert);
}

void DropCopy::operator()(Trace<json::Error> const &event) {
  auto &[trace_info, error] = event;
  log::debug("error={}"sv, error);
  log::warn("error={}"sv, error);
}

void DropCopy::operator()(Trace<json::Challenge> const &event) {
  profile_.challenge([&]() {
    auto &[trace_info, challenge] = event;
    log::debug("challenge={}"sv, challenge);
    log::info<2>("challenge={}"sv, challenge);
    assert(std::empty(original_challenge_));
    assert(std::empty(signed_challenge_));
    original_challenge_ = challenge.message;
    signed_challenge_ = authenticator_.signed_challenge(original_challenge_);
    download_.check(DropCopyState::GET_CHALLENGE);  // note!
  });
}

void DropCopy::operator()(Trace<json::Subscribed> const &event) {
  auto &[trace_info, subscribed] = event;
  log::debug("subscribed={}"sv, subscribed);
  log::info<2>("subscribed={}"sv, subscribed);
}

void DropCopy::operator()(Trace<json::Heartbeat> const &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
    log::debug("heartbeat={}"sv, heartbeat);
    log::info<2>("heartbeat={}"sv, heartbeat);
  });
}

void DropCopy::operator()(Trace<json::AccountBalancesAndMargins> const &event) {
  profile_.account_balances_and_margins([&]() {
    auto &[trace_info, account_balances_and_margins] = event;
    log::info<2>("account_balances_and_margins={}"sv, account_balances_and_margins);
    for (auto &item : account_balances_and_margins.margin_accounts) {
      auto currency = std::string{item.name};
      std::transform(std::begin(currency), std::end(currency), std::begin(currency), ::toupper);
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = authenticator_.get_account(),
          .currency = currency,
          .balance = item.balance,
          .hold = NaN,
          .external_account = account_balances_and_margins.account,
      };
      create_trace_and_dispatch(handler_, trace_info, funds_update, true);
    }
  });
}

void DropCopy::operator()(Trace<json::OpenPositions> const &event) {
  profile_.open_positions([&]() {
    auto &[trace_info, open_positions] = event;
    log::info<2>("open_positions={}"sv, open_positions);
    for (auto &item : open_positions.positions) {
      auto long_quantity = std::max(0.0, item.balance);
      auto short_quantity = std::max(0.0, -item.balance);
      auto position_update = PositionUpdate{
          .stream_id = stream_id_,
          .account = authenticator_.get_account(),
          .exchange = Flags::exchange(),
          .symbol = item.instrument,
          .external_account = open_positions.account,
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .long_quantity_begin = NaN,
          .short_quantity_begin = NaN,
      };
      create_trace_and_dispatch(handler_, trace_info, position_update, true);
    }
  });
}

void DropCopy::operator()(Trace<json::OpenOrdersSnapshot> const &event) {
  profile_.open_orders_snapshot([&]() {
    auto &[trace_info, open_orders_snapshot] = event;
    log::info<2>("open_orders_snapshot={}"sv, open_orders_snapshot);
    OrderUpdate{shared_, stream_id_, authenticator_.get_account()}(open_orders_snapshot, trace_info);
  });
}

void DropCopy::operator()(Trace<json::OpenOrders> const &event) {
  profile_.open_orders([&]() {
    auto &[trace_info, open_orders] = event;
    log::info<2>("open_orders={}"sv, open_orders);
    OrderUpdate{shared_, stream_id_, authenticator_.get_account()}(open_orders, trace_info);
  });
}

void DropCopy::operator()(Trace<json::FillsSnapshot> const &event) {
  profile_.fills_snapshot([&]() {
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
            auto fill = Fill{
                .external_trade_id = item.fill_id,
                .quantity = item.qty,
                .price = item.price,
                .liquidity = json::map(item.fill_type),
            };
            auto trade_update = oms::TradeUpdate{
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
                .update_type = {},
            };
            create_trace_and_dispatch(handler_, trace_info, trade_update, stream_id_, true, order.user_id);
          })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
        log::warn<2>("fill={}"sv, item);
      }
    }
  });
}

void DropCopy::operator()(Trace<json::Fills> const &event) {
  profile_.fills([&]() {
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
            auto fill = Fill{
                .external_trade_id = item.fill_id,
                .quantity = item.qty,
                .price = item.price,
                .liquidity = json::map(item.fill_type),
            };
            auto trade_update = oms::TradeUpdate{
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
                .update_type = {},
            };
            create_trace_and_dispatch(handler_, trace_info, trade_update, stream_id_, true, order.user_id);
          })) {
      } else {
        log::warn<1>("*** EXTERNAL ORDER ***"sv);
        log::warn<2>("fill={}"sv, item);
      }
    }
  });
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    TraceInfo trace_info;
    core::json::Buffer buffer{decode_buffer_};
    auto result = json::ParserPrivate::dispatch(*this, message, buffer, trace_info);
    if (!result)
      log::warn(R"(Unexpected: message="{}")"sv, message);
  });
}

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/kraken_futures/drop_copy.hpp"

#include <algorithm>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

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
auto create_name(auto stream_id, auto &account) {
  return fmt::format("{}:{}:{}"sv, stream_id, NAME, account);
}

auto create_connection(auto &handler, auto &settings, auto &context) {
  auto uri = settings.ws.uri;
  auto config = web::socket::Client::Config{
      // connection
      .interface = {},
      .uris = {&uri, 1},
      .host = {},
      .validate_certificate = settings.net.tls_validate_certificate,
      // connection manager
      .connection_timeout = settings.net.connection_timeout,
      .disconnect_on_idle_timeout = {},
      .always_reconnect = true,
      // proxy
      .proxy = {},
      // http
      .query = {},
      .user_agent = ROQ_PACKAGE_NAME,
      .request_timeout = {},
      .ping_frequency = settings.ws.ping_freq,
      // implementation
      .decode_buffer_size = settings.misc.decode_buffer_size,
      .encode_buffer_size = settings.misc.encode_buffer_size,
  };
  return web::socket::Client::create(handler, context, config, []() { return std::string(); });
}

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(auto &settings, auto const &group, auto const &function) : core::metrics::Factory(settings.app.name, group, function) {}
};
}  // namespace

// === IMPLEMENTATION ===

DropCopy::DropCopy(Handler &handler, io::Context &context, uint16_t stream_id, Account &account, Shared &shared)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_, account.name)}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .challenge = create_metrics(shared.settings, name_, "challenge"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
          .account_balances_and_margins = create_metrics(shared.settings, name_, "account_balances_and_margins"sv),
          .open_positions = create_metrics(shared.settings, name_, "open_positions"sv),
          .open_orders_snapshot = create_metrics(shared.settings, name_, "open_orders_snapshot"sv),
          .open_orders = create_metrics(shared.settings, name_, "open_orders"sv),
          .fills_snapshot = create_metrics(shared.settings, name_, "fills_snapshot"sv),
          .fills = create_metrics(shared.settings, name_, "fills"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
      },
      account_{account}, shared_{shared}, download_{shared.settings.ws.request_timeout, [this](auto state) { return download(state); }} {
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
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.challenge, metrics::Type::PROFILE)
      .write(profile_.heartbeat, metrics::Type::PROFILE)
      .write(profile_.account_balances_and_margins, metrics::Type::PROFILE)
      .write(profile_.open_positions, metrics::Type::PROFILE)
      .write(profile_.open_orders_snapshot, metrics::Type::PROFILE)
      .write(profile_.open_orders, metrics::Type::PROFILE)
      .write(profile_.fills_snapshot, metrics::Type::PROFILE)
      .write(profile_.fills, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void DropCopy::get_challenge() {
  assert(std::empty(original_challenge_));
  assert(std::empty(signed_challenge_));
  auto message = fmt::format(
      R"({{)"
      R"("event":"challenge",)"
      R"("api_key":"{}")"
      R"(}})"sv,
      account_.key);
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
      account_.key,
      original_challenge_,
      signed_challenge_);
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
      .account = account_.name,
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
        .account = account_.name,
        .supports = SUPPORTS,
        .transport = Transport::TCP,
        .protocol = Protocol::WS,
        .encoding = {Encoding::JSON},
        .priority = Priority::PRIMARY,
        .connection_status = status_,
        .interface = (*connection_).get_interface(),
        .authority = (*connection_).get_current_authority(),
        .path = (*connection_).get_current_path(),
        .proxy = (*connection_).get_proxy(),
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
      return 0;
    case DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return 0;
  }
  assert(false);
  return 0;
}

void DropCopy::operator()(Trace<json::Info> const &event) {
  auto &[trace_info, info] = event;
  log::info<2>("info={}"sv, info);
}

void DropCopy::operator()(Trace<json::Alert> const &event) {
  auto &[trace_info, alert] = event;
  log::warn<1>("alert={}"sv, alert);
}

void DropCopy::operator()(Trace<json::Error> const &event) {
  auto &[trace_info, error] = event;
  log::warn("error={}"sv, error);
}

void DropCopy::operator()(Trace<json::Challenge> const &event) {
  profile_.challenge([&]() {
    auto &[trace_info, challenge] = event;
    log::info<2>("challenge={}"sv, challenge);
    assert(std::empty(original_challenge_));
    assert(std::empty(signed_challenge_));
    original_challenge_ = challenge.message;
    signed_challenge_ = account_.signed_challenge(original_challenge_);
    download_.check(DropCopyState::GET_CHALLENGE);  // note!
  });
}

void DropCopy::operator()(Trace<json::Subscribed> const &event) {
  auto &[trace_info, subscribed] = event;
  log::info<2>("subscribed={}"sv, subscribed);
}

void DropCopy::operator()(Trace<json::Heartbeat> const &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
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
          .account = account_.name,
          .currency = currency,
          .margin_mode = {},
          .balance = item.balance,
          .hold = NaN,
          .external_account = account_balances_and_margins.account,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = {},
          .sending_time_utc = {},
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
          .account = account_.name,
          .exchange = shared_.settings.exchange,
          .symbol = item.instrument,
          .margin_mode = {},
          .external_account = open_positions.account,
          .long_quantity = long_quantity,
          .short_quantity = short_quantity,
          .update_type = UpdateType::INCREMENTAL,
          .exchange_time_utc = {},
          .sending_time_utc = {},
      };
      create_trace_and_dispatch(handler_, trace_info, position_update, true);
    }
  });
}

void DropCopy::operator()(Trace<json::OpenOrdersSnapshot> const &event) {
  profile_.open_orders_snapshot([&]() {
    auto &[trace_info, open_orders_snapshot] = event;
    log::info<2>("open_orders_snapshot={}"sv, open_orders_snapshot);
    OrderUpdate{shared_, stream_id_, account_.name}(open_orders_snapshot, trace_info);
  });
}

void DropCopy::operator()(Trace<json::OpenOrders> const &event) {
  profile_.open_orders([&]() {
    auto &[trace_info, open_orders] = event;
    log::info<2>("open_orders={}"sv, open_orders);
    OrderUpdate{shared_, stream_id_, account_.name}(open_orders, trace_info);
  });
}

void DropCopy::operator()(Trace<json::FillsSnapshot> const &event) {
  profile_.fills_snapshot([&]() {
    auto &trace_info = event.trace_info;
    auto &fills_snapshot = event.value;
    log::info<2>("fills_snapshot={}"sv, fills_snapshot);
    for (auto &item : fills_snapshot.fills) {
      auto symbol = std::string{item.instrument};
      std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
      auto side = item.buy ? Side::BUY : Side::SELL;
      auto fill = Fill{
          .external_trade_id = item.fill_id,
          .quantity = item.qty,
          .price = item.price,
          .liquidity = json::map(item.fill_type),
          .quote_quantity = NaN,
          .commission_quantity = NaN,
          .commission_currency = {},
      };
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .margin_mode = {},
          .create_time_utc = item.time,
          .update_time_utc = item.time,
          .external_account = {},
          .external_order_id = item.order_id,
          .client_order_id = {},
          .fills = {&fill, 1},
          .routing_id = {},
          .update_type = UpdateType::SNAPSHOT,
          .sending_time_utc = {},
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, item.cli_ord_id);
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
      auto symbol = std::string{item.instrument};
      std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
      auto side = item.buy ? Side::BUY : Side::SELL;
      auto fill = Fill{
          .external_trade_id = item.fill_id,
          .quantity = item.qty,
          .price = item.price,
          .liquidity = json::map(item.fill_type),
          .quote_quantity = NaN,
          .commission_quantity = NaN,
          .commission_currency = {},
      };
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .margin_mode = {},
          .create_time_utc = item.time,
          .update_time_utc = item.time,
          .external_account = fills.username,  // note! appears to be account
          .external_order_id = item.order_id,
          .client_order_id = {},
          .fills = {&fill, 1},
          .routing_id = {},
          .update_type = UpdateType::INCREMENTAL,
          .sending_time_utc = {},
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, item.cli_ord_id);
    }
  });
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    TraceInfo trace_info;
    auto result = json::ParserPrivate::dispatch(*this, message, decode_buffer_, trace_info);
    if (!result)
      log::warn(R"(Unexpected: message="{}")"sv, message);
  });
}

}  // namespace kraken_futures
}  // namespace roq

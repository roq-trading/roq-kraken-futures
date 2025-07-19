/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/drop_copy.hpp"

#include <algorithm>

#include "roq/mask.hpp"

#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/kraken_futures/json/map.hpp"

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

struct create_metrics final : public utils::metrics::Factory {
  create_metrics(auto &settings, auto &group, auto const &function) : utils::metrics::Factory{settings.app.name, group, function} {}
};

// helpers

auto compute_side(int32_t direction) -> Side {
  switch (direction) {
    case -1:
      break;
    case 0:
      return Side::BUY;
    case 1:
      return Side::SELL;
    default:
      break;
  }
  return {};
}

auto compute_order_status(json::Reason reason, bool is_cancel, double remaining_quantity) -> OrderStatus {
  auto result = map(reason).template get<OrderStatus>();
  if (result != OrderStatus{}) {
    return result;
  }
  if (utils::is_zero(remaining_quantity)) {
    return OrderStatus::COMPLETED;
  }
  if (is_cancel) {
    return OrderStatus::CANCELED;
  }
  return OrderStatus::WORKING;
}
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

void DropCopy::operator()(metrics::Writer &writer) const {
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
  subscribe("account_balances_and_margins"sv);  // XXX FIXME TODO doesn't appear to exist anymore
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
    log::warn("account_balances_and_margins={}"sv, account_balances_and_margins);
    for (auto &item : account_balances_and_margins.margin_accounts) {
      auto funds_update = FundsUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .currency = item.name,
          .margin_mode = {},
          .balance = item.balance,
          .hold = NaN,
          .borrowed = NaN,
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
      fill_symbols_[item.instrument] = true;  // note!
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
    // note! workaround because zero-positions aren't reported
    for (auto &[symbol, found] : fill_symbols_) {
      if (!found) {
        auto position_update = PositionUpdate{
            .stream_id = stream_id_,
            .account = account_.name,
            .exchange = shared_.settings.exchange,
            .symbol = symbol,
            .margin_mode = {},
            .external_account = {},
            .long_quantity = 0.0,
            .short_quantity = 0.0,
            .update_type = UpdateType::INCREMENTAL,
            .exchange_time_utc = {},
            .sending_time_utc = {},
        };
        create_trace_and_dispatch(handler_, trace_info, position_update, true);
      }
      found = false;  // note!
    }
  });
}

void DropCopy::operator()(Trace<json::OpenOrdersSnapshot> const &event) {
  profile_.open_orders_snapshot([&]() {
    auto &[trace_info, open_orders_snapshot] = event;
    log::info<2>("open_orders_snapshot={}"sv, open_orders_snapshot);
    // log::warn("DEBUG open_orders_snapshot={}"sv, open_orders_snapshot);
    for (auto &order : open_orders_snapshot.orders) {
      process_order(order, order.order_id, order.cli_ord_id, {}, false, trace_info, true);
    }
  });
}

void DropCopy::operator()(Trace<json::OpenOrders> const &event) {
  profile_.open_orders([&]() {
    auto &[trace_info, open_orders] = event;
    log::info<2>("open_orders={}"sv, open_orders);
    // log::warn("DEBUG open_orders={}"sv, open_orders);
    auto &order = open_orders.order;
    auto order_id = [&]() {
      if (std::empty(open_orders.order_id)) {
        return order.order_id;
      }
      return open_orders.order_id;
    }();
    auto cli_ord_id = [&]() {
      if (std::empty(open_orders.cli_ord_id)) {
        return order.cli_ord_id;
      }
      return open_orders.cli_ord_id;
    }();
    process_order(order, order_id, cli_ord_id, open_orders.reason, open_orders.is_cancel, trace_info, false);
  });
}

void DropCopy::operator()(Trace<json::FillsSnapshot> const &event) {
  profile_.fills_snapshot([&]() {
    auto &trace_info = event.trace_info;
    auto &fills_snapshot = event.value;
    log::info<2>("fills_snapshot={}"sv, fills_snapshot);
    for (auto &item : fills_snapshot.fills) {
      fill_symbols_.try_emplace(item.instrument);  // note!
      auto side = item.buy ? Side::BUY : Side::SELL;
      auto fill = Fill{
          .external_trade_id = item.fill_id,
          .quantity = item.qty,
          .price = item.price,
          .liquidity = map(item.fill_type),
          .base_amount = NaN,
          .quote_amount = NaN,
          .commission_amount = item.fee_paid,
          .commission_currency = item.fee_currency,
          .profit_loss_cost_amount = NaN,
      };
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = shared_.settings.exchange,
          .symbol = item.instrument,
          .side = side,
          .position_effect = {},
          .margin_mode = {},
          .quantity_type = {},
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
    // log::warn("DEBUG fills={}"sv, fills);
    std::string_view symbol, external_order_id, client_order_id;
    Side side = {};
    std::chrono::milliseconds update_time_utc = {};
    double remaining_quantity = NaN;
    auto dispatch = [&]() {
      // note!
      //   here we try to catch the situation where the REST response is lost and we never get a WS update because this order was never "open"
      //   this is only the case for create order and that's why we must find the order and check for max_response_version == 0
      //   an aggressive order modification will already have had an "open" order and it will correctly be reported as completed on this channel
      if (!std::isnan(remaining_quantity) && utils::is_zero(remaining_quantity)) {
        auto is_create = false;
        shared_.find_order(client_order_id, [&](auto &order) { is_create = order.max_response_version == 0; });
        if (is_create) {
          auto order_update = server::oms::OrderUpdate{
              .account = account_.name,
              .exchange = shared_.settings.exchange,
              .symbol = symbol,
              .side = side,
              .position_effect = {},
              .margin_mode = {},
              .max_show_quantity = NaN,
              .order_type = {},
              .time_in_force = {},
              .execution_instructions = {},
              .create_time_utc = {},
              .update_time_utc = update_time_utc,
              .external_account = {},
              .external_order_id = external_order_id,
              .client_order_id = client_order_id,
              .order_status = OrderStatus::COMPLETED,
              .quantity = NaN,
              .price = NaN,
              .stop_price = NaN,
              .remaining_quantity = remaining_quantity,
              .traded_quantity = NaN,
              .average_traded_price = NaN,
              .last_traded_quantity = NaN,
              .last_traded_price = NaN,
              .last_liquidity = {},
              .routing_id = {},
              .max_request_version = {},
              .max_response_version = {},
              .max_accepted_version = {},
              .update_type = UpdateType::INCREMENTAL,
              .sending_time_utc = {},
          };
          // log::warn("DEBUG order_update={}"sv, order_update);
          if (shared_.update_order(client_order_id, stream_id_, trace_info, order_update, []([[maybe_unused]] auto &order) {})) {
          } else {
            log::warn("*** EXTERNAL ORDER ***"sv);
          }
        }
      }
      auto trade_update = TradeUpdate{
          .stream_id = stream_id_,
          .account = account_.name,
          .order_id = {},
          .exchange = shared_.settings.exchange,
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .margin_mode = {},
          .quantity_type = {},
          .create_time_utc = update_time_utc,
          .update_time_utc = update_time_utc,
          .external_account = {},
          .external_order_id = external_order_id,
          .client_order_id = {},
          .fills = shared_.fills,
          .routing_id = {},
          .update_type = UpdateType::INCREMENTAL,
          .sending_time_utc = {},
          .user = {},
          .strategy_id = {},
      };
      create_trace_and_dispatch(handler_, trace_info, trade_update, true, SOURCE_NONE, client_order_id);
    };
    shared_.fills.clear();
    for (auto &item : fills.fills) {
      if (item.order_id != external_order_id) {
        if (!std::empty(shared_.fills)) {
          assert(!std::empty(symbol));
          assert(!std::empty(client_order_id));
          assert(side != Side{});
          dispatch();
        }
        symbol = item.instrument;
        external_order_id = item.order_id;
        client_order_id = item.cli_ord_id;
        side = item.buy ? Side::BUY : Side::SELL;  // note! assume the same
        update_time_utc = {};
        remaining_quantity = NaN;
        shared_.fills.clear();
        fill_symbols_.try_emplace(symbol);  // note!
      }
      update_time_utc = std::max(update_time_utc, item.time);
      remaining_quantity = [&]() {
        if (std::isnan(remaining_quantity)) {
          return item.remaining_order_qty;
        }
        if (std::isnan(item.remaining_order_qty)) {
          return remaining_quantity;
        }
        return std::min(remaining_quantity, item.remaining_order_qty);
      }();
      auto fill = Fill{
          .external_trade_id = item.fill_id,
          .quantity = item.qty,
          .price = item.price,
          .liquidity = map(item.fill_type),
          .base_amount = NaN,
          .quote_amount = NaN,
          .commission_amount = item.fee_paid,
          .commission_currency = item.fee_currency,
          .profit_loss_cost_amount = NaN,
      };
      shared_.fills.emplace_back(std::move(fill));
    }
    if (!std::empty(shared_.fills)) {
      dispatch();
    }
  });
}

void DropCopy::parse(std::string_view const &message) {
  profile_.parse([&]() {
    TraceInfo trace_info;
    auto result = json::ParserPrivate::dispatch(*this, message, decode_buffer_, trace_info);
    if (!result) {
      log::warn(R"(Unexpected: message="{}")"sv, message);
    }
  });
}

// helpers

void DropCopy::process_order(
    auto &order,
    [[maybe_unused]] std::string_view const &order_id,  // XXX FIXME TODO did we forget something ???
    std::string_view const &cli_ord_id,
    json::Reason reason,
    bool is_cancel,
    TraceInfo const &trace_info,
    bool is_download) {
  auto side = compute_side(order.direction);
  auto order_status = compute_order_status(reason, is_cancel, order.qty);
  auto quantity = order.qty + order.filled;
  auto update_type = is_download ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL;
  auto order_update = server::oms::OrderUpdate{
      .account = account_.name,
      .exchange = shared_.settings.exchange,
      .symbol = order.instrument,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order.type),
      .time_in_force = TimeInForce::GTC,  // note! assumption
      .execution_instructions = {},
      .create_time_utc = order.time,
      .update_time_utc = order.last_update_time,
      .external_account = {},
      .external_order_id = order.order_id,
      .client_order_id = cli_ord_id,
      .order_status = order_status,
      .quantity = quantity,  // note!
      .price = order.limit_price,
      .stop_price = order.stop_price,
      .remaining_quantity = order.qty,  // note!
      .traded_quantity = order.filled,
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
      .update_type = update_type,
      .sending_time_utc = {},
  };
  // log::warn("DEBUG order_update={}"sv, order_update);
  auto request_id = cli_ord_id;
  if (shared_.update_order(request_id, stream_id_, trace_info, order_update, []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order={}"sv, order);
  }
}

}  // namespace kraken_futures
}  // namespace roq

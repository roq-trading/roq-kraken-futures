/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/kraken_futures/gateway.h"

#include <limits>
#include <utility>

#include "roq/core/utils.h"

#include "roq/kraken_futures/options.h"

#include "roq/kraken_futures/json/utils.h"

namespace roq {
namespace kraken_futures {

template <typename C, typename T>
static bool mbp_update(C &data, size_t &offset, const T &item) {
  auto &obj = data[offset];
  new (&obj) MBPUpdate{
      .price = item.price,
      .quantity = item.volume,
  };
  ++offset;
  return offset < data.size();
}

template <typename C, typename T>
static bool trade_update(C &data, size_t &offset, const T &item) {
  auto &obj = data[offset];
  new (&obj) Trade{
      .side = json::map(item.side),
      .price = item.price,
      .quantity = item.volume,
      .trade_id = {},
  };
  ++offset;
  return offset < data.size();
}

Gateway::Gateway(server::Dispatcher &dispatcher, const Config &config)
    : _dispatcher(dispatcher), _account(config.get_account()),
      _access_key(config.get_access_key()), _random(config.get_access_secret()),
      _dns_base(_base, true),
      _web_socket{
          .connection =
              {
                  *this,
                  config,
                  _random,
                  _base,
                  _dns_base,
                  _ssl_context,
              },
          .download = WebSocketDownload(
              std::chrono::seconds{FLAGS_download_timeout_secs},
              [this](auto state) { return download(state); }),
      },
      _rest{
          .connection =
              {
                  *this,
                  config,
                  _random,
                  _base,
                  _dns_base,
                  _ssl_context,
              },
      },
      _bid(FLAGS_cache_mbp_max_depth), _ask(FLAGS_cache_mbp_max_depth),
      _trade(FLAGS_max_trades) {
  LOG_IF(WARNING, FLAGS_cancel_on_disconnect == false)
  ("Orders will *NOT* be cancelled on disconnect");
}

void Gateway::operator()(const Event<Start> &event) {
  LOG(INFO)("Starting the gateway...");
  _web_socket.connection(event);
  _rest.connection(event);
}

void Gateway::operator()(const Event<Stop> &event) {
  LOG(INFO)("Stopping the gateway...");
  _rest.connection(event);
  _web_socket.connection(event);
}

void Gateway::operator()(const Event<Timer> &event) {
  _web_socket.connection(event);
  _rest.connection(event);
  // download
  /*
  if (_web_socket.download.has_expired()) {
    LOG(WARNING)("WebSocket download has timed out");
    _web_socket.download.reset();
    _web_socket.connection.close();
  }
  */
  _base.loop(EVLOOP_NONBLOCK);
}

void Gateway::operator()(const Event<Connection> &) {
}

void Gateway::operator()(
    [[maybe_unused]] const Event<CreateOrder> &event,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] uint32_t gateway_order_id) {
  // TODO(thraneh): implement
}

void Gateway::operator()(
    [[maybe_unused]] const Event<ModifyOrder> &event,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const server::OMS_Order &order) {
  // TODO(thraneh): implement
}

void Gateway::operator()(
    [[maybe_unused]] const Event<CancelOrder> &event,
    [[maybe_unused]] const std::string_view &request_id,
    [[maybe_unused]] const server::OMS_Order &order) {
  // TODO(thraneh): implement
}

void Gateway::operator()(metrics::Writer &writer) {
  _rest.connection(writer);
  _web_socket.connection(writer);
}

// rest

void Gateway::operator()(const Rest &) {
  if (_rest.connection.ready()) _web_socket.download.bump();
}

void Gateway::download_asset_pairs() {
  constexpr auto state = WebSocketDownload::State::ASSET_PAIRS;
  _rest.connection.get_asset_pairs([this](auto &response) {
    try {
      auto status = response.status();
      switch (status) {
        case core::http::Status::OK: _web_socket.download.check(state); break;
        default:
          LOG(FATAL)
          (R"(Unable to get products, )"
           R"(status={})",
           status);
      }
    } catch (NotConnected &) {
      _web_socket.download.retry(state);
    } catch (TimedOut &) {
      _web_socket.download.retry(state);
    }
  });
}

void Gateway::operator()(const json::AssetPairs &asset_pairs) {
  assert(_symbols.empty());
  server::TraceInfo trace_info;  // XXX
  _symbols.reserve(asset_pairs.result.size());
  for (auto &item : asset_pairs.result) {
    if (item.wsname.empty()) {
      VLOG(1)(R"(Skipping altname={}, reason: wsname is empty)", item.altname);
      continue;
    }
    std::string symbol(item.wsname);
    // XXX remove escape
    symbol.erase(std::remove(symbol.begin(), symbol.end(), '\\'), symbol.end());
    if (_dispatcher.discard_symbol(symbol)) continue;
    _symbols.emplace_back(symbol);
    ReferenceData reference_data{
        .exchange = FLAGS_exchange,
        .symbol = symbol,
        .description = item.altname,
        .security_type = SecurityType::UNDEFINED,
        .currency = item.aclass_quote,                        // XXX check
        .settlement_currency = item.aclass_base,              // XXX check
        .commission_currency = item.aclass_base,              // XXX check
        .tick_size = std::pow(10.0, -item.pair_decimals),     // XXX check
        .multiplier = item.lot_multiplier,                    // XXX check
        .min_trade_vol = std::pow(10.0, -item.lot_decimals),  // XXX check
        .option_type = OptionType::UNDEFINED,
        .strike_currency = {},
        .strike_price = std::numeric_limits<double>::quiet_NaN(),
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
    };
    VLOG(1)(R"(reference_data={})", reference_data);
    server::create_trace_and_dispatch(
        trace_info, reference_data, _dispatcher, true);
    MarketStatus market_status{
        .exchange = FLAGS_exchange,
        .symbol = symbol,
        .trading_status = TradingStatus::OPEN,  // XXX doesn't exist?
    };
    VLOG(2)(R"(market_status={})", market_status);
    server::create_trace_and_dispatch(
        trace_info, market_status, _dispatcher, true);
  }
}

// web socket

int32_t Gateway::download(WebSocketDownload::State state) {
  if (_web_socket.connection.ready() == false) return -1;
  switch (state) {
    case WebSocketDownload::State::UNDEFINED: assert(false); break;
    case WebSocketDownload::State::ASSET_PAIRS:
      download_asset_pairs();
      return 1;
    case WebSocketDownload::State::SUBSCRIBE: subscribe(); return 0;
    case WebSocketDownload::State::DONE: update(GatewayStatus::READY); return 0;
  }
  assert(false);
  return 0;
}

void Gateway::operator()(const WebSocket &) {
  if (_web_socket.connection.ready()) {
    _web_socket.download.begin();
  } else {
    _web_socket.download.reset();
    _symbols.clear();
  }
}

void Gateway::subscribe() {
  roq::span pairs(_symbols.data(), _symbols.size());
  _web_socket.connection.subscribe("trade", pairs);
  _web_socket.connection.subscribe("spread", pairs);
  _web_socket.connection.subscribe("book", pairs);
}

void Gateway::operator()(
    const json::Trade &trade, const std::string_view &pair) {
  server::TraceInfo trace_info;  // XXX
  bool success = true;
  std::chrono::nanoseconds exchange_time_utc = {};
  size_t trade_length = 0;
  for (auto &item : trade.data) {
    if (success == false) break;
    success = trade_update(_trade, trade_length, item);
    if (exchange_time_utc.count() == 0) exchange_time_utc = item.time;
  }
  if (ROQ_UNLIKELY(success == false)) {
    LOG(FATAL)
    (R"(Insufficient trade array size: )"
     R"(len(trade)={}/{})",
     trade_length,
     _trade.size());
  }
  if (trade_length > 0) {
    TradeSummary trade_summary{
        .exchange = FLAGS_exchange,
        .symbol = pair,
        .trades =
            {
                .items = _trade.data(),
                .length = trade_length,
            },
        .exchange_time_utc = exchange_time_utc,
    };
    VLOG(3)(R"(trade_summary={})", trade_summary);
    server::create_trace_and_dispatch(
        trace_info, trade_summary, _dispatcher, true);
  }
}

void Gateway::operator()(
    const json::Spread &spread, const std::string_view &pair) {
  server::TraceInfo trace_info;  // XXX
  TopOfBook top_of_book{
      .exchange = FLAGS_exchange,
      .symbol = pair,
      .layer =
          {
              .bid_price = spread.bid,
              .bid_quantity = spread.bid_volume,
              .ask_price = spread.ask,
              .ask_quantity = spread.ask_volume,
          },
      .snapshot = false,  // note! we don't know... false is probably ok
      .exchange_time_utc = spread.timestamp,
  };
  VLOG(3)(R"(top_of_book={})", top_of_book);
  server::create_trace_and_dispatch(trace_info, top_of_book, _dispatcher, true);
}

void Gateway::operator()(const json::Book &book, const std::string_view &pair) {
  server::TraceInfo trace_info;  // XXX
  bool snapshot = book.bs.empty() == false && book.as.empty() == false;
  bool live = book.b.empty() == false && book.a.empty() == false;
  LOG_IF(FATAL, snapshot && live)("Unexpected");
  bool success = true;
  std::chrono::nanoseconds exchange_time_utc = {};
  size_t bid_length = 0, ask_length = 0;
  for (auto &item : book.b) {
    if (success == false) break;
    success = mbp_update(_bid, bid_length, item);
    if (exchange_time_utc.count() == 0) exchange_time_utc = item.timestamp;
  }
  for (auto &item : book.bs) {
    if (success == false) break;
    success = mbp_update(_bid, bid_length, item);
    if (exchange_time_utc.count() == 0) exchange_time_utc = item.timestamp;
  }
  for (auto &item : book.a) {
    if (success == false) break;
    success = mbp_update(_ask, ask_length, item);
    if (exchange_time_utc.count() == 0) exchange_time_utc = item.timestamp;
  }
  for (auto &item : book.as) {
    if (success == false) break;
    success = mbp_update(_ask, ask_length, item);
    if (exchange_time_utc.count() == 0) exchange_time_utc = item.timestamp;
  }
  if (ROQ_UNLIKELY(success == false)) {
    LOG(FATAL)
    (R"(Insufficient bid/ask array size(s): )"
     R"(len(bid={}/{}, len(ask)={}/{})",
     bid_length,
     _bid.size(),
     ask_length,
     _ask.size());
  }
  if (bid_length > 0 || ask_length > 0) {
    MarketByPriceUpdate market_by_price_update{
        .exchange = FLAGS_exchange,
        .symbol = pair,
        .bids =
            {
                .items = _bid.data(),
                .length = bid_length,
            },
        .asks =
            {
                .items = _ask.data(),
                .length = ask_length,
            },
        .snapshot = snapshot,
        .exchange_time_utc = exchange_time_utc,
    };
    VLOG(3)(R"(market_by_price_update={})", market_by_price_update);
    server::create_trace_and_dispatch(
        trace_info, market_by_price_update, _dispatcher, true);
  }
}

void Gateway::update(GatewayStatus gateway_status) {
  if (gateway_status == _gateway_status) return;
  _gateway_status = gateway_status;
  server::TraceInfo trace_info;
  MarketDataStatus market_data_status{
      .status = _gateway_status,
  };
  server::create_trace_and_dispatch(
      trace_info, market_data_status, _dispatcher, false);
  OrderManagerStatus order_manager_status{
      .account = _account,
      .status = _gateway_status,
  };
  server::create_trace_and_dispatch(
      trace_info, order_manager_status, _dispatcher, true);
  LOG(INFO)(R"(Update: gateway_status={})", _gateway_status);
}

}  // namespace kraken_futures
}  // namespace roq

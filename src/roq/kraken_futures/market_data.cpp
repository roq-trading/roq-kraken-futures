/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/market_data.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/web/socket/client_factory.hpp"

#include "roq/kraken_futures/flags.hpp"

#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === CONSTANTS ===

namespace {
auto const NAME = "md"sv;

auto const SUPPORTS = Mask{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
};
}  // namespace

// === HELPERS ===

namespace {
auto create_name(auto stream_id) {
  return fmt::format("{}:{}"sv, stream_id, NAME);
}

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_uri();
  web::socket::Client::Config config{
      .always_reconnect = true,
      .connection_timeout = server::Flags::net_connection_timeout(),
      .disconnect_on_idle_timeout = server::Flags::net_disconnect_on_idle_timeout(),
      .validate_certificate = server::Flags::net_tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
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

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index},
      connection_{create_connection(*this, context)}, decode_buffer_{Flags::decode_buffer_size()},
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(name_, "parse"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
          .ticker = create_metrics(name_, "ticker"sv),
          .book_snapshot = create_metrics(name_, "book_snapshot"sv),
          .book = create_metrics(name_, "book"sv),
          .trade_snapshot = create_metrics(name_, "trade_snapshot"sv),
          .trade = create_metrics(name_, "trade"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
          .heartbeat = create_metrics(name_, "heartbeat"sv),
      },
      shared_{shared} {
}

void MarketData::operator()(Event<Start> const &) {
  (*connection_).start();
}

void MarketData::operator()(Event<Stop> const &) {
  (*connection_).stop();
}

void MarketData::operator()(Event<Timer> const &event) {
  (*connection_).refresh(event.value.now);
}

void MarketData::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.parse, metrics::PROFILE)
      .write(profile_.heartbeat, metrics::PROFILE)
      .write(profile_.ticker, metrics::PROFILE)
      .write(profile_.book_snapshot, metrics::PROFILE)
      .write(profile_.book, metrics::PROFILE)
      .write(profile_.trade_snapshot, metrics::PROFILE)
      .write(profile_.trade, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY)
      .write(latency_.heartbeat, metrics::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready())
    subscribe(shared_.symbols.get_slice(index_, start_from));
}

void MarketData::operator()(web::socket::Client::Connected const &) {
  // note! wait for upgrade
}

void MarketData::operator()(web::socket::Client::Disconnected const &) {
  ++counter_.disconnect;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(web::socket::Client::Ready const &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(web::socket::Client::Close const &) {
}

void MarketData::operator()(web::socket::Client::Latency const &latency) {
  TraceInfo trace_info;
  const ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(web::socket::Client::Text const &text) {
  parse(text.payload);
}

void MarketData::operator()(web::socket::Client::Binary const &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    TraceInfo trace_info;
    const StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
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

void MarketData::subscribe(std::span<Symbol const> const &symbols) {
  subscribe("ticker"sv, symbols);
  subscribe("book"sv, symbols);
  subscribe("trade"sv, symbols);
}

void MarketData::subscribe(std::string_view const &feed) {
  log::info(R"(subscribe feed="{}")"sv, feed);
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}")"
      R"(}})"sv,
      feed);
  log::info<2>(R"(request="{}")"sv, message);
  (*connection_).send_text(message);
}

template <typename T>
void MarketData::subscribe(std::string_view const &feed, std::span<T> const &product_ids) {
  log::info(R"(subscribe feed="{}", len(product_ids)={})"sv, feed, std::size(product_ids));
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}",)"
      R"("product_ids":["{}"])"
      R"(}})"sv,
      feed,
      fmt::join(product_ids, R"(",")"sv));
  log::info<2>(R"(request="{}")"sv, message);
  (*connection_).send_text(message);
}

template <typename T>
void MarketData::unsubscribe(std::string_view const &feed, std::span<T> const &product_ids) {
  log::info(R"(subscribe feed="{}", len(product_ids)={})"sv, feed, std::size(product_ids));
  auto message = fmt::format(
      R"({{)"
      R"("event":"unsubscribe",)"
      R"("feed":"{}",)"
      R"("product_ids":["{}"])"
      R"(}})"sv,
      feed,
      fmt::join(product_ids, R"(",")"sv));
  log::info<2>(R"(request="{}")"sv, message);
  (*connection_).send_text(message);
}

void MarketData::parse(std::string_view const &message) {
  profile_.parse([&]() {
    TraceInfo trace_info;
    core::json::Buffer buffer{decode_buffer_};
    auto result = json::ParserPublic::dispatch(*this, message, buffer, trace_info);
    if (!result) [[unlikely]]
      log::warn(R"(Unexpected: message="{}")"sv, message);
  });
}

void MarketData::operator()(Trace<json::Info> const &event) {
  auto &[trace_info, info] = event;
  log::info<2>("info={}"sv, info);
}

void MarketData::operator()(Trace<json::Alert> const &event) {
  auto &[trace_info, alert] = event;
  log::warn<1>("alert={}"sv, alert);
}

void MarketData::operator()(Trace<json::Error> const &event) {
  auto &[trace_info, error] = event;
  log::warn("error={}"sv, error);
}

void MarketData::operator()(Trace<json::Subscribed> const &event) {
  auto &[trace_info, subscribed] = event;
  log::info<2>("subscribed={}"sv, subscribed);
}

void MarketData::operator()(Trace<json::Heartbeat> const &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
    log::info<2>("heartbeat={}"sv, heartbeat);
  });
}

void MarketData::operator()(Trace<json::Ticker> const &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    log::info<4>("ticker={}"sv, ticker);
    (*connection_).touch(trace_info.source_receive_time);
    const TopOfBook top_of_book{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = ticker.product_id,
        .layer{
            .bid_price = ticker.bid,
            .bid_quantity = ticker.bid_size,
            .ask_price = ticker.ask,
            .ask_quantity = ticker.ask_size,
        },
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(ticker.time),
        .exchange_sequence = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
    // note! using *relative* funding rate to be compatible with other exchanges
    Statistics statistics[] = {
        {
            .type = StatisticsType::INDEX_VALUE,
            .value = ticker.index,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::SETTLEMENT_PRICE,
            .value = ticker.mark_price,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = ticker.relative_funding_rate,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::FUNDING_RATE_PREDICTION,
            .value = ticker.relative_funding_rate_prediction,
            .begin_time_utc = utils::safe_cast(ticker.next_funding_rate_time),
            .end_time_utc = {},
        },
    };
    const StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = ticker.product_id,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(ticker.time),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    auto trading_status = ticker.suspended ? TradingStatus::HALT : TradingStatus::OPEN;
    const MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = ticker.product_id,
        .trading_status = trading_status,
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  });
}

void MarketData::operator()(Trace<json::BookSnapshot> const &event) {
  profile_.book_snapshot([&]() {
    auto &[trace_info, book_snapshot] = event;
    log::info<4>("book_snapshot={}"sv, book_snapshot);
    (*connection_).touch(trace_info.source_receive_time);
    auto &symbol = book_snapshot.product_id;
    latch_.erase(symbol);  // unlatch
    auto create_mbp_update = []<typename T>(T &result, auto const &value) {
      new (&result) T{
          .price = value.price,
          .quantity = value.qty,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
    };
    core::back_emplacer bids{shared_.bids}, asks{shared_.asks};
    for (const auto &bid : book_snapshot.bids)
      bids.emplace_back([&](auto &result) { create_mbp_update(result, bid); });
    for (const auto &ask : book_snapshot.asks)
      asks.emplace_back([&](auto &result) { create_mbp_update(result, ask); });
    const MarketByPriceUpdate market_by_price_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .bids = bids,
        .asks = asks,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = book_snapshot.timestamp,
        .exchange_sequence = book_snapshot.seq,
        .price_decimals = {},
        .quantity_decimals = {},
        .checksum = {},
    };
    log::info<3>("market_by_price_update={}"sv, market_by_price_update);
    create_trace_and_dispatch(handler_, trace_info, market_by_price_update, false, false);
  });
}

void MarketData::operator()(Trace<json::Book> const &event) {
  profile_.book([&]() {
    auto &[trace_info, book] = event;
    log::info<4>("book={}"sv, book);
    (*connection_).touch(trace_info.source_receive_time);
    auto &symbol = book.product_id;
    if (latch_.find(symbol) != std::end(latch_))
      return;  //  waiting for snapshot
    MBPUpdate mbp_update{
        .price = book.price,
        .quantity = book.qty,
        .implied_quantity = NaN,
        .number_of_orders = {},
        .update_action = {},
        .price_level = {},
    };
    auto bid = book.side == json::Side::BUY;
    auto ask = book.side == json::Side::SELL;
    assert((bid || ask) && !(bid && ask));
    const MarketByPriceUpdate market_by_price_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .bids = {bid ? &mbp_update : nullptr, bid ? 1u : 0u},
        .asks = {ask ? &mbp_update : nullptr, ask ? 1u : 0u},
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = book.timestamp,
        .exchange_sequence = book.seq,
        .price_decimals = {},
        .quantity_decimals = {},
        .checksum = {},
    };
    try {
      log::info<3>("market_by_price_update={}"sv, market_by_price_update);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, false, false);
    } catch (BadState &e) {
      resubscribe(trace_info, symbol);
    }
  });
}

void MarketData::operator()(Trace<json::TradeSnapshot> const &event) {
  profile_.trade_snapshot([&]() {
    auto &[trace_info, trade_snapshot] = event;
    log::info<4>("trade_snapshot={}"sv, trade_snapshot);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void MarketData::operator()(Trace<json::Trade> const &event) {
  profile_.trade([&]() {
    // auto &[trace_info, trade] = event;
    auto &trace_info = event.trace_info;
    auto &trade = event.value;
    log::info<4>("trade={}"sv, trade);
    (*connection_).touch(trace_info.source_receive_time);
    auto create_trade = []<typename T>(T &result, auto const &value) {
      new (&result) T{
          .side = json::map(value.side),
          .price = value.price,
          .quantity = value.qty,
          .trade_id = value.uid,
          .taker_order_id = {},
          .maker_order_id = {},
      };
    };
    core::back_emplacer trades{shared_.trades};
    trades.emplace_back([&](auto &result) { create_trade(result, trade); });
    const TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = trade.product_id,
        .trades = trades,
        .exchange_time_utc = trade.time,
        .exchange_sequence = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::resubscribe(TraceInfo const &trace_info, std::string_view const &symbol) {
  log::warn<1>(R"(*** RESUBSCRIBE *** (symbol="{}"))"sv, symbol);
  const MarketByPriceUpdate market_by_price_update{
      .stream_id = stream_id_,
      .exchange = Flags::exchange(),
      .symbol = symbol,
      .bids = {},
      .asks = {},
      .update_type = UpdateType::STALE,
      .exchange_time_utc = {},
      .exchange_sequence = {},
      .price_decimals = {},
      .quantity_decimals = {},
      .checksum = {},
  };
  log::info<3>("market_by_price_update={}"sv, market_by_price_update);
  create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true, false);
  latch_.emplace(symbol);  // latch
  unsubscribe("book"sv, symbol);
  subscribe("book"sv, symbol);
}

}  // namespace kraken_futures
}  // namespace roq

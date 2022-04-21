/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/market_data.hpp"

#include <algorithm>

#include "roq/mask.hpp"
#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/core/back_emplacer.hpp"

#include "roq/core/metrics/factory.hpp"

#include "roq/kraken_futures/flags.hpp"

#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

namespace {
const auto NAME = "md"sv;
const Mask SUPPORTS{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::ws_uri();
  core::web::ClientSocket::Config config{
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri, 1},
      .query = {},
      .ping_frequency = Flags::ws_ping_freq(),
      .read_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
  };
  return core::web::ClientSocket{handler, context, config, []() { return std::string(); }};
}

template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.qty,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}

template <typename T>
void emplace(Trade &result, const T &value) {
  new (&result) Trade{
      .side = json::map(value.side),
      .price = value.price,
      .quantity = value.qty,
      .trade_id = value.uid,
  };
}
}  // namespace

MarketData::MarketData(
    Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      index_(index), connection_(create_connection(*this, context)),
      decode_buffer_(Flags::decode_buffer_size()),
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
      shared_(shared) {
}

void MarketData::operator()(const Event<Start> &) {
  connection_.start();
}

void MarketData::operator()(const Event<Stop> &) {
  connection_.stop();
}

void MarketData::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
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

void MarketData::operator()(const core::web::ClientSocket::Connected &) {
  // note! wait for upgrade
}

void MarketData::operator()(const core::web::ClientSocket::Disconnected &) {
  ++counter_.disconnect;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
}

void MarketData::operator()(const core::web::ClientSocket::Ready &) {
  (*this)(ConnectionStatus::READY);
  subscribe();
}

void MarketData::operator()(const core::web::ClientSocket::Close &) {
}

void MarketData::operator()(const core::web::ClientSocket::Latency &latency) {
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::ClientSocket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(const core::web::ClientSocket::Binary &) {
  log::fatal("Unexpected"sv);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
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

void MarketData::subscribe(const std::span<Symbol const> &symbols) {
  subscribe("ticker"sv, symbols);
  subscribe("book"sv, symbols);
  subscribe("trade"sv, symbols);
}

void MarketData::subscribe(const std::string_view &feed) {
  log::info(R"(subscribe feed="{}")"sv, feed);
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}")"
      R"(}})"sv,
      feed);
  log::info<2>(R"(request="{}")"sv, message);
  connection_.send_text(message);
}

template <typename T>
void MarketData::subscribe(const std::string_view &feed, const std::span<T> &product_ids) {
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
  connection_.send_text(message);
}

template <typename T>
void MarketData::unsubscribe(const std::string_view &feed, const std::span<T> &product_ids) {
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
  connection_.send_text(message);
}

void MarketData::parse(const std::string_view &message) {
  profile_.parse([&]() {
    auto trace_info = server::create_trace_info();
    core::json::Buffer buffer(decode_buffer_);
    auto result = json::ParserPublic::dispatch(*this, message, buffer, trace_info);
    if (!result) [[unlikely]]
      log::warn(R"(Unexpected: message="{}")"sv, message);
  });
}

void MarketData::operator()(const Trace<json::Info> &event) {
  auto &[trace_info, info] = event;
  log::info<2>("info={}"sv, info);
}

void MarketData::operator()(const Trace<json::Alert> &event) {
  auto &[trace_info, alert] = event;
  log::warn<1>("alert={}"sv, alert);
}

void MarketData::operator()(const Trace<json::Error> &event) {
  auto &[trace_info, error] = event;
  log::warn("error={}"sv, error);
}

void MarketData::operator()(const Trace<json::Subscribed> &event) {
  auto &[trace_info, subscribed] = event;
  log::info<2>("subscribed={}"sv, subscribed);
}

void MarketData::operator()(const Trace<json::Heartbeat> &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
    log::info<2>("heartbeat={}"sv, heartbeat);
  });
}

void MarketData::operator()(const Trace<json::Ticker> &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    log::info<4>("ticker={}"sv, ticker);
    TopOfBook top_of_book{
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
    StatisticsUpdate statistics_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = ticker.product_id,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = utils::safe_cast(ticker.time),
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    auto trading_status = ticker.suspended ? TradingStatus::HALT : TradingStatus::OPEN;
    MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = ticker.product_id,
        .trading_status = trading_status,
    };
    create_trace_and_dispatch(handler_, trace_info, market_status, true);
  });
}

void MarketData::operator()(const Trace<json::BookSnapshot> &event) {
  profile_.book_snapshot([&]() {
    auto &[trace_info, book_snapshot] = event;
    log::info<4>("book_snapshot={}"sv, book_snapshot);
    auto &symbol = book_snapshot.product_id;
    latch_.erase(symbol);  // unlatch
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (const auto &bid : book_snapshot.bids)
      bids.emplace_back([&](auto &result) { emplace(result, bid); });
    for (const auto &ask : book_snapshot.asks)
      asks.emplace_back([&](auto &result) { emplace(result, ask); });
    MarketByPriceUpdate market_by_price_update{
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

void MarketData::operator()(const Trace<json::Book> &event) {
  profile_.book([&]() {
    auto &[trace_info, book] = event;
    log::info<4>("book={}"sv, book);
    auto &symbol = book.product_id;
    if (latch_.find(symbol) != std::end(latch_))
      return;  //  waiting for snapshot
    MBPUpdate mbp_update{
        .price = book.price,
        .quantity = book.qty,
        .implied_quantity = NaN,
        .price_level = {},
        .number_of_orders = {},
    };
    auto bid = book.side == json::Side::BUY;
    auto ask = book.side == json::Side::SELL;
    assert((bid || ask) && !(bid && ask));
    MarketByPriceUpdate market_by_price_update{
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

void MarketData::operator()(const Trace<json::TradeSnapshot> &event) {
  profile_.trade_snapshot([&]() {
    auto &[trace_info, trade_snapshot] = event;
    log::info<4>("trade_snapshot={}"sv, trade_snapshot);
  });
}

void MarketData::operator()(const Trace<json::Trade> &event) {
  profile_.trade([&]() {
    // auto &[trace_info, trade] = event;
    auto &trace_info = event.trace_info;
    auto &trade = event.value;
    log::info<4>("trade={}"sv, trade);
    core::back_emplacer trades(shared_.trades);
    trades.emplace_back([&](auto &result) { emplace(result, trade); });
    TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = trade.product_id,
        .trades = trades,
        .exchange_time_utc = trade.time,
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::resubscribe(const TraceInfo &trace_info, const std::string_view &symbol) {
  log::warn<1>(R"(*** RESUBSCRIBE *** (symbol="{}"))"sv, symbol);
  MarketByPriceUpdate market_by_price_update{
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

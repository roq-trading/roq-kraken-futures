/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/market_data.h"

#include <algorithm>

#include "roq/utils/mask.h"
#include "roq/utils/safe_cast.h"
#include "roq/utils/update.h"

#include "roq/core/back_emplacer.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"

#include "roq/kraken_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

namespace {
static const auto NAME = "md"_sv;
static const auto SUPPORTS = utils::Mask{
    SupportType::TOP_OF_BOOK,
    SupportType::MARKET_BY_PRICE,
    SupportType::TRADE_SUMMARY,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

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
    Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
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
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
          .ticker = create_metrics(name_, "ticker"_sv),
          .book_snapshot = create_metrics(name_, "book_snapshot"_sv),
          .book = create_metrics(name_, "book"_sv),
          .trade_snapshot = create_metrics(name_, "trade_snapshot"_sv),
          .trade = create_metrics(name_, "trade"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
          .heartbeat = create_metrics(name_, "heartbeat"_sv),
      },
      shared_(shared),
      download_(Flags::ws_request_timeout(), [this](auto state) { return download(state); }) {
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

void MarketData::update_subscriptions(std::vector<std::string> &symbols) {
  assert(&symbols != &symbols_);
  auto max_size = Flags::ws_max_subscriptions_per_stream();
  auto offset = symbols_.size();
  if (max_size <= offset)
    return;
  if (symbols.empty())
    return;
  symbols_.reserve(max_size);
  auto length = std::min(max_size - offset, symbols.size());
  assert(length > 0);
  for (size_t i = {}; i < length; ++i) {
    symbols_.emplace_back(symbols.back());
    symbols.pop_back();
  }
  assert(length == (symbols_.size() - offset));
  if (ready_)
    subscribe({&symbols_[offset], length});
}

void MarketData::operator()(const core::web::Socket::Connected &) {
  // note! wait for upgrade
}

void MarketData::operator()(const core::web::Socket::Disconnected &) {
  ++counter_.disconnect;
  ready_ = false;
  next_heartbeat_ = {};
  (*this)(ConnectionStatus::DISCONNECTED);
  download_.reset();
}

void MarketData::operator()(const core::web::Socket::Ready &) {
  (*this)(ConnectionStatus::DOWNLOADING);
  download_.begin();
}

void MarketData::operator()(const core::web::Socket::Close &) {
}

void MarketData::operator()(const core::web::Socket::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

void MarketData::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void MarketData::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::WEB_SOCKET,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

uint32_t MarketData::download(MarketDataState state) {
  switch (state) {
    case MarketDataState::UNDEFINED:
      assert(false);
      break;
    case MarketDataState::SUBSCRIBE:
      subscribe("heartbeat"_sv);
      subscribe(symbols_);
      return {};
    case MarketDataState::DONE:
      (*this)(ConnectionStatus::READY);
      assert(!ready_);
      ready_ = true;
      return {};
  }
  assert(false);
  return {};
}

void MarketData::subscribe(const roq::span<std::string> &symbols) {
  subscribe("ticker"_sv, symbols);
  subscribe("book"_sv, symbols);
  subscribe("trade"_sv, symbols);
}

void MarketData::subscribe(const std::string_view &feed) {
  log::info(R"(subscribe feed="{}")"_sv, feed);
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}")"
      R"(}})"_sv,
      feed);
  log::info<3>(R"(request="{}")"_sv, message);
  connection_.send_text(message);
}

void MarketData::subscribe(
    const std::string_view &feed, const roq::span<std::string> &product_ids) {
  log::info(R"(subscribe feed="{}", len(product_ids)={})"_sv, feed, std::size(product_ids));
  auto message = fmt::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}",)"
      R"("product_ids":["{}"])"
      R"(}})"_sv,
      feed,
      fmt::join(product_ids, R"(",")"_sv));
  log::info<3>(R"(request="{}")"_sv, message);
  connection_.send_text(message);
}

void MarketData::parse(const std::string_view &message) {
  profile_.parse([&]() {
    server::TraceInfo trace_info;
    core::json::Buffer buffer(decode_buffer_);
    auto result = json::ParserPublic::dispatch(*this, message, buffer, trace_info);
    if (ROQ_UNLIKELY(!result))
      log::warn(R"(Unexpected: message="{}")"_sv, message);
  });
}

void MarketData::operator()(const server::Trace<json::Info> &event) {
  auto &[trace_info, info] = event;
  log::info<1>("info={}"_sv, info);
}

void MarketData::operator()(const server::Trace<json::Alert> &event) {
  auto &[trace_info, alert] = event;
  log::warn<1>("alert={}"_sv, alert);
}

void MarketData::operator()(const server::Trace<json::Error> &event) {
  auto &[trace_info, error] = event;
  log::warn("error={}"_sv, error);
}

void MarketData::operator()(const server::Trace<json::Subscribed> &event) {
  auto &[trace_info, subscribed] = event;
  log::info<1>("subscribed={}"_sv, subscribed);
}

void MarketData::operator()(const server::Trace<json::Heartbeat> &event) {
  profile_.heartbeat([&]() {
    auto &[trace_info, heartbeat] = event;
    log::info<3>("heartbeat={}"_sv, heartbeat);
  });
}

void MarketData::operator()(const server::Trace<json::Ticker> &event) {
  profile_.ticker([&]() {
    auto &[trace_info, ticker] = event;
    log::info<3>("ticker={}"_sv, ticker);
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
        .snapshot = false,
        .exchange_time_utc = utils::safe_cast(ticker.time),
    };
    server::create_trace_and_dispatch(trace_info, top_of_book, handler_, true);
    // note! using *relative* funding rate to be compatible with other exchanges
    Statistics statistics[] = {
        {
            .type = StatisticsType::INDEX_VALUE,
            .value = ticker.index,
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
        .snapshot = false,
        .exchange_time_utc = utils::safe_cast(ticker.time),
    };
    server::create_trace_and_dispatch(trace_info, statistics_update, handler_, true);
    MarketStatus market_status{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = ticker.product_id,
        .trading_status = ticker.suspended ? TradingStatus::HALT : TradingStatus::OPEN,
    };
    server::create_trace_and_dispatch(trace_info, market_status, handler_, true);
  });
}

void MarketData::operator()(const server::Trace<json::BookSnapshot> &event) {
  profile_.book_snapshot([&]() {
    auto &[trace_info, book_snapshot] = event;
    log::info<3>("book_snapshot={}"_sv, book_snapshot);
    core::back_emplacer bids(shared_.bids), asks(shared_.asks);
    for (const auto &bid : book_snapshot.bids)
      bids.emplace_back([&](auto &result) { emplace(result, bid); });
    for (const auto &ask : book_snapshot.asks)
      asks.emplace_back([&](auto &result) { emplace(result, ask); });
    MarketByPriceUpdate market_by_price_update{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = book_snapshot.product_id,
        .bids = bids,
        .asks = asks,
        .snapshot = true,
        .exchange_time_utc = book_snapshot.timestamp,
    };
    log::info<3>("market_by_price_update={}"_sv, market_by_price_update);
    server::create_trace_and_dispatch(trace_info, market_by_price_update, handler_, false);
  });
}

void MarketData::operator()(const server::Trace<json::Book> &event) {
  profile_.book([&]() {
    auto &[trace_info, book] = event;
    log::info<3>("book={}"_sv, book);
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
        .symbol = book.product_id,
        .bids = {bid ? &mbp_update : nullptr, bid ? 1u : 0u},
        .asks = {ask ? &mbp_update : nullptr, ask ? 1u : 0u},
        .snapshot = false,
        .exchange_time_utc = book.timestamp,
    };
    log::info<3>("market_by_price_update={}"_sv, market_by_price_update);
    server::create_trace_and_dispatch(trace_info, market_by_price_update, handler_, false);
  });
}

void MarketData::operator()(const server::Trace<json::TradeSnapshot> &event) {
  profile_.trade_snapshot([&]() {
    auto &[trace_info, trade_snapshot] = event;
    log::info<3>("trade_snapshot={}"_sv, trade_snapshot);
  });
}

void MarketData::operator()(const server::Trace<json::Trade> &event) {
  profile_.trade([&]() {
    // auto &[trace_info, trade] = event;
    auto &trace_info = event.trace_info;
    auto &trade = event.value;
    log::info<3>("trade={}"_sv, trade);
    core::back_emplacer trades(shared_.trades);
    trades.emplace_back([&](auto &result) { emplace(result, trade); });
    TradeSummary trade_summary{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = trade.product_id,
        .trades = trades,
        .exchange_time_utc = trade.time,
    };
    server::create_trace_and_dispatch(trace_info, trade_summary, handler_, true);
  });
}

}  // namespace kraken_futures
}  // namespace roq

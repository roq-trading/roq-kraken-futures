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
/*
template <typename T>
void emplace(MBPUpdate &result, const T &value) {
  new (&result) MBPUpdate{
      .price = value.price,
      .quantity = value.volume,
      .implied_quantity = NaN,
      .price_level = {},
      .number_of_orders = {},
  };
}
*/
template <typename T>
void emplace(Trade &result, const T &value) {
  new (&result) Trade{
      .side = json::map(value.side),
      .price = value.price,
      .quantity = value.qty,
      .trade_id = {},
  };
}
}  // namespace

MarketData::MarketData(
    Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(roq::format("{}:{}"_sv, stream_id_, NAME)),
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
          .ticker = create_metrics(name_, "ticker"_sv),
          .trades = create_metrics(name_, "trades"_sv),
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
      .write(profile_.ticker, metrics::PROFILE)
      .write(profile_.trades, metrics::PROFILE)
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
  subscribe("trade"_sv, symbols);
}

void MarketData::subscribe(
    const std::string_view &feed, const roq::span<std::string> &product_ids) {
  log::info(R"(subscribe feed="{}", len(product_ids)={})"_sv, feed, std::size(product_ids));
  auto message = roq::format(
      R"({{)"
      R"("event":"subscribe",)"
      R"("feed":"{}",)"
      R"("product_ids":["{}"])"
      R"(}})"_sv,
      feed,
      roq::join(product_ids, R"(",")"_sv));
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

void MarketData::operator()(const json::Info &info, const server::TraceInfo &trace_info) {
  log::info("DEBUG: info={}"_sv, info);
}

void MarketData::operator()(const json::Alert &alert, const server::TraceInfo &trace_info) {
  log::info("DEBUG: alert={}"_sv, alert);
}

void MarketData::operator()(const json::Error &error, const server::TraceInfo &trace_info) {
  log::warn("error={}"_sv, error);
}

void MarketData::operator()(
    const json::Subscribed &subscribed, const server::TraceInfo &trace_info) {
  log::info("DEBUG: subscribed={}"_sv, subscribed);
}

void MarketData::operator()(const json::Ticker &ticker, const server::TraceInfo &trace_info) {
  profile_.ticker([&]() {
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
    Statistics statistics[] = {
        {
            .type = StatisticsType::INDEX_VALUE,
            .value = ticker.index,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = ticker.funding_rate_prediction,
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

void MarketData::operator()(const json::Trades &trades, const server::TraceInfo &trace_info) {
  profile_.trades([&]() { log::info<3>("trades={}"_sv, trades); });
}

void MarketData::operator()(const json::Trade &trade, const server::TraceInfo &trace_info) {
  profile_.trade([&]() {
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

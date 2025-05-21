/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/market_data.hpp"

#include <algorithm>
#include <utility>

#include "roq/mask.hpp"

#include "roq/utils/safe_cast.hpp"
#include "roq/utils/update.hpp"

#include "roq/utils/metrics/factory.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/kraken_futures/json/map.hpp"
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
      .disconnect_on_idle_timeout = settings.net.disconnect_on_idle_timeout,
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
}  // namespace

// === IMPLEMENTATION ===

MarketData::MarketData(Handler &handler, io::Context &context, uint16_t stream_id, Shared &shared, size_t index)
    : handler_{handler}, stream_id_{stream_id}, name_{create_name(stream_id_)}, index_{index}, connection_{create_connection(*this, shared.settings, context)},
      decode_buffer_(shared.settings.misc.decode_buffer_size),
      counter_{
          .disconnect = create_metrics(shared.settings, name_, "disconnect"sv),
      },
      profile_{
          .parse = create_metrics(shared.settings, name_, "parse"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
          .ticker = create_metrics(shared.settings, name_, "ticker"sv),
          .book_snapshot = create_metrics(shared.settings, name_, "book_snapshot"sv),
          .book = create_metrics(shared.settings, name_, "book"sv),
          .trade_snapshot = create_metrics(shared.settings, name_, "trade_snapshot"sv),
          .trade = create_metrics(shared.settings, name_, "trade"sv),
      },
      latency_{
          .ping = create_metrics(shared.settings, name_, "ping"sv),
          .heartbeat = create_metrics(shared.settings, name_, "heartbeat"sv),
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

void MarketData::operator()(metrics::Writer &writer) const {
  writer
      // counter
      .write(counter_.disconnect, metrics::Type::COUNTER)
      // profile
      .write(profile_.parse, metrics::Type::PROFILE)
      .write(profile_.heartbeat, metrics::Type::PROFILE)
      .write(profile_.ticker, metrics::Type::PROFILE)
      .write(profile_.book_snapshot, metrics::Type::PROFILE)
      .write(profile_.book, metrics::Type::PROFILE)
      .write(profile_.trade_snapshot, metrics::Type::PROFILE)
      .write(profile_.trade, metrics::Type::PROFILE)
      // latency
      .write(latency_.ping, metrics::Type::LATENCY)
      .write(latency_.heartbeat, metrics::Type::LATENCY);
}

void MarketData::subscribe(size_t start_from) {
  if (ready()) {
    subscribe(shared_.symbols.get_slice(index_, start_from));
  }
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
  auto external_latency = ExternalLatency{
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
    auto stream_status = StreamStatus{
        .stream_id = stream_id_,
        .account = {},
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
    auto result = json::ParserPublic::dispatch(*this, message, decode_buffer_, trace_info);
    if (!result) [[unlikely]] {
      log::warn(R"(Unexpected: message="{}")"sv, message);
    }
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
    auto top_of_book = TopOfBook{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = ticker.product_id,
        .layer{
            .bid_price = ticker.bid,
            .bid_quantity = ticker.bid_size,
            .ask_price = ticker.ask,
            .ask_quantity = ticker.ask_size,
        },
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = ticker.time,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, top_of_book, true);
    std::array<Statistics, 8> statistics{{
        {
            .type = StatisticsType::OPEN_PRICE,
            .value = ticker.open,
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
            .type = StatisticsType::OPEN_INTEREST,
            .value = ticker.open_interest,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::HIGHEST_TRADED_PRICE,
            .value = ticker.high,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::LOWEST_TRADED_PRICE,
            .value = ticker.low,
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::FUNDING_RATE,
            .value = ticker.relative_funding_rate,  // note! using *relative* funding rate to be compatible with other exchanges
            .begin_time_utc = {},
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::FUNDING_RATE_PREDICTION,
            .value = ticker.relative_funding_rate_prediction,
            .begin_time_utc = utils::safe_cast(ticker.next_funding_rate_time),
            .end_time_utc = {},
        },
        {
            .type = StatisticsType::TRADE_VOLUME,
            .value = ticker.volume,  // XXX FIXME TODO or ... volume_quote ???
            .begin_time_utc = {},
            .end_time_utc = {},
        },
    }};
    auto statistics_update = StatisticsUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = ticker.product_id,
        .statistics = statistics,
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = ticker.time,
        .exchange_sequence = {},
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, statistics_update, true);
    auto trading_status = ticker.suspended ? TradingStatus::CLOSE : TradingStatus::OPEN;
    auto market_status = MarketStatus{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = ticker.product_id,
        .trading_status = trading_status,
        .exchange_time_utc = {},
        .exchange_sequence = {},
        .sending_time_utc = {},
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
    shared_.bids.clear();
    shared_.asks.clear();
    auto emplace_back = [](auto &result, auto &value) {
      auto mbp_update = MBPUpdate{
          .price = value.price,
          .quantity = value.qty,
          .implied_quantity = NaN,
          .number_of_orders = {},
          .update_action = {},
          .price_level = {},
      };
      result.emplace_back(std::move(mbp_update));
    };
    for (auto &item : book_snapshot.bids) {
      emplace_back(shared_.bids, item);
    }
    for (auto &item : book_snapshot.asks) {
      emplace_back(shared_.asks, item);
    }
    auto market_by_price_update = MarketByPriceUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .bids = shared_.bids,
        .asks = shared_.asks,
        .update_type = UpdateType::SNAPSHOT,
        .exchange_time_utc = book_snapshot.timestamp,
        .exchange_sequence = book_snapshot.seq,
        .sending_time_utc = {},
        .price_precision = {},
        .quantity_precision = {},
        .checksum = {},
    };
    log::info<3>("market_by_price_update={}"sv, market_by_price_update);
    create_trace_and_dispatch(handler_, trace_info, market_by_price_update, false);
  });
}

void MarketData::operator()(Trace<json::Book> const &event) {
  profile_.book([&]() {
    auto &[trace_info, book] = event;
    log::info<4>("book={}"sv, book);
    (*connection_).touch(trace_info.source_receive_time);
    auto &symbol = book.product_id;
    if (latch_.find(symbol) != std::end(latch_)) {
      return;  //  waiting for snapshot
    }
    auto mbp_update = MBPUpdate{
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
    auto market_by_price_update = MarketByPriceUpdate{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = symbol,
        .bids = {bid ? &mbp_update : nullptr, bid ? 1U : 0U},
        .asks = {ask ? &mbp_update : nullptr, ask ? 1U : 0U},
        .update_type = UpdateType::INCREMENTAL,
        .exchange_time_utc = book.timestamp,
        .exchange_sequence = book.seq,
        .sending_time_utc = {},
        .price_precision = {},
        .quantity_precision = {},
        .checksum = {},
    };
    try {
      log::info<3>("market_by_price_update={}"sv, market_by_price_update);
      create_trace_and_dispatch(handler_, trace_info, market_by_price_update, false);
    } catch (BadState &e) {
      resubscribe(trace_info, symbol);
    }
  });
}

// note! we don't collect the snapshot
void MarketData::operator()(Trace<json::TradeSnapshot> const &event) {
  profile_.trade_snapshot([&]() {
    auto &[trace_info, trade_snapshot] = event;
    log::info<4>("trade_snapshot={}"sv, trade_snapshot);
    (*connection_).touch(trace_info.source_receive_time);
  });
}

void MarketData::operator()(Trace<json::Trade> const &event) {
  profile_.trade([&]() {
    auto &[trace_info, trade] = event;
    log::info<4>("trade={}"sv, trade);
    (*connection_).touch(trace_info.source_receive_time);
    auto trade_2 = Trade{
        .side = map(trade.side),
        .price = trade.price,
        .quantity = trade.qty,
        .trade_id = trade.uid,
        .taker_order_id = {},
        .maker_order_id = {},
    };
    auto trade_summary = TradeSummary{
        .stream_id = stream_id_,
        .exchange = shared_.settings.exchange,
        .symbol = trade.product_id,
        .trades = {&trade_2, 1U},
        .exchange_time_utc = trade.time,
        .exchange_sequence = trade.seq,
        .sending_time_utc = {},
    };
    create_trace_and_dispatch(handler_, trace_info, trade_summary, true);
  });
}

void MarketData::resubscribe(TraceInfo const &trace_info, std::string_view const &symbol) {
  log::warn<1>(R"(*** RESUBSCRIBE *** (symbol="{}"))"sv, symbol);
  auto market_by_price_update = MarketByPriceUpdate{
      .stream_id = stream_id_,
      .exchange = shared_.settings.exchange,
      .symbol = symbol,
      .bids = {},
      .asks = {},
      .update_type = UpdateType::STALE,
      .exchange_time_utc = {},
      .exchange_sequence = {},
      .sending_time_utc = {},
      .price_precision = {},
      .quantity_precision = {},
      .checksum = {},
  };
  log::info<3>("market_by_price_update={}"sv, market_by_price_update);
  create_trace_and_dispatch(handler_, trace_info, market_by_price_update, true);
  latch_.emplace(symbol);  // latch
  unsubscribe("book"sv, symbol);
  subscribe("book"sv, symbol);
}

}  // namespace kraken_futures
}  // namespace roq

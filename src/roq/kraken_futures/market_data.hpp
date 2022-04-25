/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/core/io/context.hpp"

#include "roq/core/web/client_socket.hpp"

#include "roq/server.hpp"

#include "roq/kraken_futures/shared.hpp"

#include "roq/kraken_futures/json/parser_public.hpp"

namespace roq {
namespace kraken_futures {

class MarketData final : public core::web::ClientSocket::Handler,
                         public json::ParserPublic::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus const> &) = 0;
    virtual void operator()(const Trace<ExternalLatency const> &) = 0;
    virtual void operator()(const Trace<MarketStatus const> &, bool is_last) = 0;
    virtual void operator()(const Trace<TopOfBook const> &, bool is_last) = 0;
    virtual void operator()(
        const Trace<MarketByPriceUpdate const> &, bool is_last, bool refresh) = 0;
    virtual void operator()(const Trace<TradeSummary const> &, bool is_last) = 0;
    virtual void operator()(const Trace<StatisticsUpdate const> &, bool is_last) = 0;
  };

  MarketData(Handler &, core::io::Context &, uint16_t stream_id, Shared &, size_t index);

  MarketData(MarketData &&) = delete;
  MarketData(const MarketData &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  void subscribe(size_t start_from = 0);

 protected:
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

  void operator()(ConnectionStatus);

  void subscribe(const std::span<Symbol const> &symbols);

  void subscribe(const std::string_view &feed);

  template <typename T>
  void subscribe(const std::string_view &feed, const std::span<T> &product_ids);
  void subscribe(const std::string_view &feed, const std::string_view &symbol) {
    return subscribe(feed, std::span{&symbol, 1});
  }

  template <typename T>
  void unsubscribe(const std::string_view &feed, const std::span<T> &product_ids);
  void unsubscribe(const std::string_view &feed, const std::string_view &symbol) {
    return unsubscribe(feed, std::span{&symbol, 1});
  }

  // json::ParserPublic::Handler

  void operator()(const Trace<json::Info const> &) override;
  void operator()(const Trace<json::Alert const> &) override;
  void operator()(const Trace<json::Error const> &) override;

  void operator()(const Trace<json::Subscribed const> &) override;

  void operator()(const Trace<json::Heartbeat const> &) override;

  void operator()(const Trace<json::Ticker const> &) override;
  void operator()(const Trace<json::BookSnapshot const> &) override;
  void operator()(const Trace<json::Book const> &) override;
  void operator()(const Trace<json::TradeSnapshot const> &) override;
  void operator()(const Trace<json::Trade const> &) override;

 private:
  void parse(const std::string_view &message);

  void reset();

  void resubscribe(const TraceInfo &, const std::string_view &symbol);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  const size_t index_;
  // web socket
  core::web::ClientSocket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, heartbeat, ticker, book_snapshot, book, trade_snapshot, trade;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  // state
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus status_ = {};
  // experimental
  absl::flat_hash_set<Symbol> latch_;
};

}  // namespace kraken_futures
}  // namespace roq

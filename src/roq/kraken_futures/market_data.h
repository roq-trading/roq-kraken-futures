/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/download.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client_socket.h"

#include "roq/server.h"

#include "roq/kraken_futures/shared.h"

#include "roq/kraken_futures/json/parser_public.h"

namespace roq {
namespace kraken_futures {

class MarketData final : public core::web::ClientSocket::Handler,
                         public json::ParserPublic::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
    virtual void operator()(const server::Trace<MarketStatus> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<TopOfBook> &, bool is_last) = 0;
    virtual void operator()(
        const server::Trace<MarketByPriceUpdate> &, bool is_last, bool refresh) = 0;
    virtual void operator()(const server::Trace<TradeSummary> &, bool is_last) = 0;
    virtual void operator()(const server::Trace<StatisticsUpdate> &, bool is_last) = 0;
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

  void subscribe(const std::span<std::string const> &symbols);

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

  void operator()(const server::Trace<json::Info> &) override;
  void operator()(const server::Trace<json::Alert> &) override;
  void operator()(const server::Trace<json::Error> &) override;

  void operator()(const server::Trace<json::Subscribed> &) override;

  void operator()(const server::Trace<json::Heartbeat> &) override;

  void operator()(const server::Trace<json::Ticker> &) override;
  void operator()(const server::Trace<json::BookSnapshot> &) override;
  void operator()(const server::Trace<json::Book> &) override;
  void operator()(const server::Trace<json::TradeSnapshot> &) override;
  void operator()(const server::Trace<json::Trade> &) override;

 private:
  void parse(const std::string_view &message);

  void reset();

  void resubscribe(const server::TraceInfo &, const std::string_view &symbol);

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
  absl::flat_hash_set<std::string> latch_;
};

}  // namespace kraken_futures
}  // namespace roq

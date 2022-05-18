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

class MarketData final : public core::web::ClientSocket::Handler, public json::ParserPublic::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<MarketStatus const> const &, bool is_last) = 0;
    virtual void operator()(Trace<TopOfBook const> const &, bool is_last) = 0;
    virtual void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) = 0;
    virtual void operator()(Trace<TradeSummary const> const &, bool is_last) = 0;
    virtual void operator()(Trace<StatisticsUpdate const> const &, bool is_last) = 0;
  };

  MarketData(Handler &, core::io::Context &, uint16_t stream_id, Shared &, size_t index);

  MarketData(MarketData &&) = delete;
  MarketData(MarketData const &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

  void subscribe(size_t start_from = 0);

 protected:
  void operator()(core::web::ClientSocket::Connected const &) override;
  void operator()(core::web::ClientSocket::Disconnected const &) override;
  void operator()(core::web::ClientSocket::Ready const &) override;
  void operator()(core::web::ClientSocket::Close const &) override;
  void operator()(core::web::ClientSocket::Latency const &) override;
  void operator()(core::web::ClientSocket::Text const &) override;
  void operator()(core::web::ClientSocket::Binary const &) override;

  void operator()(ConnectionStatus);

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::string_view const &feed);

  template <typename T>
  void subscribe(std::string_view const &feed, std::span<T> const &product_ids);
  void subscribe(std::string_view const &feed, std::string_view const &symbol) {
    return subscribe(feed, std::span{&symbol, 1});
  }

  template <typename T>
  void unsubscribe(std::string_view const &feed, std::span<T> const &product_ids);
  void unsubscribe(std::string_view const &feed, std::string_view const &symbol) {
    return unsubscribe(feed, std::span{&symbol, 1});
  }

  // json::ParserPublic::Handler

  void operator()(Trace<json::Info const> const &) override;
  void operator()(Trace<json::Alert const> const &) override;
  void operator()(Trace<json::Error const> const &) override;

  void operator()(Trace<json::Subscribed const> const &) override;

  void operator()(Trace<json::Heartbeat const> const &) override;

  void operator()(Trace<json::Ticker const> const &) override;
  void operator()(Trace<json::BookSnapshot const> const &) override;
  void operator()(Trace<json::Book const> const &) override;
  void operator()(Trace<json::TradeSnapshot const> const &) override;
  void operator()(Trace<json::Trade const> const &) override;

 private:
  void parse(std::string_view const &message);

  void reset();

  void resubscribe(TraceInfo const &, std::string_view const &symbol);

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

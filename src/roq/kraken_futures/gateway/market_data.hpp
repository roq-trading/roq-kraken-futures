/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/kraken_futures/gateway/shared.hpp"

#include "roq/kraken_futures/protocol/json/parser_public.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

struct MarketData final : public Base<MarketData>,
                          public server::MarketDataStream,
                          public web::socket::Client::Handler,
                          public protocol::json::ParserPublic::Handler {
  struct Handler {};

  MarketData(Handler &, io::Context &, uint16_t stream_id, Shared &, size_t index);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

  // server::MarketDataStream

  void subscribe(size_t start_from = 0) override;

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;

  // protocol::json::ParserPublic::Handler

  void operator()(Trace<protocol::json::Info> const &) override;
  void operator()(Trace<protocol::json::Alert> const &) override;
  void operator()(Trace<protocol::json::Error> const &) override;

  void operator()(Trace<protocol::json::Subscribed> const &) override;

  void operator()(Trace<protocol::json::Heartbeat> const &) override;

  void operator()(Trace<protocol::json::Ticker> const &) override;
  void operator()(Trace<protocol::json::BookSnapshot> const &) override;
  void operator()(Trace<protocol::json::Book> const &) override;
  void operator()(Trace<protocol::json::TradeSnapshot> const &) override;
  void operator()(Trace<protocol::json::Trade> const &) override;

  // helpers

  void subscribe(std::span<Symbol const> const &symbols);

  void subscribe(std::string_view const &feed);

  template <typename T>
  void subscribe(std::string_view const &feed, std::span<T> const &product_ids);
  void subscribe(std::string_view const &feed, std::string_view const &symbol) { subscribe(feed, std::span{&symbol, 1}); }

  template <typename T>
  void unsubscribe(std::string_view const &feed, std::span<T> const &product_ids);
  void unsubscribe(std::string_view const &feed, std::string_view const &symbol) { unsubscribe(feed, std::span{&symbol, 1}); }

  void parse(std::string_view const &message);

  void reset();

  void resubscribe(TraceInfo const &, std::string_view const &symbol);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  size_t const index_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, heartbeat, ticker, book_snapshot, book, trade_snapshot, trade;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // cache
  Shared &shared_;
  // state
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus connection_status_ = {};
  // experimental
  utils::unordered_set<std::string> latch_;
};

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

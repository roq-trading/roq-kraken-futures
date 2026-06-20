/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/kraken_futures/gateway/shared.hpp"

#include "roq/kraken_futures/protocol/json/candles.hpp"
#include "roq/kraken_futures/protocol/json/instruments.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

struct Rest final : public web::rest::Client::Handler {
  struct TokenUpdate final {
    std::string_view account;
    std::string_view token;
  };

  struct SymbolsUpdate final {
    std::span<Symbol const> symbols;
  };

  struct Handler {
    virtual void operator()(SymbolsUpdate &) = 0;
  };

  Rest(Handler &, io::Context &context, uint16_t stream_id, Shared &);

  Rest(Rest const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Client::Connected> const &) override;
  void operator()(Trace<web::rest::Client::Disconnected> const &) override;
  void operator()(Trace<web::rest::Client::Latency> const &) override;

  // helpers

  bool ready() const { return connection_status_ == ConnectionStatus::READY; }

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    INSTRUMENTS,
    DONE,
  };

  uint32_t download(State);

  // instruments

  void get_instruments();
  void get_instruments_ack(Trace<web::rest::Response> const &, uint32_t sequence);
  void operator()(Trace<protocol::json::Instruments> const &);

  // candles

  void get_candles(std::string_view const &symbol);
  void get_candles_ack(Trace<web::rest::Response> const &, std::string_view const &symbol);
  void operator()(Trace<protocol::json::Candles> const &, std::string_view const &symbol);

  // helpers

  void check_request_queue(std::chrono::nanoseconds now);

  void process_response(web::rest::Response const &, auto error_handler, auto success_handler);

 private:
  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // connection
  std::unique_ptr<web::rest::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile instruments, instruments_ack, candles, candles_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download<State> download_;
};

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

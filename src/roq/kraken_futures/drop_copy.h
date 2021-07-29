/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/socket.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/kraken_futures/drop_copy_state.h"
#include "roq/kraken_futures/security.h"
#include "roq/kraken_futures/shared.h"

#include "roq/kraken_futures/json/parser_private.h"

namespace roq {
namespace kraken_futures {

class DropCopy final : public core::web::Socket::Handler, public json::ParserPrivate::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
  };

  DropCopy(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(const DropCopy &) = delete;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(const core::web::Socket::Connected &) override;
  void operator()(const core::web::Socket::Disconnected &) override;
  void operator()(const core::web::Socket::Ready &) override;
  void operator()(const core::web::Socket::Close &) override;
  void operator()(const core::web::Socket::Latency &) override;
  void operator()(const core::web::Socket::Text &) override;

  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void get_challenge();

  void subscribe();
  void subscribe(const std::string_view &feed);

  // json::ParserPrivate::Handler

  void operator()(const server::Trace<json::Info> &) override;
  void operator()(const server::Trace<json::Alert> &) override;
  void operator()(const server::Trace<json::Error> &) override;

  void operator()(const server::Trace<json::Challenge> &) override;

  void operator()(const server::Trace<json::Subscribed> &) override;

  void operator()(const server::Trace<json::Heartbeat> &) override;

  void operator()(const server::Trace<json::AccountBalancesAndMargins> &) override;
  void operator()(const server::Trace<json::OpenPositions> &) override;

  void operator()(const server::Trace<json::OpenOrdersSnapshot> &) override;
  void operator()(const server::Trace<json::OpenOrders> &) override;

  void operator()(const server::Trace<json::FillsSnapshot> &) override;
  void operator()(const server::Trace<json::Fills> &) override;

 private:
  void parse(const std::string_view &message);

  void reset();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  core::web::Socket connection_;
  // buffers
  core::Buffer decode_buffer_;
  // core::stack::Buffer<char, 32> stack_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, challenge, heartbeat, account_balances_and_margins,
        open_positions, open_orders_snapshot, open_orders, fills_snapshot, fills;
  } profile_;
  struct {
    core::metrics::Latency ping, heartbeat;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  bool ready_ = false;
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus status_ = {};
  server::Download<DropCopyState> download_;
  // challenge
  std::string original_challenge_;
  std::string signed_challenge_;
};

}  // namespace kraken_futures
}  // namespace roq

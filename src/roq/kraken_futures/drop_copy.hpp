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

#include "roq/kraken_futures/drop_copy_state.hpp"
#include "roq/kraken_futures/security.hpp"
#include "roq/kraken_futures/shared.hpp"

#include "roq/kraken_futures/json/parser_private.hpp"

namespace roq {
namespace kraken_futures {

class DropCopy final : public core::web::ClientSocket::Handler,
                       public json::ParserPrivate::Handler {
 public:
  struct Handler {
    virtual void operator()(const Trace<StreamStatus const> &) = 0;
    virtual void operator()(const Trace<ExternalLatency const> &) = 0;
    virtual void operator()(const Trace<TradeUpdate const> &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(const Trace<FundsUpdate const> &, bool is_last) = 0;
    virtual void operator()(const Trace<PositionUpdate const> &, bool is_last) = 0;
  };

  DropCopy(Handler &, core::io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(const DropCopy &) = delete;

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(const core::web::ClientSocket::Connected &) override;
  void operator()(const core::web::ClientSocket::Disconnected &) override;
  void operator()(const core::web::ClientSocket::Ready &) override;
  void operator()(const core::web::ClientSocket::Close &) override;
  void operator()(const core::web::ClientSocket::Latency &) override;
  void operator()(const core::web::ClientSocket::Text &) override;
  void operator()(const core::web::ClientSocket::Binary &) override;

  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void get_challenge();

  void subscribe();
  void subscribe(const std::string_view &feed);

  // json::ParserPrivate::Handler

  void operator()(const Trace<json::Info const> &) override;
  void operator()(const Trace<json::Alert const> &) override;
  void operator()(const Trace<json::Error const> &) override;

  void operator()(const Trace<json::Challenge const> &) override;

  void operator()(const Trace<json::Subscribed const> &) override;

  void operator()(const Trace<json::Heartbeat const> &) override;

  void operator()(const Trace<json::AccountBalancesAndMargins const> &) override;
  void operator()(const Trace<json::OpenPositions const> &) override;

  void operator()(const Trace<json::OpenOrdersSnapshot const> &) override;
  void operator()(const Trace<json::OpenOrders const> &) override;

  void operator()(const Trace<json::FillsSnapshot const> &) override;
  void operator()(const Trace<json::Fills const> &) override;

 private:
  void parse(const std::string_view &message);

  void reset();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  core::web::ClientSocket connection_;
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
  core::Download<DropCopyState> download_;
  // challenge
  std::string original_challenge_;
  std::string signed_challenge_;
};

}  // namespace kraken_futures
}  // namespace roq

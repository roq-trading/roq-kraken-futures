/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/download.hpp"

#include "roq/core/metrics/counter.hpp"
#include "roq/core/metrics/latency.hpp"
#include "roq/core/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/server.hpp"

#include "roq/kraken_futures/drop_copy_state.hpp"
#include "roq/kraken_futures/security.hpp"
#include "roq/kraken_futures/shared.hpp"

#include "roq/kraken_futures/json/parser_private.hpp"

namespace roq {
namespace kraken_futures {

class DropCopy final : public web::socket::Client::Handler, public json::ParserPrivate::Handler {
 public:
  struct Handler {
    virtual void operator()(Trace<StreamStatus const> const &) = 0;
    virtual void operator()(Trace<ExternalLatency const> const &) = 0;
    virtual void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<FundsUpdate const> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate const> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Security &, Shared &);

  DropCopy(DropCopy &&) = delete;
  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &);

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  void operator()(ConnectionStatus);

  uint32_t download(DropCopyState);

  void get_challenge();

  void subscribe();
  void subscribe(std::string_view const &feed);

  // json::ParserPrivate::Handler

  void operator()(Trace<json::Info const> const &) override;
  void operator()(Trace<json::Alert const> const &) override;
  void operator()(Trace<json::Error const> const &) override;

  void operator()(Trace<json::Challenge const> const &) override;

  void operator()(Trace<json::Subscribed const> const &) override;

  void operator()(Trace<json::Heartbeat const> const &) override;

  void operator()(Trace<json::AccountBalancesAndMargins const> const &) override;
  void operator()(Trace<json::OpenPositions const> const &) override;

  void operator()(Trace<json::OpenOrdersSnapshot const> const &) override;
  void operator()(Trace<json::OpenOrders const> const &) override;

  void operator()(Trace<json::FillsSnapshot const> const &) override;
  void operator()(Trace<json::Fills const> const &) override;

 private:
  void parse(std::string_view const &message);

  void reset();

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  // web socket
  std::unique_ptr<web::socket::Client> connection_;
  // buffers
  core::Buffer decode_buffer_;
  // core::stack::Buffer<char, 32> stack_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile parse, challenge, heartbeat, account_balances_and_margins, open_positions,
        open_orders_snapshot, open_orders, fills_snapshot, fills;
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

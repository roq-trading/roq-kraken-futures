/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/kraken_futures/gateway/account.hpp"
#include "roq/kraken_futures/gateway/shared.hpp"

#include "roq/kraken_futures/protocol/json/parser_private.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

struct DropCopy final : public web::socket::Client::Handler, public protocol::json::ParserPrivate::Handler {
  struct Handler {
    virtual void operator()(Trace<StreamStatus> const &) = 0;
    virtual void operator()(Trace<ExternalLatency> const &) = 0;
    virtual void operator()(Trace<TradeUpdate> const &, bool is_last, uint8_t user_id) = 0;
    virtual void operator()(Trace<FundsUpdate> const &, bool is_last) = 0;
    virtual void operator()(Trace<PositionUpdate> const &, bool is_last) = 0;
  };

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  DropCopy(DropCopy const &) = delete;

  void operator()(Event<Start> const &);
  void operator()(Event<Stop> const &);
  void operator()(Event<Timer> const &);

  void operator()(metrics::Writer &) const;

 protected:
  void operator()(web::socket::Client::Connected const &) override;
  void operator()(web::socket::Client::Disconnected const &) override;
  void operator()(web::socket::Client::Ready const &) override;
  void operator()(web::socket::Client::Close const &) override;
  void operator()(web::socket::Client::Latency const &) override;
  void operator()(web::socket::Client::Text const &) override;
  void operator()(web::socket::Client::Binary const &) override;

  void operator()(ConnectionStatus, std::string_view const &reason = {});

  enum class State {
    UNDEFINED = 0,
    GET_CHALLENGE,
    SUBSCRIBE,
    DONE,
  };

  uint32_t download(State);

  void get_challenge();

  void subscribe();
  void subscribe(std::string_view const &feed);

  // protocol::json::ParserPrivate::Handler

  void operator()(Trace<protocol::json::Info> const &) override;
  void operator()(Trace<protocol::json::Alert> const &) override;
  void operator()(Trace<protocol::json::Error> const &) override;

  void operator()(Trace<protocol::json::Challenge> const &) override;

  void operator()(Trace<protocol::json::Subscribed> const &) override;

  void operator()(Trace<protocol::json::Heartbeat> const &) override;

  void operator()(Trace<protocol::json::AccountBalancesAndMargins> const &) override;
  void operator()(Trace<protocol::json::OpenPositions> const &) override;

  void operator()(Trace<protocol::json::OpenOrdersSnapshot> const &) override;
  void operator()(Trace<protocol::json::OpenOrders> const &) override;

  void operator()(Trace<protocol::json::FillsSnapshot> const &) override;
  void operator()(Trace<protocol::json::Fills> const &) override;

  // helpers

  void process_order(
      auto &order,
      std::string_view const &order_id,
      std::string_view const &cli_ord_id,
      protocol::json::Reason,
      bool is_cancel,
      TraceInfo const &,
      bool is_download);

 private:
  void parse(std::string_view const &message);

  void reset();

  Handler &handler_;
  // config
  uint16_t const stream_id_;
  std::string const name_;
  // web socket
  std::unique_ptr<web::socket::Client> const connection_;
  // buffers
  core::json::BufferStack decode_buffer_;
  // metrics
  struct {
    utils::metrics::Counter disconnect;
  } counter_;
  struct {
    utils::metrics::Profile parse, challenge, heartbeat, account_balances_and_margins, open_positions, open_orders_snapshot, open_orders, fills_snapshot, fills;
  } profile_;
  struct {
    utils::metrics::Latency ping, heartbeat;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  bool ready_ = false;
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus connection_status_ = {};
  core::Download<State> download_;
  // challenge
  std::string original_challenge_;
  std::string signed_challenge_;
  // workaround
  utils::unordered_map<std::string, bool> fill_symbols_;
};

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

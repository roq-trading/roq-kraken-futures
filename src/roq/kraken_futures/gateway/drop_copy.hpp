/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/container.hpp"

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/socket/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/kraken_futures/gateway/account.hpp"
#include "roq/kraken_futures/gateway/shared.hpp"

#include "roq/kraken_futures/protocol/json/parser_private.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

struct DropCopy final : public Base<DropCopy>, public server::Stream, public web::socket::Client::Handler, public protocol::json::ParserPrivate::Handler {
  struct Handler {};

  DropCopy(Handler &, io::Context &, uint16_t stream_id, Account &, Shared &);

  // protected:
  friend base_type;

  // server::Stream

  uint16_t stream_id() const override { return stream_id_; }

  bool ready() const override { return connection_status_ == ConnectionStatus::READY; }

  void operator()(Trace<Start> const &) override;
  void operator()(Trace<Stop> const &) override;
  void operator()(Trace<Timer> const &) override;

  void operator()(metrics::Writer &) const override;

  void operator()(Trace<ConnectionStatus> const &, std::string_view const &reason = {}) override;

 protected:
  // web::socket::Client::Handler

  void operator()(Trace<web::socket::Connected> const &) override;
  void operator()(Trace<web::socket::Disconnected> const &) override;
  void operator()(Trace<web::socket::Ready> const &) override;
  void operator()(Trace<web::socket::Close> const &) override;
  void operator()(Trace<web::socket::Latency> const &) override;
  void operator()(Trace<web::socket::Text> const &) override;
  void operator()(Trace<web::socket::Binary> const &) override;

  // core::Download

  enum class State {
    UNDEFINED = 0,
    GET_CHALLENGE,
    SUBSCRIBE,
    DONE,
  };

  int32_t download(Trace<State> const &);

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

  void get_challenge();

  void subscribe();
  void subscribe(std::string_view const &feed);

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
  core::Download2<State> download_;
  // challenge
  std::string original_challenge_;
  std::string signed_challenge_;
  // workaround
  utils::unordered_map<std::string, bool> fill_symbols_;
};

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>

#include "roq/utils/metrics/counter.hpp"
#include "roq/utils/metrics/latency.hpp"
#include "roq/utils/metrics/profile.hpp"

#include "roq/io/context.hpp"

#include "roq/web/rest/client.hpp"

#include "roq/core/download_2.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/server.hpp"

#include "roq/server/stream.hpp"

#include "roq/kraken_futures/gateway/account.hpp"
#include "roq/kraken_futures/gateway/shared.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

struct OrderEntry final : public Base<OrderEntry>, public server::OrderActionStream, public web::rest::Client::Handler {
  struct Handler {};

  OrderEntry(Handler &, io::Context &context, uint16_t stream_id, Account &, Shared &);

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

  // server::OrderActionStream

  uint16_t operator()(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

 protected:
  // web::rest::Client::Handler

  void operator()(Trace<web::rest::Connected> const &) override;
  void operator()(Trace<web::rest::Disconnected> const &) override;
  void operator()(Trace<web::rest::Latency> const &) override;

  // create-order

  void create_order(Event<CreateOrder> const &, server::oms::Order const &, server::oms::RefData const &, std::string_view const &request_id);
  void create_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // modify-order

  void modify_order(
      Event<ModifyOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void modify_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-order

  void cancel_order(
      Event<CancelOrder> const &,
      server::oms::Order const &,
      server::oms::RefData const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
  void cancel_order_ack(Trace<web::rest::Response> const &, uint8_t user_id, uint64_t order_id, uint32_t version);

  // cancel-all-orders

  void cancel_all_orders(Event<CancelAllOrders> const &, std::string_view const &request_id);
  void cancel_all_orders_ack(Trace<web::rest::Response> const &, std::string_view const &request_id);

  // cancel-all-orders-after

  void cancel_all_orders_after(std::chrono::nanoseconds timeout);
  void cancel_all_orders_after_ack(Trace<web::rest::Response> const &);

  // core::Download

  enum class State {
    UNDEFINED = 0,
    DONE,
  };

  int32_t download(Trace<State> const &);

  // helpers

  void process_response(Trace<web::rest::Response> const &, auto error_handler, auto success_handler);

  template <typename Accept, typename Reject>
  void process_send_order(auto &request_status, Accept, Reject);

  template <typename Accept, typename Reject>
  void process_edit_order(auto &request_status, Accept, Reject);

  template <typename Accept, typename Reject>
  void process_cancel_order(auto &request_status, Accept, Reject);

  // helpers 2

  template <typename Callback>
  void process_place(auto &request_status, Callback);

  template <typename Callback>
  void process_edit(auto &request_status, Callback);

  template <typename Callback>
  void process_cancel(auto &request_status, Callback);

  template <typename Callback>
  void process_execution(auto &request_status, Callback);

 private:
  [[maybe_unused]] Handler &handler_;
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
    utils::metrics::Profile create_order, create_order_ack,  //
        modify_order, modify_order_ack,                      //
        cancel_order, cancel_order_ack,                      //
        cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    utils::metrics::Latency ping;
  } latency_;
  // account
  Account &account_;
  // cache
  Shared &shared_;
  // state
  ConnectionStatus connection_status_ = {};
  core::Download2<State> download_;
  // cancel all
  std::chrono::nanoseconds next_cancel_all_timer_ = {};
  //
  std::string encode_buffer_;
};

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

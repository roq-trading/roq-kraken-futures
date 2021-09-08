/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_set.h>

#include <string>
#include <string_view>
#include <vector>

#include "roq/core/promise.h"

#include "roq/core/buffer.h"

#include "roq/core/metrics/counter.h"
#include "roq/core/metrics/latency.h"
#include "roq/core/metrics/profile.h"

#include "roq/core/io/context.h"

#include "roq/core/web/client.h"

#include "roq/download.h"
#include "roq/server.h"

#include "roq/kraken_futures/order_entry_state.h"
#include "roq/kraken_futures/security.h"
#include "roq/kraken_futures/shared.h"

namespace roq {
namespace kraken_futures {

class OrderEntry final : public core::web::Client::Handler {
 public:
  struct Handler {
    virtual void operator()(const server::Trace<StreamStatus> &) = 0;
    virtual void operator()(const server::Trace<ExternalLatency> &) = 0;
  };

  OrderEntry(
      Handler &, core::io::Context &context, uint16_t stream_id, Security &, Shared &, bool master);

  OrderEntry(OrderEntry &&) = delete;
  OrderEntry(const OrderEntry &) = delete;

  bool ready() const { return status_ == ConnectionStatus::READY; }

  void operator()(const Event<Start> &);
  void operator()(const Event<Stop> &);
  void operator()(const Event<Timer> &);

  void operator()(metrics::Writer &);

  uint16_t operator()(const Event<CreateOrder> &, const std::string_view &request_id);
  uint16_t operator()(
      const Event<ModifyOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);
  uint16_t operator()(
      const Event<CancelOrder> &,
      const oms::Order &,
      const std::string_view &request_id,
      const std::string_view &previous_request_id);

  uint16_t operator()(const Event<CancelAllOrders> &, const std::string_view &request_id);

 protected:
  void operator()(const core::web::Client::Connected &) override;
  void operator()(const core::web::Client::Disconnected &) override;
  void operator()(const core::web::Client::Latency &) override;

  void operator()(ConnectionStatus);

  void create_order_ack(
      const core::web::Response &, const uint8_t user_id, const uint32_t order_id);

  void modify_order_ack(
      const core::web::Response &,
      const uint8_t user_id,
      const uint32_t order_id,
      const uint32_t version);

  void cancel_order_ack(
      const core::web::Response &,
      const uint8_t user_id,
      const uint32_t order_id,
      const uint32_t version);

  void cancel_all_orders_ack(const core::web::Response &);

  void cancel_all_after(std::chrono::nanoseconds timeout);

  void cancel_all_after_ack(const core::web::Response &);

  template <typename T>
  void get(std::function<void(const core::Promise<T> &)> &&callback);

  uint32_t download(OrderEntryState);

 private:
  Handler &handler_;
  // config
  const uint16_t stream_id_;
  const std::string name_;
  const bool master_;
  // connection
  core::web::Client connection_;
  // buffers
  core::Buffer decode_buffer_;
  // metrics
  struct {
    core::metrics::Counter disconnect;
  } counter_;
  struct {
    core::metrics::Profile create_order, create_order_ack, modify_order, modify_order_ack,
        cancel_order, cancel_order_ack, cancel_all_orders, cancel_all_orders_ack;
  } profile_;
  struct {
    core::metrics::Latency ping;
  } latency_;
  // security
  Security &security_;
  // cache
  Shared &shared_;
  // state
  std::chrono::nanoseconds next_heartbeat_ = {};
  ConnectionStatus status_ = {};
  server::Download<OrderEntryState> download_;
  // cancel all
  std::chrono::nanoseconds next_cancel_all_timer_ = {};
};

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/kraken_futures/shared.h"

#include "roq/kraken_futures/json/cancel_order.h"
#include "roq/kraken_futures/json/edit_order.h"
#include "roq/kraken_futures/json/open_orders.h"
#include "roq/kraken_futures/json/open_orders_snapshot.h"
#include "roq/kraken_futures/json/send_order.h"

namespace roq {
namespace kraken_futures {

class OrderUpdate final {
 public:
  explicit OrderUpdate(Shared &shared, uint16_t stream_id, const std::string_view &account)
      : shared_(shared), stream_id_(stream_id), account_(account) {}

  OrderUpdate(OrderUpdate &&) = delete;
  OrderUpdate(const OrderUpdate &) = delete;

  void operator()(const json::SendOrder &, const server::TraceInfo &, uint32_t order_id);
  void operator()(const json::EditOrder &, const server::TraceInfo &, uint32_t order_id);
  void operator()(const json::CancelOrder &, const server::TraceInfo &, uint32_t order_id);

  void operator()(const json::OpenOrdersSnapshot &, const server::TraceInfo &);
  void operator()(const json::OpenOrders &, const server::TraceInfo &);

 protected:
  void operator()(const json::Order &, const server::TraceInfo &);

 private:
  Shared &shared_;
  const uint16_t stream_id_;
  const std::string_view account_;
};

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/order_update.h"

#include <algorithm>
#include <string>

#include "roq/logging.h"

#include "roq/kraken_futures/flags.h"

#include "roq/kraken_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

void OrderUpdate::operator()(
    const json::OpenOrdersSnapshot &open_orders_snapshot, const server::TraceInfo &trace_info) {
  for (auto &order : open_orders_snapshot.orders)
    (*this)(order, trace_info);
}

void OrderUpdate::operator()(
    const json::OpenOrders &open_orders, const server::TraceInfo &trace_info) {
  (*this)(open_orders.order, trace_info);
}

void OrderUpdate::operator()(const json::Order &order, const server::TraceInfo &trace_info) {
  // note! we receive upper-case symbol from websocket
  auto side = compute_side(order.direction);
  auto order_type = json::map(order.type);
  auto status = OrderStatus::WORKING;  // XXX is there anything else ???
  roq::OrderUpdate order_update{
      .stream_id = stream_id_,
      .account = account_,
      .order_id = {},
      .exchange = Flags::exchange(),
      .symbol = order.instrument,
      .side = side,
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = order_type,
      .time_in_force = {},
      .execution_instruction = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = order.last_update_time,
      .external_account = {},
      .external_order_id = order.order_id,
      .status = status,
      .quantity = order.qty,
      .price = order.limit_price,
      .stop_price = NaN,
      .remaining_quantity = NaN,
      .traded_quantity = order.filled,
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
  };
  auto request_id = order.cli_ord_id;
  if (shared_.find_order(
          stream_id_,
          trace_info,
          order_update,
          request_id,
          [&]([[maybe_unused]] auto &order, [[maybe_unused]] auto callback) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"_sv);
    log::warn("order={}"_sv, order);
  }
}

}  // namespace kraken_futures
}  // namespace roq

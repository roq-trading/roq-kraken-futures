/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/order_update.h"

#include <algorithm>
#include <string>

#include "roq/logging.h"

#include "roq/utils/compare.h"

#include "roq/kraken_futures/flags.h"

#include "roq/kraken_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

void OrderUpdate::operator()(
    const json::OpenOrdersSnapshot &open_orders_snapshot, const server::TraceInfo &trace_info) {
  for (auto &order : open_orders_snapshot.orders)
    (*this)(order, order.order_id, order.cli_ord_id, {}, false, trace_info);
}

void OrderUpdate::operator()(
    const json::OpenOrders &open_orders, const server::TraceInfo &trace_info) {
  auto &order = open_orders.order;
  // ... just confusing
  auto order_id = open_orders.order_id.empty() ? order.order_id : open_orders.order_id;
  auto cli_ord_id = open_orders.cli_ord_id.empty() ? order.cli_ord_id : open_orders.cli_ord_id;
  (*this)(order, order_id, cli_ord_id, open_orders.reason, open_orders.is_cancel, trace_info);
}

namespace {
static RequestType compute_request_type(const json::Reason reason) {
  switch (reason) {
    case json::Reason::UNDEFINED:
    case json::Reason::UNKNOWN:
      break;
    case json::Reason::NEW_PLACED_ORDER_BY_USER:
      return RequestType::CREATE_ORDER;
    case json::Reason::LIQUIDATION:
      break;
    case json::Reason::STOP_ORDER_TRIGGERED:
      break;
    case json::Reason::LIMIT_ORDER_FROM_STOP:
      break;
    case json::Reason::PARTIAL_FILL:
      break;
    case json::Reason::FULL_FILL:
      break;
    case json::Reason::CANCELLED_BY_USER:
      return RequestType::CANCEL_ORDER;
    case json::Reason::CONTRACT_EXPIRED:
      break;
    case json::Reason::NOT_ENOUGH_MARGIN:
      break;
    case json::Reason::MARKET_INACTIVE:
      break;
    case json::Reason::CANCELLED_BY_ADMIN:
      break;
    case json::Reason::DEAD_MAN_SWITCH:
      break;
    case json::Reason::IOC_ORDER_FAILED_BECAUSE_IT_WOULD_NOT:
      break;
    case json::Reason::POST_ORDER_FAILED_BECAUSE_IT_WOULD_FILLED:
      break;
    case json::Reason::WOULD_EXECUTE_SELF:
      break;
    case json::Reason::WOULD_NOT_REDUCE_POSITION:
      break;
    case json::Reason::ORDER_FOR_EDIT_NOT_FOUND:
      break;
    case json::Reason::EDITED_BY_USER:
      return RequestType::MODIFY_ORDER;
  }
  return {};
}

static OrderStatus compute_order_status_2(
    json::Reason reason, bool is_cancel, double quantity, double filled) {
  switch (reason) {
    case json::Reason::UNDEFINED:
    case json::Reason::UNKNOWN:
      break;
    case json::Reason::NEW_PLACED_ORDER_BY_USER:
      return OrderStatus::WORKING;  // note! is this correct also for market/stop orders?
    case json::Reason::LIQUIDATION:
      break;
    case json::Reason::STOP_ORDER_TRIGGERED:
      // XXX is this a market order?
      break;
    case json::Reason::LIMIT_ORDER_FROM_STOP:
      return OrderStatus::WORKING;
    case json::Reason::PARTIAL_FILL:
      return OrderStatus::WORKING;
    case json::Reason::FULL_FILL:
      return OrderStatus::COMPLETED;
    case json::Reason::CANCELLED_BY_USER:
      return OrderStatus::CANCELED;
    case json::Reason::CONTRACT_EXPIRED:
      return OrderStatus::CANCELED;
    case json::Reason::NOT_ENOUGH_MARGIN:
      return OrderStatus::CANCELED;
    case json::Reason::MARKET_INACTIVE:
      break;
    case json::Reason::CANCELLED_BY_ADMIN:
      return OrderStatus::CANCELED;
    case json::Reason::DEAD_MAN_SWITCH:
      return OrderStatus::CANCELED;  // note! what is this?
    case json::Reason::IOC_ORDER_FAILED_BECAUSE_IT_WOULD_NOT:
      return OrderStatus::CANCELED;
    case json::Reason::POST_ORDER_FAILED_BECAUSE_IT_WOULD_FILLED:
      return OrderStatus::CANCELED;
    case json::Reason::WOULD_EXECUTE_SELF:
      return OrderStatus::CANCELED;
    case json::Reason::WOULD_NOT_REDUCE_POSITION:
      return OrderStatus::CANCELED;
    case json::Reason::ORDER_FOR_EDIT_NOT_FOUND:
      break;
    case json::Reason::EDITED_BY_USER:
      break;
  }
  if (utils::compare(quantity, filled) == 0)
    return OrderStatus::COMPLETED;
  // note! is_cancel is also true when completed -- so wait until other options have been exhausted
  if (is_cancel)
    return OrderStatus::CANCELED;
  return OrderStatus::WORKING;
}
}  // namespace

void OrderUpdate::operator()(
    const json::Order &order,
    [[maybe_unused]] const std::string_view &order_id,
    const std::string_view &cli_ord_id,
    const json::Reason reason,
    bool is_cancel,
    const server::TraceInfo &trace_info) {
  if (shared_.find_order(
          cli_ord_id, [&]([[maybe_unused]] auto &order_2, [[maybe_unused]] auto callback) {
            // XXX HANS we need some other way to update order
            // note! we receive upper-case symbol from websocket
            auto side = compute_side(order.direction);
            auto order_type = json::map(order.type);
            auto status = compute_order_status_2(reason, is_cancel, order.qty, order.filled);
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
            auto request_type = compute_request_type(reason);  // XXX HANS NEW
            auto request_id = cli_ord_id;
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
          })) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"_sv);
    log::warn("order={}"_sv, order);
  }
}

}  // namespace kraken_futures
}  // namespace roq

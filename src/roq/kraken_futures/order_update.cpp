/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/order_update.hpp"

#include <algorithm>
#include <string>

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

void OrderUpdate::operator()(json::OpenOrdersSnapshot const &open_orders_snapshot, TraceInfo const &trace_info) {
  for (auto &order : open_orders_snapshot.orders)
    (*this)(order, order.order_id, order.cli_ord_id, {}, false, trace_info, true);
}

void OrderUpdate::operator()(json::OpenOrders const &open_orders, TraceInfo const &trace_info) {
  auto &order = open_orders.order;
  // ... just confusing
  auto order_id = std::empty(open_orders.order_id) ? order.order_id : open_orders.order_id;
  auto cli_ord_id = std::empty(open_orders.cli_ord_id) ? order.cli_ord_id : open_orders.cli_ord_id;
  (*this)(order, order_id, cli_ord_id, open_orders.reason, open_orders.is_cancel, trace_info, false);
}

namespace {
/*
RequestType compute_request_type(const json::Reason reason) {
  switch (reason) {
    using enum json::Reason::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case NEW_PLACED_ORDER_BY_USER:
      return RequestType::CREATE_ORDER;
    case LIQUIDATION:
      break;
    case STOP_ORDER_TRIGGERED:
      break;
    case LIMIT_ORDER_FROM_STOP:
      break;
    case PARTIAL_FILL:
      break;
    case FULL_FILL:
      break;
    case CANCELLED_BY_USER:
      return RequestType::CANCEL_ORDER;
    case CONTRACT_EXPIRED:
      break;
    case NOT_ENOUGH_MARGIN:
      break;
    case MARKET_INACTIVE:
      break;
    case CANCELLED_BY_ADMIN:
      break;
    case DEAD_MAN_SWITCH:
      break;
    case IOC_ORDER_FAILED_BECAUSE_IT_WOULD_NOT:
      break;
    case POST_ORDER_FAILED_BECAUSE_IT_WOULD_FILLED:
      break;
    case WOULD_EXECUTE_SELF:
      break;
    case WOULD_NOT_REDUCE_POSITION:
      break;
    case ORDER_FOR_EDIT_NOT_FOUND:
      break;
    case EDITED_BY_USER:
      return RequestType::MODIFY_ORDER;
  }
  return {};
}
*/
OrderStatus compute_order_status_2(json::Reason reason, bool is_cancel, double quantity, double filled) {
  switch (reason) {
    using enum json::Reason::type_t;
    case UNDEFINED__:
    case UNKNOWN__:
      break;
    case NEW_PLACED_ORDER_BY_USER:
      return OrderStatus::WORKING;  // note! is this correct also for market/stop orders?
    case LIQUIDATION:
      break;
    case STOP_ORDER_TRIGGERED:
      // XXX is this a market order?
      break;
    case LIMIT_ORDER_FROM_STOP:
      return OrderStatus::WORKING;
    case PARTIAL_FILL:
      return OrderStatus::WORKING;
    case FULL_FILL:
      return OrderStatus::COMPLETED;
    case CANCELLED_BY_USER:
      return OrderStatus::CANCELED;
    case CONTRACT_EXPIRED:
      return OrderStatus::CANCELED;
    case NOT_ENOUGH_MARGIN:
      return OrderStatus::CANCELED;
    case MARKET_INACTIVE:
      break;
    case CANCELLED_BY_ADMIN:
      return OrderStatus::CANCELED;
    case DEAD_MAN_SWITCH:
      return OrderStatus::CANCELED;  // note! what is this?
    case IOC_ORDER_FAILED_BECAUSE_IT_WOULD_NOT:
      return OrderStatus::CANCELED;
    case POST_ORDER_FAILED_BECAUSE_IT_WOULD_FILLED:
      return OrderStatus::CANCELED;
    case WOULD_EXECUTE_SELF:
      return OrderStatus::CANCELED;
    case WOULD_NOT_REDUCE_POSITION:
      return OrderStatus::CANCELED;
    case ORDER_FOR_EDIT_NOT_FOUND:
      break;
    case EDITED_BY_USER:
      break;
  }
  if (utils::is_equal(quantity, filled))
    return OrderStatus::COMPLETED;
  // note! is_cancel is also true when completed -- so wait until other options have been exhausted
  if (is_cancel)
    return OrderStatus::CANCELED;
  return OrderStatus::WORKING;
}
}  // namespace

void OrderUpdate::operator()(
    json::Order const &order,
    [[maybe_unused]] std::string_view const &order_id,
    std::string_view const &cli_ord_id,
    json::Reason const reason,
    bool is_cancel,
    TraceInfo const &trace_info,
    bool is_download) {
  auto side = compute_side(order.direction);
  auto order_status = compute_order_status_2(reason, is_cancel, order.qty, order.filled);
  auto update_type = is_download ? UpdateType::SNAPSHOT : UpdateType::INCREMENTAL;
  auto order_update = server::oms::OrderUpdate{
      .account = account_,
      .exchange = shared_.settings.exchange,
      .symbol = order.instrument,
      .side = side,
      .position_effect = {},
      .margin_mode = {},
      .max_show_quantity = NaN,
      .order_type = map(order.type),
      .time_in_force = {},
      .execution_instructions = {},
      .create_time_utc = {},
      .update_time_utc = order.last_update_time,
      .external_account = {},
      .external_order_id = order.order_id,
      .client_order_id = {},
      .order_status = order_status,
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
      .update_type = update_type,
      .sending_time_utc = {},
  };
  // auto request_type = compute_request_type(reason);
  auto request_id = cli_ord_id;
  if (shared_.update_order(request_id, stream_id_, trace_info, order_update, []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order={}"sv, order);
  }
}

}  // namespace kraken_futures
}  // namespace roq

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

// === HELPERS ===

namespace {
auto compute_side(int32_t direction) -> Side {
  switch (direction) {
    case -1:
      break;
    case 0:
      return Side::BUY;
    case 1:
      return Side::SELL;
    default:
      break;
  }
  return {};
}

auto compute_order_status_2(json::Reason reason, bool is_cancel, double remaining_quantity) -> OrderStatus {
  auto result = map(reason).template get<OrderStatus>();
  if (result != OrderStatus{}) {
    return result;
  }
  if (utils::is_zero(remaining_quantity)) {
    return OrderStatus::COMPLETED;
  }
  if (is_cancel) {
    return OrderStatus::CANCELED;
  }
  return OrderStatus::WORKING;
}
}  // namespace

// === IMPLEMENTATION ===

void OrderUpdate::operator()(Trace<json::OpenOrdersSnapshot> const &event) {
  auto &[trace_info, open_orders_snapshot] = event;
  for (auto &order : open_orders_snapshot.orders) {
    (*this)(order, order.order_id, order.cli_ord_id, {}, false, trace_info, true);
  }
}

void OrderUpdate::operator()(Trace<json::OpenOrders> const &event) {
  auto &[trace_info, open_orders] = event;
  auto &order = open_orders.order;
  auto order_id = [&]() {
    if (std::empty(open_orders.order_id)) {
      return order.order_id;
    }
    return open_orders.order_id;
  }();
  auto cli_ord_id = [&]() {
    if (std::empty(open_orders.cli_ord_id)) {
      return order.cli_ord_id;
    }
    return open_orders.cli_ord_id;
  }();
  (*this)(order, order_id, cli_ord_id, open_orders.reason, open_orders.is_cancel, trace_info, false);
}

void OrderUpdate::operator()(
    json::Order const &order,
    [[maybe_unused]] std::string_view const &order_id,
    std::string_view const &cli_ord_id,
    json::Reason reason,
    bool is_cancel,
    TraceInfo const &trace_info,
    bool is_download) {
  auto side = compute_side(order.direction);
  auto order_status = compute_order_status_2(reason, is_cancel, order.qty);
  auto quantity = order.qty + order.filled;
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
      .create_time_utc = order.time,
      .update_time_utc = order.last_update_time,
      .external_account = {},
      .external_order_id = order.order_id,
      .client_order_id = cli_ord_id,
      .order_status = order_status,
      .quantity = quantity,  // note!
      .price = order.limit_price,
      .stop_price = order.stop_price,
      .remaining_quantity = order.qty,  // note!
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
  log::warn("DEBUG order_update={}"sv, order_update);
  auto request_id = cli_ord_id;
  if (shared_.update_order(request_id, stream_id_, trace_info, order_update, []([[maybe_unused]] auto &order) {})) {
  } else {
    log::warn("*** EXTERNAL ORDER ***"sv);
    log::warn("order={}"sv, order);
  }
}

}  // namespace kraken_futures
}  // namespace roq

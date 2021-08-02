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
    const json::EditOrder &edit_order, const server::TraceInfo &trace_info, uint32_t order_id) {
  if (edit_order.result != json::Result::SUCCESS) {
    log::warn("edit_order={}"_sv, edit_order);
    throw RuntimeErrorException("{}"_sv, edit_order.error);
  }
  auto &edit_status = edit_order.edit_status;
  if (std::size(edit_status.order_events) != 1)
    throw RuntimeErrorException("Unexpected: size={}"_sv, std::size(edit_status.order_events));
  auto &order_event = edit_status.order_events[0];
  switch (order_event.type) {
    case json::OrderEventType::UNDEFINED:
    case json::OrderEventType::UNKNOWN:
    case json::OrderEventType::PLACE:
    case json::OrderEventType::CANCEL:
      throw RuntimeErrorException("Unexpected: type={}"_sv, order_event.type);
      break;
    case json::OrderEventType::EDIT: {
      auto &new_order = order_event.new_;
      auto symbol = std::string{new_order.symbol};
      std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
      auto side = json::map(new_order.side);
      auto order_type = json::map(new_order.type);
      auto status = compute_order_status(edit_status.status);
      // XXX HANS should we use reduced_quantity to compute remaining quantity ???
      roq::OrderUpdate order_update{
          .stream_id = stream_id_,
          .account = account_,
          .order_id = order_id,
          .exchange = Flags::exchange(),
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = {},
          .execution_instruction = {},
          .order_template = {},
          .create_time_utc = {},
          .update_time_utc = edit_status.received_time,
          .external_account = {},
          .external_order_id = edit_status.order_id,
          .status = status,
          .quantity = new_order.quantity,
          .price = new_order.limit_price,
          .stop_price = NaN,
          .remaining_quantity = NaN,
          .traded_quantity = new_order.filled,
          .average_traded_price = NaN,
          .last_traded_quantity = NaN,
          .last_traded_price = NaN,
          .last_liquidity = {},
          .routing_id = {},
          .max_request_version = {},
          .max_response_version = {},
          .max_accepted_version = {},
      };
      auto request_id = edit_status.cli_ord_id;
      if (shared_.find_order(
              stream_id_, trace_info, order_update, request_id, [&](auto &order, auto callback) {
                server::Ack ack{
                    .stream_id = stream_id_,
                    .account = account_,
                    .order_id = order.order_id,
                    .type = RequestType::MODIFY_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::ACCEPTED,
                    .error = {},
                    .text = {},
                    .version = {},
                    .request_id = request_id,
                };
                server::Trace order_event(trace_info, ack);
                callback(order_event, true, order.user_id);
              })) {
      } else {
        log::warn("*** EXTERNAL ORDER ***"_sv);
        log::warn("edit_order={}"_sv, edit_order);
      }
      break;
    }
    case json::OrderEventType::REJECT: {
      auto &order = order_event.order;
      auto symbol = std::string{order.symbol};
      std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
      auto side = json::map(order.side);
      auto order_type = json::map(order.type);
      auto status = compute_order_status(edit_status.status);
      // XXX HANS should we use reduced_quantity to compute remaining quantity ???
      roq::OrderUpdate order_update{
          .stream_id = stream_id_,
          .account = account_,
          .order_id = order_id,
          .exchange = Flags::exchange(),
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = {},
          .execution_instruction = {},
          .order_template = {},
          .create_time_utc = {},
          .update_time_utc = edit_status.received_time,
          .external_account = {},
          .external_order_id = edit_status.order_id,
          .status = status,
          .quantity = order.quantity,
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
      auto request_id = edit_status.cli_ord_id;
      if (shared_.find_order(
              stream_id_, trace_info, order_update, request_id, [&](auto &order, auto callback) {
                server::Ack ack{
                    .stream_id = stream_id_,
                    .account = account_,
                    .order_id = order.order_id,
                    .type = RequestType::MODIFY_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::REJECTED,
                    .error = Error::UNKNOWN,
                    .text = order_event.reason,
                    .version = {},
                    .request_id = request_id,
                };
                server::Trace event(trace_info, ack);
                callback(event, true, order.user_id);
              })) {
      } else {
        log::warn("*** EXTERNAL ORDER ***"_sv);
        log::warn("edit_order={}"_sv, edit_order);
      }
      break;
    }
  }
}

void OrderUpdate::operator()(
    const json::CancelOrder &cancel_order, const server::TraceInfo &trace_info, uint32_t order_id) {
  if (cancel_order.result != json::Result::SUCCESS) {
    log::warn("cancel_order={}"_sv, cancel_order);
    throw RuntimeErrorException("{}"_sv, cancel_order.error);
  }
  auto &cancel_status = cancel_order.cancel_status;
  if (std::size(cancel_status.order_events) != 1)
    throw RuntimeErrorException("Unexpected: size={}"_sv, std::size(cancel_status.order_events));
  auto &order_event = cancel_status.order_events[0];
  switch (order_event.type) {
    case json::OrderEventType::UNDEFINED:
    case json::OrderEventType::UNKNOWN:
    case json::OrderEventType::PLACE:
    case json::OrderEventType::EDIT:
      throw RuntimeErrorException("Unexpected: type={}"_sv, order_event.type);
      break;
    case json::OrderEventType::CANCEL: {
      auto &order = order_event.new_;
      auto symbol = std::string{order.symbol};
      std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
      auto side = json::map(order.side);
      auto order_type = json::map(order.type);
      auto status = compute_order_status(cancel_status.status);
      roq::OrderUpdate order_update{
          .stream_id = stream_id_,
          .account = account_,
          .order_id = order_id,
          .exchange = Flags::exchange(),
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = {},
          .execution_instruction = {},
          .order_template = {},
          .create_time_utc = {},
          .update_time_utc = cancel_status.received_time,
          .external_account = {},
          .external_order_id = cancel_status.order_id,
          .status = status,
          .quantity = order.quantity,
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
      auto request_id = cancel_status.cli_ord_id;
      if (shared_.find_order(
              stream_id_, trace_info, order_update, request_id, [&](auto &order, auto callback) {
                server::Ack ack{
                    .stream_id = stream_id_,
                    .account = account_,
                    .order_id = order.order_id,
                    .type = RequestType::CANCEL_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::ACCEPTED,
                    .error = {},
                    .text = {},
                    .version = {},
                    .request_id = request_id,
                };
                server::Trace event(trace_info, ack);
                callback(event, true, order.user_id);
              })) {
      } else {
        log::warn("*** EXTERNAL ORDER ***"_sv);
        log::warn("cancel_order={}"_sv, cancel_order);
      }
      break;
    }
    case json::OrderEventType::REJECT: {
      auto &order = order_event.order;
      auto symbol = std::string{order.symbol};
      std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
      auto side = json::map(order.side);
      auto order_type = json::map(order.type);
      auto status = compute_order_status(cancel_status.status);
      roq::OrderUpdate order_update{
          .stream_id = stream_id_,
          .account = account_,
          .order_id = order_id,
          .exchange = Flags::exchange(),
          .symbol = symbol,
          .side = side,
          .position_effect = {},
          .max_show_quantity = NaN,
          .order_type = order_type,
          .time_in_force = {},
          .execution_instruction = {},
          .order_template = {},
          .create_time_utc = {},
          .update_time_utc = cancel_status.received_time,
          .external_account = {},
          .external_order_id = cancel_status.order_id,
          .status = status,
          .quantity = order.quantity,
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
      auto request_id = cancel_status.cli_ord_id;
      if (shared_.find_order(
              stream_id_, trace_info, order_update, request_id, [&](auto &order, auto callback) {
                server::Ack ack{
                    .stream_id = stream_id_,
                    .account = account_,
                    .order_id = order.order_id,
                    .type = RequestType::CANCEL_ORDER,
                    .origin = Origin::EXCHANGE,
                    .status = RequestStatus::REJECTED,
                    .error = Error::UNKNOWN,
                    .text = order_event.reason,
                    .version = {},
                    .request_id = request_id,
                };
                server::Trace event(trace_info, ack);
                callback(event, true, order.user_id);
              })) {
      } else {
        log::warn("*** EXTERNAL ORDER ***"_sv);
        log::warn("cancel_order={}"_sv, cancel_order);
      }
      break;
    }
  }
}

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

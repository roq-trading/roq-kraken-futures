/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/order_update.h"

#include "roq/logging.h"

#include "roq/kraken_futures/flags.h"

#include "roq/kraken_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

namespace {
OrderStatus compute_order_status(json::Status status) {
  switch (status) {
    case json::Status::UNDEFINED:
    case json::Status::UNKNOWN:
      break;
    case json::Status::PLACED:
      return OrderStatus::WORKING;
    case json::Status::EDITED:
      break;
    case json::Status::CANCELLED:
      return OrderStatus::CANCELED;
  }
  return {};
}
}  // namespace

void OrderUpdate::operator()(
    const json::SendOrder &send_order, const server::TraceInfo &trace_info) {
  auto &send_status = send_order.send_status;
  auto status = compute_order_status(send_status.status);
  log::debug("status={}"_sv, status);
  roq::OrderUpdate order_update{
      .stream_id = stream_id_,
      .account = account_,
      .order_id = {},
      .exchange = Flags::exchange(),
      .symbol = {},
      .side = {},
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = {},
      .time_in_force = {},
      .execution_instruction = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = send_status.received_time,
      .external_account = {},
      .external_order_id = send_status.order_id,
      .status = status,
      .quantity = NaN,
      .price = NaN,
      .stop_price = NaN,
      .remaining_quantity = NaN,
      .traded_quantity = NaN,
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},  // XXX TODO(thraneh): decode clOrdID ?
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
  };
  auto request_id = send_status.cli_ord_id;
  if (shared_.find_order(
          stream_id_, trace_info, order_update, request_id, [&](const auto &order, auto callback) {
            server::Ack ack{
                .stream_id = stream_id_,
                .account = account_,
                .order_id = order.order_id,
                .type = {},
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
    log::warn("send_order={}"_sv, send_order);
  }
}

void OrderUpdate::operator()(
    const json::EditOrder &edit_order, const server::TraceInfo &trace_info) {
  auto &edit_status = edit_order.edit_status;
  auto status = compute_order_status(edit_status.status);
  log::debug("status={}"_sv, status);
  roq::OrderUpdate order_update{
      .stream_id = stream_id_,
      .account = account_,
      .order_id = {},
      .exchange = Flags::exchange(),
      .symbol = {},
      .side = {},
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = {},
      .time_in_force = {},
      .execution_instruction = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = edit_status.received_time,
      .external_account = {},
      .external_order_id = edit_status.order_id,
      .status = status,
      .quantity = NaN,
      .price = NaN,
      .stop_price = NaN,
      .remaining_quantity = NaN,
      .traded_quantity = NaN,
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},  // XXX TODO(thraneh): decode clOrdID ?
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
  };
  auto request_id = edit_status.cli_ord_id;
  if (shared_.find_order(
          stream_id_, trace_info, order_update, request_id, [&](const auto &order, auto callback) {
            server::Ack ack{
                .stream_id = stream_id_,
                .account = account_,
                .order_id = order.order_id,
                .type = {},
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
    log::warn("edit_order={}"_sv, edit_order);
  }
}

void OrderUpdate::operator()(
    const json::CancelOrder &cancel_order, const server::TraceInfo &trace_info) {
  auto &cancel_status = cancel_order.cancel_status;
  auto status = compute_order_status(cancel_status.status);
  log::debug("status={}"_sv, status);
  roq::OrderUpdate order_update{
      .stream_id = stream_id_,
      .account = account_,
      .order_id = {},
      .exchange = Flags::exchange(),
      .symbol = {},
      .side = {},
      .position_effect = {},
      .max_show_quantity = NaN,
      .order_type = {},
      .time_in_force = {},
      .execution_instruction = {},
      .order_template = {},
      .create_time_utc = {},
      .update_time_utc = cancel_status.received_time,
      .external_account = {},
      .external_order_id = cancel_status.order_id,
      .status = status,
      .quantity = NaN,
      .price = NaN,
      .stop_price = NaN,
      .remaining_quantity = NaN,
      .traded_quantity = NaN,
      .average_traded_price = NaN,
      .last_traded_quantity = NaN,
      .last_traded_price = NaN,
      .last_liquidity = {},
      .routing_id = {},  // XXX TODO(thraneh): decode clOrdID ?
      .max_request_version = {},
      .max_response_version = {},
      .max_accepted_version = {},
  };
  auto request_id = cancel_status.cli_ord_id;
  if (shared_.find_order(
          stream_id_, trace_info, order_update, request_id, [&](const auto &order, auto callback) {
            server::Ack ack{
                .stream_id = stream_id_,
                .account = account_,
                .order_id = order.order_id,
                .type = {},
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
}

}  // namespace kraken_futures
}  // namespace roq

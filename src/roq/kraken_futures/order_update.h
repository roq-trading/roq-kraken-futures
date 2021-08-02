/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <algorithm>
#include <string>
#include <string_view>

#include "roq/kraken_futures/shared.h"

#include "roq/kraken_futures/json/cancel_order.h"
#include "roq/kraken_futures/json/edit_order.h"
#include "roq/kraken_futures/json/open_orders.h"
#include "roq/kraken_futures/json/open_orders_snapshot.h"
#include "roq/kraken_futures/json/send_order.h"

// new

#include "roq/kraken_futures/json/utils.h"

namespace roq {
namespace kraken_futures {

class OrderUpdate final {
 public:
  explicit OrderUpdate(Shared &shared, uint16_t stream_id, const std::string_view &account)
      : shared_(shared), stream_id_(stream_id), account_(account) {}

  OrderUpdate(OrderUpdate &&) = delete;
  OrderUpdate(const OrderUpdate &) = delete;

  void operator()(const json::OpenOrdersSnapshot &, const server::TraceInfo &);
  void operator()(const json::OpenOrders &, const server::TraceInfo &);

  template <typename Accept, typename Reject>
  void operator()(
      const server::Order &order, const json::SendOrder &send_order, Accept accept, Reject reject) {
    auto &send_status = send_order.send_status;
    switch (send_status.status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
      case json::Status::EDITED:
      case json::Status::CANCELLED:
      case json::Status::NO_ORDERS_TO_CANCEL:
      case json::Status::NOT_FOUND:
        throw RuntimeErrorException("Unexpected: status={}"_sv, send_status.status);
        break;
      case json::Status::PLACED: {
        if (std::size(send_status.order_events) != 1)
          throw RuntimeErrorException(
              "Unexpected: size={}"_sv, std::size(send_status.order_events));
        auto &order_event = send_status.order_events[0];
        switch (order_event.type) {
          case json::OrderEventType::UNDEFINED:
          case json::OrderEventType::UNKNOWN:
          case json::OrderEventType::EDIT:
          case json::OrderEventType::CANCEL:
            throw RuntimeErrorException("Unexpected: type={}"_sv, order_event.type);
            break;
          case json::OrderEventType::PLACE: {
            auto &order_ = order_event.order;
            auto symbol = std::string{order_.symbol};
            std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
            auto side = json::map(order_.side);
            auto order_type = json::map(order_.type);
            auto status = compute_order_status(send_status.status);
            // XXX HANS should we use reduced_quantity to log a warning ???
            roq::OrderUpdate order_update{
                .stream_id = stream_id_,
                .account = account_,
                .order_id = order.order_id,
                .exchange = order.exchange,
                .symbol = symbol,
                .side = side,
                .position_effect = {},
                .max_show_quantity = NaN,
                .order_type = order_type,
                .time_in_force = {},
                .execution_instruction = {},
                .order_template = {},
                .create_time_utc = {},
                .update_time_utc = send_status.received_time,
                .external_account = {},
                .external_order_id = send_status.order_id,
                .status = status,
                .quantity = order_.quantity,
                .price = order_.limit_price,
                .stop_price = NaN,
                .remaining_quantity = NaN,
                .traded_quantity = order_.filled,
                .average_traded_price = NaN,
                .last_traded_quantity = NaN,
                .last_traded_price = NaN,
                .last_liquidity = {},
                .routing_id = {},
                .max_request_version = {},
                .max_response_version = {},
                .max_accepted_version = {},
            };
            accept(std::as_const(order_update));
            break;
          }
          case json::OrderEventType::REJECT: {
            reject(Error::UNKNOWN, order_event.reason);
            break;
          }
        }
        break;
      }
    }
  }

  template <typename Accept, typename Reject>
  void operator()(
      const server::Order &order, const json::EditOrder &edit_order, Accept accept, Reject reject) {
    auto &edit_status = edit_order.edit_status;
    switch (edit_status.status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
      case json::Status::PLACED:
      case json::Status::CANCELLED:
      case json::Status::NO_ORDERS_TO_CANCEL:
      case json::Status::NOT_FOUND:
        throw RuntimeErrorException("Unexpected: status={}"_sv, edit_status.status);
        break;
      case json::Status::EDITED: {
        if (std::size(edit_status.order_events) != 1)
          throw RuntimeErrorException(
              "Unexpected: size={}"_sv, std::size(edit_status.order_events));
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
                .order_id = order.order_id,
                .exchange = order.exchange,
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
            accept(std::as_const(order_update));
            break;
          }
          case json::OrderEventType::REJECT: {
            auto error = order_event.reason.compare("EDIT_HAS_NO_EFFECT"_sv) == 0
                             ? Error::MODIFY_HAS_NO_EFFECT
                             : Error::UNKNOWN;
            reject(error, order_event.reason);
            break;
          }
        }
        break;
      }
    }
  }

  template <typename Accept, typename Reject>
  void operator()(
      const server::Order &order,
      const json::CancelOrder &cancel_order,
      Accept accept,
      Reject reject) {
    auto &cancel_status = cancel_order.cancel_status;
    switch (cancel_status.status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
      case json::Status::PLACED:
      case json::Status::EDITED:
      case json::Status::NO_ORDERS_TO_CANCEL:
        throw RuntimeErrorException("Unexpected: status={}"_sv, cancel_status.status);
        break;
      case json::Status::CANCELLED: {
        if (std::size(cancel_status.order_events) != 1)
          throw RuntimeErrorException(
              "Unexpected: size={}"_sv, std::size(cancel_status.order_events));
        auto &order_event = cancel_status.order_events[0];
        switch (order_event.type) {
          case json::OrderEventType::UNDEFINED:
          case json::OrderEventType::UNKNOWN:
          case json::OrderEventType::PLACE:
          case json::OrderEventType::EDIT:
            throw RuntimeErrorException("Unexpected: type={}"_sv, order_event.type);
            break;
          case json::OrderEventType::CANCEL: {
            auto &order_ = order_event.new_;
            auto symbol = std::string{order_.symbol};
            std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
            auto side = json::map(order_.side);
            auto order_type = json::map(order_.type);
            auto status = compute_order_status(cancel_status.status);
            roq::OrderUpdate order_update{
                .stream_id = stream_id_,
                .account = account_,
                .order_id = order.order_id,
                .exchange = order.exchange,
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
                .quantity = order_.quantity,
                .price = order_.limit_price,
                .stop_price = NaN,
                .remaining_quantity = NaN,
                .traded_quantity = order_.filled,
                .average_traded_price = NaN,
                .last_traded_quantity = NaN,
                .last_traded_price = NaN,
                .last_liquidity = {},
                .routing_id = {},
                .max_request_version = {},
                .max_response_version = {},
                .max_accepted_version = {},
            };
            accept(std::as_const(order_update));
            break;
          }
          case json::OrderEventType::REJECT: {
            reject(Error::UNKNOWN, order_event.reason);
            break;
          }
        }
        break;
      }
      case json::Status::NOT_FOUND:
        reject(Error::TOO_LATE_TO_MODIFY_OR_CANCEL, ""_sv);
        break;
    }
  }

 protected:
  void operator()(const json::Order &, const server::TraceInfo &);

  Side compute_side(int32_t direction) {
    switch (direction) {
      case 0:
        return Side::BUY;
      case 1:
        return Side::SELL;
      default:
        return {};
    }
  }

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
      case json::Status::NO_ORDERS_TO_CANCEL:
      case json::Status::NOT_FOUND:
        break;
    }
    return {};
  }

 private:
  Shared &shared_;
  const uint16_t stream_id_;
  const std::string_view account_;
};

}  // namespace kraken_futures
}  // namespace roq

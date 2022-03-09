/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <algorithm>
#include <string>
#include <string_view>

#include "roq/logging.hpp"

#include "roq/utils/compare.hpp"

#include "roq/kraken_futures/shared.hpp"

#include "roq/kraken_futures/json/cancel_order.hpp"
#include "roq/kraken_futures/json/edit_order.hpp"
#include "roq/kraken_futures/json/open_orders.hpp"
#include "roq/kraken_futures/json/open_orders_snapshot.hpp"
#include "roq/kraken_futures/json/send_order.hpp"

// new

#include "roq/kraken_futures/json/utils.hpp"

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
      [[maybe_unused]] uint32_t order_id,
      const json::SendOrder &send_order,
      Accept accept,
      Reject reject) {
    using namespace std::literals;
    auto &send_status = send_order.send_status;
    switch (send_status.status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
      case json::Status::EDITED:
      case json::Status::FILLED:  // note! have only seen event type execution
      case json::Status::CANCELLED:
      case json::Status::NO_ORDERS_TO_CANCEL:
      case json::Status::NOT_FOUND:
        throw RuntimeError("Unexpected: status={}"sv, send_status.status);
        break;
      case json::Status::INVALID_ORDER_TYPE:
      case json::Status::INVALID_SIDE:
      case json::Status::INVALID_PRICE:
      case json::Status::INSUFFICIENT_AVAILABLE_FUNDS:
      case json::Status::SELF_FILL:
      case json::Status::TOO_MANY_SMALL_ORDERS:
      case json::Status::MAX_POSITION_VIOLATION:
      case json::Status::MARKET_SUSPENDED:
      case json::Status::MARKET_INACTIVE:
      case json::Status::CLIENT_ORDER_ID_ALREADY_EXIST:
      case json::Status::CLIENT_ORDER_ID_TOO_LONG:
      case json::Status::OUTSIDE_PRICE_COLLAR:
      case json::Status::POST_WOULD_EXECUTE:
      case json::Status::IOC_WOULD_NOT_EXECUTE:
      case json::Status::WOULD_CAUSE_LIQUIDATION:
      case json::Status::WOULD_NOT_REDUCE_POSITION:
        reject(Error::UNKNOWN, send_status.status.as_text());
        break;
      case json::Status::PLACED: {
        if (std::size(send_status.order_events) != 1)
          throw RuntimeError("Unexpected: size={}"sv, std::size(send_status.order_events));
        auto &order_event = send_status.order_events[0];
        switch (order_event.type) {
          case json::OrderEventType::UNDEFINED:
          case json::OrderEventType::UNKNOWN:
          case json::OrderEventType::EDIT:
          case json::OrderEventType::CANCEL:
            throw RuntimeError("Unexpected: type={}"sv, order_event.type);
            break;
          case json::OrderEventType::PLACE: {
            auto &order_ = order_event.order;
            auto symbol = std::string{order_.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto side = json::map(order_.side);
            auto order_type = json::map(order_.type);
            auto status = compute_order_status(send_status.status);
            // XXX HANS should we use reduced_quantity to log a warning ???
            oms::OrderUpdate order_update{
                .account = account_,
                .exchange = {},
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
            };
            accept(std::as_const(order_update));
            break;
          }
          case json::OrderEventType::EXECUTION: {
            auto &order_ = order_event.order_prior_execution;
            auto symbol = std::string{order_.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto side = json::map(order_.side);
            auto order_type = json::map(order_.type);
            auto status =
                compute_order_status(send_status.status, order_.quantity, order_event.amount);
            auto traded_quantity = order_event.amount;
            auto remaining_quantity = order_.quantity - traded_quantity;
            // XXX HANS should we use reduced_quantity to log a warning ???
            oms::OrderUpdate order_update{
                .account = account_,
                .exchange = {},
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
                .remaining_quantity = remaining_quantity,
                .traded_quantity = traded_quantity,
                .average_traded_price = NaN,
                .last_traded_quantity = NaN,
                .last_traded_price = NaN,
                .last_liquidity = {},
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
      [[maybe_unused]] uint32_t order_id,
      const json::EditOrder &edit_order,
      Accept accept,
      Reject reject) {
    using namespace std::literals;
    auto &edit_status = edit_order.edit_status;
    switch (edit_status.status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
      case json::Status::PLACED:
      case json::Status::FILLED:
      case json::Status::CANCELLED:
      case json::Status::NO_ORDERS_TO_CANCEL:
        throw RuntimeError("Unexpected: status={}"sv, edit_status.status);
        break;
      case json::Status::NOT_FOUND:
      case json::Status::INVALID_ORDER_TYPE:
      case json::Status::INVALID_SIDE:
      case json::Status::INVALID_PRICE:
      case json::Status::INSUFFICIENT_AVAILABLE_FUNDS:
      case json::Status::SELF_FILL:
      case json::Status::TOO_MANY_SMALL_ORDERS:
      case json::Status::MAX_POSITION_VIOLATION:
      case json::Status::MARKET_SUSPENDED:
      case json::Status::MARKET_INACTIVE:
      case json::Status::CLIENT_ORDER_ID_ALREADY_EXIST:
      case json::Status::CLIENT_ORDER_ID_TOO_LONG:
      case json::Status::OUTSIDE_PRICE_COLLAR:
      case json::Status::POST_WOULD_EXECUTE:
      case json::Status::IOC_WOULD_NOT_EXECUTE:
      case json::Status::WOULD_CAUSE_LIQUIDATION:
      case json::Status::WOULD_NOT_REDUCE_POSITION:
        reject(Error::UNKNOWN, edit_status.status.as_text());
        break;
      case json::Status::EDITED: {
        if (std::size(edit_status.order_events) != 1)
          throw RuntimeError("Unexpected: size={}"sv, std::size(edit_status.order_events));
        auto &order_event = edit_status.order_events[0];
        switch (order_event.type) {
          case json::OrderEventType::UNDEFINED:
          case json::OrderEventType::UNKNOWN:
          case json::OrderEventType::PLACE:
          case json::OrderEventType::CANCEL:
            throw RuntimeError("Unexpected: type={}"sv, order_event.type);
            break;
          case json::OrderEventType::EDIT: {
            auto &new_order = order_event.new_;
            auto symbol = std::string{new_order.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto side = json::map(new_order.side);
            auto order_type = json::map(new_order.type);
            auto status = compute_order_status(edit_status.status);
            // XXX HANS should we use reduced_quantity to compute remaining quantity ???
            oms::OrderUpdate order_update{
                .account = account_,
                .exchange = {},
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
            };
            accept(std::as_const(order_update));
            break;
          }
          case json::OrderEventType::EXECUTION: {
            // XXX HANS FIX THIS
            log::warn("order_event={}"sv, order_event);
            log::fatal("Unexpected"sv);
            break;
          }
          case json::OrderEventType::REJECT: {
            auto error = order_event.reason.compare("EDIT_HAS_NO_EFFECT"sv) == 0
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
      [[maybe_unused]] uint32_t order_id,
      const json::CancelOrder &cancel_order,
      Accept accept,
      Reject reject) {
    using namespace std::literals;
    auto &cancel_status = cancel_order.cancel_status;
    switch (cancel_status.status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
      case json::Status::PLACED:
      case json::Status::EDITED:
      case json::Status::FILLED:
        throw RuntimeError("Unexpected: status={}"sv, cancel_status.status);
        break;
      case json::Status::NO_ORDERS_TO_CANCEL:
      case json::Status::INVALID_ORDER_TYPE:
      case json::Status::INVALID_SIDE:
      case json::Status::INVALID_PRICE:
      case json::Status::INSUFFICIENT_AVAILABLE_FUNDS:
      case json::Status::SELF_FILL:
      case json::Status::TOO_MANY_SMALL_ORDERS:
      case json::Status::MAX_POSITION_VIOLATION:
      case json::Status::MARKET_SUSPENDED:
      case json::Status::MARKET_INACTIVE:
      case json::Status::CLIENT_ORDER_ID_ALREADY_EXIST:
      case json::Status::CLIENT_ORDER_ID_TOO_LONG:
      case json::Status::OUTSIDE_PRICE_COLLAR:
      case json::Status::POST_WOULD_EXECUTE:
      case json::Status::IOC_WOULD_NOT_EXECUTE:
      case json::Status::WOULD_CAUSE_LIQUIDATION:
      case json::Status::WOULD_NOT_REDUCE_POSITION:
        reject(Error::UNKNOWN, cancel_status.status.as_text());
        break;
      case json::Status::CANCELLED: {
        if (std::size(cancel_status.order_events) != 1)
          throw RuntimeError("Unexpected: size={}"sv, std::size(cancel_status.order_events));
        auto &order_event = cancel_status.order_events[0];
        switch (order_event.type) {
          case json::OrderEventType::UNDEFINED:
          case json::OrderEventType::UNKNOWN:
          case json::OrderEventType::PLACE:
          case json::OrderEventType::EDIT:
            throw RuntimeError("Unexpected: type={}"sv, order_event.type);
            break;
          case json::OrderEventType::CANCEL: {
            auto &order_ = order_event.new_;
            auto symbol = std::string{order_.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto side = json::map(order_.side);
            auto order_type = json::map(order_.type);
            auto status = compute_order_status(cancel_status.status);
            oms::OrderUpdate order_update{
                .account = account_,
                .exchange = {},
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
            };
            accept(std::as_const(order_update));
            break;
          }
          case json::OrderEventType::EXECUTION: {
            // XXX HANS FIX THIS
            log::warn("order_event={}"sv, order_event);
            log::fatal("Unexpected"sv);
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
        reject(Error::TOO_LATE_TO_MODIFY_OR_CANCEL, ""sv);
        break;
    }
  }

 protected:
  void operator()(
      const json::Order &,
      const std::string_view &order_id,
      const std::string_view &cli_ord_id,
      const json::Reason,
      bool is_cancel,
      const server::TraceInfo &,
      bool is_download);

  Side compute_side(int32_t direction) {
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

  OrderStatus compute_order_status(json::Status status) {
    switch (status) {
      case json::Status::UNDEFINED:
      case json::Status::UNKNOWN:
        break;
      case json::Status::PLACED:
        return OrderStatus::WORKING;
      case json::Status::EDITED:
        return OrderStatus::WORKING;
      case json::Status::FILLED:
        return OrderStatus::COMPLETED;
      case json::Status::CANCELLED:
        return OrderStatus::CANCELED;
      case json::Status::NO_ORDERS_TO_CANCEL:
      case json::Status::NOT_FOUND:
        break;
      case json::Status::INVALID_ORDER_TYPE:
      case json::Status::INVALID_SIDE:
      case json::Status::INVALID_PRICE:
      case json::Status::INSUFFICIENT_AVAILABLE_FUNDS:
      case json::Status::SELF_FILL:
      case json::Status::TOO_MANY_SMALL_ORDERS:
      case json::Status::MAX_POSITION_VIOLATION:
      case json::Status::MARKET_SUSPENDED:
      case json::Status::MARKET_INACTIVE:
      case json::Status::CLIENT_ORDER_ID_ALREADY_EXIST:
      case json::Status::CLIENT_ORDER_ID_TOO_LONG:
      case json::Status::OUTSIDE_PRICE_COLLAR:
      case json::Status::POST_WOULD_EXECUTE:
      case json::Status::IOC_WOULD_NOT_EXECUTE:
      case json::Status::WOULD_CAUSE_LIQUIDATION:
      case json::Status::WOULD_NOT_REDUCE_POSITION:
        return OrderStatus::REJECTED;
    }
    return {};
  }

  OrderStatus compute_order_status(json::Status status, double quantity, double filled) {
    if (utils::compare(quantity, filled) == 0)
      return OrderStatus::COMPLETED;
    return compute_order_status(status);
  }

 private:
  Shared &shared_;
  const uint16_t stream_id_;
  const std::string_view account_;
};

}  // namespace kraken_futures
}  // namespace roq

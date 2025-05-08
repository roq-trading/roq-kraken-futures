/* Copyright (c) 2017-2025, Hans Erik Thrane */

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

#include "roq/kraken_futures/json/map.hpp"

// new

#include "roq/kraken_futures/json/utils.hpp"

namespace roq {
namespace kraken_futures {

struct OrderUpdate final {
  explicit OrderUpdate(Shared &shared, uint16_t stream_id, std::string_view const &account) : shared_(shared), stream_id_(stream_id), account_(account) {}

  OrderUpdate(OrderUpdate const &) = delete;

  void operator()(json::OpenOrdersSnapshot const &, TraceInfo const &);
  void operator()(json::OpenOrders const &, TraceInfo const &);

  template <typename Accept, typename Reject>
  void operator()([[maybe_unused]] uint64_t order_id, json::SendOrder const &send_order, Accept accept, Reject reject) {
    using namespace std::literals;
    auto &send_status = send_order.send_status;
    switch (send_status.status) {
      using enum json::Status::type_t;
      case UNDEFINED__:
      case UNKNOWN__:
      case EDITED:
      case FILLED:  // note! have only seen event type execution
      case CANCELLED:
      case NO_ORDERS_TO_CANCEL:
      case NOT_FOUND:
        throw RuntimeError{"Unexpected: status={}"sv, send_status.status};
        break;
      case INVALID_ORDER_TYPE:
      case INVALID_SIDE:
      case INVALID_PRICE:
      case INSUFFICIENT_AVAILABLE_FUNDS:
      case SELF_FILL:
      case TOO_MANY_SMALL_ORDERS:
      case MAX_POSITION_VIOLATION:
      case MARKET_SUSPENDED:
      case MARKET_INACTIVE:
      case CLIENT_ORDER_ID_ALREADY_EXIST:
      case CLIENT_ORDER_ID_TOO_LONG:
      case OUTSIDE_PRICE_COLLAR:
      case POST_WOULD_EXECUTE:
      case IOC_WOULD_NOT_EXECUTE:
      case WOULD_CAUSE_LIQUIDATION:
      case WOULD_NOT_REDUCE_POSITION:
        reject(Error::UNKNOWN, send_status.status.as_raw_text());
        break;
      case PLACED: {
        if (std::size(send_status.order_events) != 1) {
          throw RuntimeError{"Unexpected: size={}"sv, std::size(send_status.order_events)};
        }
        auto &order_event = send_status.order_events[0];
        switch (order_event.type) {
          using enum json::OrderEventType::type_t;
          case UNDEFINED__:
          case UNKNOWN__:
          case EDIT:
          case CANCEL:
            throw RuntimeError{"Unexpected: type={}"sv, order_event.type};
            break;
          case PLACE: {
            auto &order_ = order_event.order;
            auto symbol = std::string{order_.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto status = compute_order_status(send_status.status);
            // XXX HANS should we use reduced_quantity to log a warning ???
            auto order_update = server::oms::OrderUpdate{
                .account = account_,
                .exchange = {},
                .symbol = symbol,
                .side = map(order_.side),
                .position_effect = {},
                .margin_mode = {},
                .max_show_quantity = NaN,
                .order_type = map(order_.type),
                .time_in_force = {},
                .execution_instructions = {},
                .create_time_utc = {},
                .update_time_utc = send_status.received_time,
                .external_account = {},
                .external_order_id = send_status.order_id,
                .client_order_id = {},
                .order_status = status,
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
                .update_type = {},
                .sending_time_utc = {},
            };
            accept(std::as_const(order_update));
            break;
          }
          case EXECUTION: {
            auto &order_ = order_event.order_prior_execution;
            auto symbol = std::string{order_.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto status = compute_order_status(send_status.status, order_.quantity, order_event.amount);
            auto traded_quantity = order_event.amount;
            auto remaining_quantity = order_.quantity - traded_quantity;
            // XXX HANS should we use reduced_quantity to log a warning ???
            auto order_update = server::oms::OrderUpdate{
                .account = account_,
                .exchange = {},
                .symbol = symbol,
                .side = map(order_.side),
                .position_effect = {},
                .margin_mode = {},
                .max_show_quantity = NaN,
                .order_type = map(order_.type),
                .time_in_force = {},
                .execution_instructions = {},
                .create_time_utc = {},
                .update_time_utc = send_status.received_time,
                .external_account = {},
                .external_order_id = send_status.order_id,
                .client_order_id = {},
                .order_status = status,
                .quantity = order_.quantity,
                .price = order_.limit_price,
                .stop_price = NaN,
                .remaining_quantity = remaining_quantity,
                .traded_quantity = traded_quantity,
                .average_traded_price = NaN,
                .last_traded_quantity = NaN,
                .last_traded_price = NaN,
                .last_liquidity = {},
                .routing_id = {},
                .max_request_version = {},
                .max_response_version = {},
                .max_accepted_version = {},
                .update_type = {},
                .sending_time_utc = {},
            };
            accept(std::as_const(order_update));
            break;
          }
          case REJECT: {
            reject(Error::UNKNOWN, order_event.reason);
            break;
          }
        }
        break;
      }
    }
  }

  template <typename Accept, typename Reject>
  void operator()([[maybe_unused]] uint64_t order_id, json::EditOrder const &edit_order, Accept accept, Reject reject) {
    using namespace std::literals;
    auto &edit_status = edit_order.edit_status;
    switch (edit_status.status) {
      using enum json::Status::type_t;
      case UNDEFINED__:
      case UNKNOWN__:
      case PLACED:
      case FILLED:
      case CANCELLED:
      case NO_ORDERS_TO_CANCEL:
        throw RuntimeError{"Unexpected: status={}"sv, edit_status.status};
        break;
      case NOT_FOUND:
      case INVALID_ORDER_TYPE:
      case INVALID_SIDE:
      case INVALID_PRICE:
      case INSUFFICIENT_AVAILABLE_FUNDS:
      case SELF_FILL:
      case TOO_MANY_SMALL_ORDERS:
      case MAX_POSITION_VIOLATION:
      case MARKET_SUSPENDED:
      case MARKET_INACTIVE:
      case CLIENT_ORDER_ID_ALREADY_EXIST:
      case CLIENT_ORDER_ID_TOO_LONG:
      case OUTSIDE_PRICE_COLLAR:
      case POST_WOULD_EXECUTE:
      case IOC_WOULD_NOT_EXECUTE:
      case WOULD_CAUSE_LIQUIDATION:
      case WOULD_NOT_REDUCE_POSITION:
        reject(Error::UNKNOWN, edit_status.status.as_raw_text());
        break;
      case EDITED: {
        if (std::size(edit_status.order_events) != 1) {
          throw RuntimeError{"Unexpected: size={}"sv, std::size(edit_status.order_events)};
        }
        auto &order_event = edit_status.order_events[0];
        switch (order_event.type) {
          using enum json::OrderEventType::type_t;
          case UNDEFINED__:
          case UNKNOWN__:
          case PLACE:
          case CANCEL:
            throw RuntimeError{"Unexpected: type={}"sv, order_event.type};
            break;
          case EDIT: {
            auto &new_order = order_event.new_;
            auto symbol = std::string{new_order.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto status = compute_order_status(edit_status.status);
            // XXX HANS should we use reduced_quantity to compute remaining quantity ???
            auto order_update = server::oms::OrderUpdate{
                .account = account_,
                .exchange = {},
                .symbol = symbol,
                .side = map(new_order.side),
                .position_effect = {},
                .margin_mode = {},
                .max_show_quantity = NaN,
                .order_type = map(new_order.type),
                .time_in_force = {},
                .execution_instructions = {},
                .create_time_utc = {},
                .update_time_utc = edit_status.received_time,
                .external_account = {},
                .external_order_id = edit_status.order_id,
                .client_order_id = {},
                .order_status = status,
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
                .update_type = {},
                .sending_time_utc = {},
            };
            accept(std::as_const(order_update));
            break;
          }
          case EXECUTION: {
            // XXX HANS FIX THIS
            log::warn("order_event={}"sv, order_event);
            log::fatal("Unexpected"sv);
            break;
          }
          case REJECT: {
            auto error = order_event.reason.compare("EDIT_HAS_NO_EFFECT"sv) == 0 ? Error::MODIFY_HAS_NO_EFFECT : Error::UNKNOWN;
            reject(error, order_event.reason);
            break;
          }
        }
        break;
      }
    }
  }

  template <typename Accept, typename Reject>
  void operator()([[maybe_unused]] uint64_t order_id, json::CancelOrder const &cancel_order, Accept accept, Reject reject) {
    using namespace std::literals;
    auto &cancel_status = cancel_order.cancel_status;
    switch (cancel_status.status) {
      using enum json::Status::type_t;
      case UNDEFINED__:
      case UNKNOWN__:
      case PLACED:
      case EDITED:
      case FILLED:
        throw RuntimeError{"Unexpected: status={}"sv, cancel_status.status};
        break;
      case NO_ORDERS_TO_CANCEL:
      case INVALID_ORDER_TYPE:
      case INVALID_SIDE:
      case INVALID_PRICE:
      case INSUFFICIENT_AVAILABLE_FUNDS:
      case SELF_FILL:
      case TOO_MANY_SMALL_ORDERS:
      case MAX_POSITION_VIOLATION:
      case MARKET_SUSPENDED:
      case MARKET_INACTIVE:
      case CLIENT_ORDER_ID_ALREADY_EXIST:
      case CLIENT_ORDER_ID_TOO_LONG:
      case OUTSIDE_PRICE_COLLAR:
      case POST_WOULD_EXECUTE:
      case IOC_WOULD_NOT_EXECUTE:
      case WOULD_CAUSE_LIQUIDATION:
      case WOULD_NOT_REDUCE_POSITION:
        reject(Error::UNKNOWN, cancel_status.status.as_raw_text());
        break;
      case CANCELLED: {
        if (std::size(cancel_status.order_events) != 1) {
          throw RuntimeError{"Unexpected: size={}"sv, std::size(cancel_status.order_events)};
        }
        auto &order_event = cancel_status.order_events[0];
        switch (order_event.type) {
          using enum json::OrderEventType::type_t;
          case UNDEFINED__:
          case UNKNOWN__:
          case PLACE:
          case EDIT:
            throw RuntimeError{"Unexpected: type={}"sv, order_event.type};
            break;
          case CANCEL: {
            auto &order_ = order_event.new_;
            auto symbol = std::string{order_.symbol};
            std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
            auto status = compute_order_status(cancel_status.status);
            auto order_update = server::oms::OrderUpdate{
                .account = account_,
                .exchange = {},
                .symbol = symbol,
                .side = map(order_.side),
                .position_effect = {},
                .margin_mode = {},
                .max_show_quantity = NaN,
                .order_type = map(order_.type),
                .time_in_force = {},
                .execution_instructions = {},
                .create_time_utc = {},
                .update_time_utc = cancel_status.received_time,
                .external_account = {},
                .external_order_id = cancel_status.order_id,
                .client_order_id = {},
                .order_status = status,
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
                .update_type = {},
                .sending_time_utc = {},
            };
            accept(std::as_const(order_update));
            break;
          }
          case EXECUTION: {
            // XXX HANS FIX THIS
            log::warn("order_event={}"sv, order_event);
            log::fatal("Unexpected"sv);
            break;
          }
          case REJECT: {
            reject(Error::UNKNOWN, order_event.reason);
            break;
          }
        }
        break;
      }
      case NOT_FOUND:
        reject(Error::TOO_LATE_TO_MODIFY_OR_CANCEL, ""sv);
        break;
    }
  }

 protected:
  void operator()(
      json::Order const &,
      std::string_view const &order_id,
      std::string_view const &cli_ord_id,
      json::Reason const,
      bool is_cancel,
      TraceInfo const &,
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
      using enum json::Status::type_t;
      case UNDEFINED__:
      case UNKNOWN__:
        break;
      case PLACED:
        return OrderStatus::WORKING;
      case EDITED:
        return OrderStatus::WORKING;
      case FILLED:
        return OrderStatus::COMPLETED;
      case CANCELLED:
        return OrderStatus::CANCELED;
      case NO_ORDERS_TO_CANCEL:
      case NOT_FOUND:
        break;
      case INVALID_ORDER_TYPE:
      case INVALID_SIDE:
      case INVALID_PRICE:
      case INSUFFICIENT_AVAILABLE_FUNDS:
      case SELF_FILL:
      case TOO_MANY_SMALL_ORDERS:
      case MAX_POSITION_VIOLATION:
      case MARKET_SUSPENDED:
      case MARKET_INACTIVE:
      case CLIENT_ORDER_ID_ALREADY_EXIST:
      case CLIENT_ORDER_ID_TOO_LONG:
      case OUTSIDE_PRICE_COLLAR:
      case POST_WOULD_EXECUTE:
      case IOC_WOULD_NOT_EXECUTE:
      case WOULD_CAUSE_LIQUIDATION:
      case WOULD_NOT_REDUCE_POSITION:
        return OrderStatus::REJECTED;
    }
    return {};
  }

  OrderStatus compute_order_status(json::Status status, double quantity, double filled) {
    if (utils::is_equal(quantity, filled)) {
      return OrderStatus::COMPLETED;
    }
    return compute_order_status(status);
  }

 private:
  Shared &shared_;
  uint16_t const stream_id_;
  std::string_view const account_;
};

}  // namespace kraken_futures
}  // namespace roq

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
      case UNDEFINED_INTERNAL:
      case UNKNOWN_INTERNAL:
      case EDITED:
      case NO_ORDERS_TO_CANCEL:  // ???
      case NOT_FOUND:            // ???
      case ORDER_FOR_EDIT_NOT_FOUND:
        reject(Error::UNKNOWN, send_status.status.as_raw_text());
        break;
      case CANCELLED:  // XXX FIXME TODO possible ???
        reject(Error::UNKNOWN, send_status.status.as_raw_text());
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
      case WOULD_NOT_REDUCE_POSITION: {
        std::string_view const text{send_status.status.as_raw_text()};
        reject(Error::UNKNOWN, text);
        break;
      }
      case PLACED:
        process_order(send_status, json::OrderEventType::PLACE, accept);
        break;
      case FILLED:
        process_execution(send_status, accept);
        break;
    }
  }

  template <typename Accept, typename Reject>
  void operator()([[maybe_unused]] uint64_t order_id, json::EditOrder const &edit_order, Accept accept, Reject reject) {
    using namespace std::literals;
    auto &edit_status = edit_order.edit_status;
    switch (edit_status.status) {
      using enum json::Status::type_t;
      case UNDEFINED_INTERNAL:
      case UNKNOWN_INTERNAL:
      case PLACED:
      case NO_ORDERS_TO_CANCEL:
        throw RuntimeError{"Unexpected: status={}"sv, edit_status.status.as_raw_text()};
        break;
      case CANCELLED:  // XXX FIXME TODO possible ???
        throw RuntimeError{"Unexpected: status={}"sv, edit_status.status.as_raw_text()};
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
      case ORDER_FOR_EDIT_NOT_FOUND: {
        std::string_view const text{edit_status.status.as_raw_text()};
        reject(Error::UNKNOWN, text);
        break;
      }
      case EDITED:
        process_order(edit_status, json::OrderEventType::EDIT, accept);
        break;
      case FILLED:
        process_execution(edit_status, accept);
        break;
    }
  }

  template <typename Accept, typename Reject>
  void operator()([[maybe_unused]] uint64_t order_id, json::CancelOrder const &cancel_order, Accept accept, Reject reject) {
    using namespace std::literals;
    auto &cancel_status = cancel_order.cancel_status;
    switch (cancel_status.status) {
      using enum json::Status::type_t;
      case UNDEFINED_INTERNAL:
      case UNKNOWN_INTERNAL:
      case PLACED:
      case EDITED:
      case ORDER_FOR_EDIT_NOT_FOUND:
        reject(Error::UNKNOWN, cancel_status.status.as_raw_text());
        break;
      case NO_ORDERS_TO_CANCEL:  // XXX FIXME TODO what does this really mean ???
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
      case NOT_FOUND: {
        std::string_view const text{cancel_status.status.as_raw_text()};
        reject(Error::UNKNOWN, text);
        break;
      }
      case CANCELLED:
        process_order(cancel_status, json::OrderEventType::CANCEL, accept);
        break;
      case FILLED:  // XXX FIXME TODO is this actually possible ???
        process_execution(cancel_status, accept);
        break;
    }
  }

 protected:
  void operator()(
      json::Order const &,
      std::string_view const &order_id,
      std::string_view const &cli_ord_id,
      json::Reason,
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
      case UNDEFINED_INTERNAL:
      case UNKNOWN_INTERNAL:
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
      case ORDER_FOR_EDIT_NOT_FOUND:
        return OrderStatus::REJECTED;
    }
    return {};
  }

  OrderStatus compute_order_status(json::Status status, double remaining_quantity) {
    if (utils::is_zero(remaining_quantity)) {
      return OrderStatus::COMPLETED;
    }
    return compute_order_status(status);
  }

  template <typename Callback>
  void process_order(auto &request_status, auto expected_event_type, Callback callback) {
    using namespace std::literals;
    if (std::size(request_status.order_events) != 1) {
      throw RuntimeError{"Unexpected: size={}"sv, std::size(request_status.order_events)};
    }
    auto &order_event = request_status.order_events[0];
    if (order_event.type == json::OrderEventType::EXECUTION) {
      process_execution(request_status, callback);
      return;  // note!
    }
    // XXX FIXME TODO can REJECT also happen ???
    if (order_event.type != expected_event_type) {
      throw RuntimeError{"Unexpected: type={} (expected type={})"sv, order_event.type, expected_event_type};
    }
    auto &order_ = order_event.order;
    auto symbol = std::string{order_.symbol};
    std::ranges::transform(symbol, std::begin(symbol), ::toupper);
    auto remaining_quantity = order_.quantity;
    auto traded_quantity = order_.filled;
    auto quantity = remaining_quantity + traded_quantity;
    auto order_status = compute_order_status(request_status.status, remaining_quantity);
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
        .update_time_utc = request_status.received_time,
        .external_account = {},
        .external_order_id = request_status.order_id,
        .client_order_id = {},
        .order_status = order_status,
        .quantity = quantity,         // note!
        .price = order_.limit_price,  // note!
        .stop_price = NaN,
        .remaining_quantity = remaining_quantity,  // note!
        .traded_quantity = traded_quantity,        // note!
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
    callback(std::as_const(order_update));
  }

  template <typename Callback>
  void process_execution(auto &request_status, Callback callback) {
    using namespace std::literals;
    if (std::empty(request_status.order_events)) {
      throw RuntimeError{"Unexpected: size={}"sv, std::size(request_status.order_events)};
    }
    std::string symbol;
    OrderType order_type = {};
    Side side = {};
    double price = NaN;
    double prior_quantity = NaN;
    double prior_filled = NaN;
    double last_traded_quantity = 0.0;
    double sum_product = 0.0;
    for (auto &order_event : request_status.order_events) {
      if (order_event.type != json::OrderEventType::EXECUTION) {
        throw RuntimeError{"Unexpected: type={}"sv, order_event.type};
      }
      auto &order_prior_execution = order_event.order_prior_execution;
      if (std::empty(symbol)) {
        symbol = std::string{order_prior_execution.symbol};
        std::ranges::transform(symbol, std::begin(symbol), ::toupper);
      }
      order_type = map(order_prior_execution.type);
      side = map(order_prior_execution.side);
      price = order_prior_execution.limit_price;
      prior_quantity = order_prior_execution.quantity;
      prior_filled = order_prior_execution.filled;
      last_traded_quantity += order_event.amount;
      sum_product += order_event.amount * order_event.price;
    }
    auto last_traded_price = utils::is_zero(last_traded_quantity) ? NaN : (sum_product / last_traded_quantity);
    auto remaining_quantity = prior_quantity - last_traded_quantity;  // ???
    auto traded_quantity = prior_filled + last_traded_quantity;
    auto order_status = compute_order_status(request_status.status, remaining_quantity);
    auto order_update = server::oms::OrderUpdate{
        .account = account_,
        .exchange = {},
        .symbol = symbol,
        .side = side,
        .position_effect = {},
        .margin_mode = {},
        .max_show_quantity = NaN,
        .order_type = order_type,
        .time_in_force = {},
        .execution_instructions = {},
        .create_time_utc = {},
        .update_time_utc = request_status.received_time,
        .external_account = {},
        .external_order_id = request_status.order_id,
        .client_order_id = {},
        .order_status = order_status,
        .quantity = NaN,  // note!
        .price = price,   // note!
        .stop_price = NaN,
        .remaining_quantity = remaining_quantity,  // note!
        .traded_quantity = traded_quantity,        // note!
        .average_traded_price = NaN,
        .last_traded_quantity = last_traded_quantity,  // note!
        .last_traded_price = last_traded_price,        // note!
        .last_liquidity = Liquidity::TAKER,
        .routing_id = {},
        .max_request_version = {},
        .max_response_version = {},
        .max_accepted_version = {},
        .update_type = {},
        .sending_time_utc = {},
    };
    callback(std::as_const(order_update));  // XXX FIXME TODO include the fills
  }

 private:
  Shared &shared_;
  uint16_t const stream_id_;
  std::string_view const account_;
};

}  // namespace kraken_futures
}  // namespace roq

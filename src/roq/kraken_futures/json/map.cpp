/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/json/map.hpp"

using namespace std::literals;

namespace roq {

namespace {
template <typename... Args>
using Helper = detail::MapHelper<Args...>;
}

// kraken_futures::json => roq

// kraken_futures::json::FillType => roq::OrderType

template <>
template <>
constexpr Helper<kraken_futures::json::FillType>::operator std::optional<roq::Liquidity>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::FillType::type_t;
    case UNDEFINED_INTERNAL:
      return Liquidity::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return Liquidity::UNDEFINED;
    case MAKER:
      return Liquidity::MAKER;
    case TAKER:
      return Liquidity::TAKER;
    case LIQUIDATION:
      return Liquidity::UNDEFINED;
    case ASSIGNEE:
      return Liquidity::UNDEFINED;
    case ASSIGNOR:
      return Liquidity::UNDEFINED;
    case UNWIND_BANKRUPT:
      return Liquidity::UNDEFINED;
    case UNWIND_COUNTERPARTY:
      return Liquidity::UNDEFINED;
    case TAKER_AFTER_EDIT:
      return Liquidity::UNDEFINED;
  }
  return {};
}

static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::UNDEFINED_INTERNAL}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::MAKER}} == roq::Liquidity::MAKER);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::TAKER}} == roq::Liquidity::TAKER);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::LIQUIDATION}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::ASSIGNEE}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::ASSIGNOR}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::UNWIND_BANKRUPT}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::UNWIND_COUNTERPARTY}} == roq::Liquidity::UNDEFINED);
static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::TAKER_AFTER_EDIT}} == roq::Liquidity::UNDEFINED);

template <>
template <>
std::optional<roq::Liquidity> Map<kraken_futures::json::FillType>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::OrderEventOrderType => roq::OrderType

template <>
template <>
constexpr Helper<kraken_futures::json::OrderEventOrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::OrderEventOrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case LMT:
      return roq::OrderType::LIMIT;
    case MKT:
      return roq::OrderType::MARKET;
    case STP:
      return roq::OrderType::UNDEFINED;
    case TAKE_PROFIT:
      return roq::OrderType::UNDEFINED;
    case IOC:
      return roq::OrderType::LIMIT;
  }
  return {};
}

static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::LMT}} == roq::OrderType::LIMIT);
static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::MKT}} == roq::OrderType::MARKET);
static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::STP}} == roq::OrderType::UNDEFINED);
static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::TAKE_PROFIT}} == roq::OrderType::UNDEFINED);
static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::IOC}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<kraken_futures::json::OrderEventOrderType>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::OrderType => roq::OrderType

template <>
template <>
constexpr Helper<kraken_futures::json::OrderType>::operator std::optional<roq::OrderType>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::OrderType::type_t;
    case UNDEFINED_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::OrderType::UNDEFINED;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      return roq::OrderType::LIMIT;  // XXX could also be market ???
  }
  return {};
}

static_assert(Helper{kraken_futures::json::OrderType{kraken_futures::json::OrderType::UNDEFINED_INTERNAL}} == roq::OrderType::UNDEFINED);
static_assert(Helper{kraken_futures::json::OrderType{kraken_futures::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{kraken_futures::json::OrderType{kraken_futures::json::OrderType::STOP}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<kraken_futures::json::OrderType>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::Reason => OrderStatus

template <>
template <>
constexpr Helper<kraken_futures::json::Reason>::operator std::optional<OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::Reason::type_t;
    case UNDEFINED_INTERNAL:
      return OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return OrderStatus::UNDEFINED;
    case NEW_PLACED_ORDER_BY_USER:
      return OrderStatus::WORKING;
    case LIQUIDATION:
      return OrderStatus::UNDEFINED;
    case STOP_ORDER_TRIGGERED:
      return OrderStatus::UNDEFINED;
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
      return OrderStatus::UNDEFINED;
    case CANCELLED_BY_ADMIN:
      return OrderStatus::CANCELED;
    case DEAD_MAN_SWITCH:
      return OrderStatus::CANCELED;
    case IOC_ORDER_FAILED_BECAUSE_IT_WOULD_NOT:
      return OrderStatus::CANCELED;
    case POST_ORDER_FAILED_BECAUSE_IT_WOULD_FILLED:
      return OrderStatus::CANCELED;
    case WOULD_EXECUTE_SELF:
      return OrderStatus::CANCELED;
    case WOULD_NOT_REDUCE_POSITION:
      return OrderStatus::CANCELED;
    case ORDER_FOR_EDIT_NOT_FOUND:
      return OrderStatus::UNDEFINED;
    case EDITED_BY_USER:
      return OrderStatus::UNDEFINED;  // XXX FIXME TODO working?
  }
  return {};
}

static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::UNDEFINED_INTERNAL}} == OrderStatus::UNDEFINED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::NEW_PLACED_ORDER_BY_USER}} == OrderStatus::WORKING);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::LIQUIDATION}} == OrderStatus::UNDEFINED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::STOP_ORDER_TRIGGERED}} == OrderStatus::UNDEFINED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::LIMIT_ORDER_FROM_STOP}} == OrderStatus::WORKING);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::PARTIAL_FILL}} == OrderStatus::WORKING);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::FULL_FILL}} == OrderStatus::COMPLETED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::CANCELLED_BY_USER}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::CONTRACT_EXPIRED}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::NOT_ENOUGH_MARGIN}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::MARKET_INACTIVE}} == OrderStatus::UNDEFINED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::CANCELLED_BY_ADMIN}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::DEAD_MAN_SWITCH}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::IOC_ORDER_FAILED_BECAUSE_IT_WOULD_NOT}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::POST_ORDER_FAILED_BECAUSE_IT_WOULD_FILLED}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::WOULD_EXECUTE_SELF}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::WOULD_NOT_REDUCE_POSITION}} == OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::ORDER_FOR_EDIT_NOT_FOUND}} == OrderStatus::UNDEFINED);
static_assert(Helper{kraken_futures::json::Reason{kraken_futures::json::Reason::ORDER_FOR_EDIT_NOT_FOUND}} == OrderStatus::UNDEFINED);

template <>
template <>
std::optional<OrderStatus> Map<kraken_futures::json::Reason>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::Side => roq::Side

template <>
template <>
constexpr Helper<kraken_futures::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::Side::type_t;
    case UNDEFINED_INTERNAL:
      return roq::Side::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{kraken_futures::json::Side{kraken_futures::json::Side::UNDEFINED_INTERNAL}} == roq::Side::UNDEFINED);
static_assert(Helper{kraken_futures::json::Side{kraken_futures::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{kraken_futures::json::Side{kraken_futures::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<kraken_futures::json::Side>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::Status => roq::Error

template <>
template <>
constexpr Helper<kraken_futures::json::Status>::operator std::optional<roq::Error>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::Status::type_t;
    case UNDEFINED_INTERNAL:
      return Error::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return Error::UNDEFINED;
    case PLACED:
      return Error::UNDEFINED;
    case PARTIALLY_FILLED:
      return Error::UNDEFINED;
    case FILLED:
      return Error::UNDEFINED;
    case CANCELLED:
      return Error::UNDEFINED;
    case EDITED:
      return Error::UNDEFINED;
    case MARKET_SUSPENDED:
      return Error::UNKNOWN;
    case MARKET_INACTIVE:
      return Error::UNKNOWN;
    case INVALID_PRICE:
      return Error::INVALID_PRICE;
    case INVALID_SIZE:
      return Error::INVALID_QUANTITY;
    case TOO_MANY_SMALL_ORDERS:
      return Error::UNKNOWN;
    case INSUFFICIENT_AVAILABLE_FUNDS:
      return Error::INSUFFICIENT_FUNDS;
    case WOULD_CAUSE_LIQUIDATION:
      return Error::UNKNOWN;
    case CLIENT_ORDER_ID_ALREADY_EXIST:
      return Error::UNKNOWN;
    case CLIENT_ORDER_ID_TOO_BIG:
      return Error::UNKNOWN;
    case MAX_POSITION_VIOLATION:
      return Error::UNKNOWN;
    case OUTSIDE_PRICE_COLLAR:
      return Error::UNKNOWN;
    case WOULD_INCREASE_PRICE_DISLOCATION:
      return Error::UNKNOWN;
    case NOT_FOUND:
      return Error::UNKNOWN_ORDER_ID;
    case ORDER_FOR_EDIT_NOT_A_STOP:
      return Error::UNKNOWN_ORDER_ID;
    case ORDER_FOR_EDIT_NOT_FOUND:
      return Error::TOO_LATE_TO_MODIFY_OR_CANCEL;
    case POST_WOULD_EXECUTE:
      return Error::UNKNOWN;
    case IOC_WOULD_NOT_EXECUTE:
      return Error::UNKNOWN;
    case SELF_FILL:
      return Error::UNKNOWN;
    case WOULD_NOT_REDUCE_POSITION:
      return Error::UNKNOWN;
    case MARKET_IS_POST_ONLY:
      return Error::UNKNOWN;
    case TOO_MANY_ORDERS:
      return Error::UNKNOWN;
    case FIXED_LEVERAGE_TOO_HIGH:
      return Error::UNKNOWN;
    case CLIENT_ORDER_ID_INVALID:
      return Error::UNKNOWN;
    case CANNOT_EDIT_TRIGGER_PRICE_OF_TRAILING_STOP:
      return Error::UNKNOWN;
    case CANNOT_EDIT_LIMIT_PRICE_OF_TRAILING_STOP:
      return Error::UNKNOWN;
    case WOULD_PROCESS_AFTER_SPECIFIED_TIME:
      return Error::UNKNOWN;
    case NO_ORDERS_TO_CANCEL:
      return Error::UNKNOWN;
    case INVALID_ORDER_TYPE:
      return Error::INVALID_ORDER_TYPE;
    case INVALID_SIDE:
      return Error::INVALID_SIDE;
    case CLIENT_ORDER_ID_TOO_LONG:
      return Error::UNKNOWN;
  }
  return {};
}

static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::UNDEFINED_INTERNAL}} == roq::Error::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::PLACED}} == roq::Error::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::PARTIALLY_FILLED}} == roq::Error::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::FILLED}} == roq::Error::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CANCELLED}} == roq::Error::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::EDITED}} == roq::Error::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MARKET_SUSPENDED}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MARKET_INACTIVE}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_PRICE}} == roq::Error::INVALID_PRICE);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_SIZE}} == roq::Error::INVALID_QUANTITY);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::TOO_MANY_SMALL_ORDERS}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INSUFFICIENT_AVAILABLE_FUNDS}} == roq::Error::INSUFFICIENT_FUNDS);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_CAUSE_LIQUIDATION}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_ALREADY_EXIST}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_TOO_BIG}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MAX_POSITION_VIOLATION}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::OUTSIDE_PRICE_COLLAR}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_INCREASE_PRICE_DISLOCATION}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::NOT_FOUND}} == roq::Error::UNKNOWN_ORDER_ID);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::ORDER_FOR_EDIT_NOT_A_STOP}} == roq::Error::UNKNOWN_ORDER_ID);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::ORDER_FOR_EDIT_NOT_FOUND}} == roq::Error::TOO_LATE_TO_MODIFY_OR_CANCEL);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::POST_WOULD_EXECUTE}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::IOC_WOULD_NOT_EXECUTE}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::SELF_FILL}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_NOT_REDUCE_POSITION}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MARKET_IS_POST_ONLY}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::TOO_MANY_ORDERS}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::FIXED_LEVERAGE_TOO_HIGH}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_INVALID}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CANNOT_EDIT_TRIGGER_PRICE_OF_TRAILING_STOP}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CANNOT_EDIT_LIMIT_PRICE_OF_TRAILING_STOP}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_PROCESS_AFTER_SPECIFIED_TIME}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::NO_ORDERS_TO_CANCEL}} == roq::Error::UNKNOWN);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_ORDER_TYPE}} == roq::Error::INVALID_ORDER_TYPE);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_SIDE}} == roq::Error::INVALID_SIDE);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_TOO_LONG}} == roq::Error::UNKNOWN);

template <>
template <>
std::optional<roq::Error> Map<kraken_futures::json::Status>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::Status => roq::OrderStatus

template <>
template <>
constexpr Helper<kraken_futures::json::Status>::operator std::optional<roq::OrderStatus>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::Status::type_t;
    case UNDEFINED_INTERNAL:
      return OrderStatus::UNDEFINED;
    case UNKNOWN_INTERNAL:
      return OrderStatus::UNDEFINED;
    case PLACED:
      return OrderStatus::WORKING;
    case PARTIALLY_FILLED:
      return OrderStatus::WORKING;
    case FILLED:
      return OrderStatus::COMPLETED;
    case CANCELLED:
      return OrderStatus::CANCELED;
    case EDITED:
      return OrderStatus::WORKING;
    case MARKET_SUSPENDED:
      return OrderStatus::REJECTED;
    case MARKET_INACTIVE:
      return OrderStatus::REJECTED;
    case INVALID_PRICE:
      return OrderStatus::REJECTED;
    case INVALID_SIZE:
      return OrderStatus::REJECTED;
    case TOO_MANY_SMALL_ORDERS:
      return OrderStatus::REJECTED;
    case INSUFFICIENT_AVAILABLE_FUNDS:
      return OrderStatus::REJECTED;
    case WOULD_CAUSE_LIQUIDATION:
      return OrderStatus::REJECTED;
    case CLIENT_ORDER_ID_ALREADY_EXIST:
      return OrderStatus::REJECTED;
    case CLIENT_ORDER_ID_TOO_BIG:
      return OrderStatus::REJECTED;
    case MAX_POSITION_VIOLATION:
      return OrderStatus::REJECTED;
    case OUTSIDE_PRICE_COLLAR:
      return OrderStatus::REJECTED;
    case WOULD_INCREASE_PRICE_DISLOCATION:
      return OrderStatus::REJECTED;
    case NOT_FOUND:
      return OrderStatus::REJECTED;
    case ORDER_FOR_EDIT_NOT_A_STOP:
      return OrderStatus::REJECTED;
    case ORDER_FOR_EDIT_NOT_FOUND:
      return OrderStatus::REJECTED;
    case POST_WOULD_EXECUTE:
      return OrderStatus::REJECTED;
    case IOC_WOULD_NOT_EXECUTE:
      return OrderStatus::REJECTED;
    case SELF_FILL:
      return OrderStatus::REJECTED;
    case WOULD_NOT_REDUCE_POSITION:
      return OrderStatus::REJECTED;
    case MARKET_IS_POST_ONLY:
      return OrderStatus::REJECTED;
    case TOO_MANY_ORDERS:
      return OrderStatus::REJECTED;
    case FIXED_LEVERAGE_TOO_HIGH:
      return OrderStatus::REJECTED;
    case CLIENT_ORDER_ID_INVALID:
      return OrderStatus::REJECTED;
    case CANNOT_EDIT_TRIGGER_PRICE_OF_TRAILING_STOP:
      return OrderStatus::REJECTED;
    case CANNOT_EDIT_LIMIT_PRICE_OF_TRAILING_STOP:
      return OrderStatus::REJECTED;
    case WOULD_PROCESS_AFTER_SPECIFIED_TIME:
      return OrderStatus::REJECTED;
    case NO_ORDERS_TO_CANCEL:
      return OrderStatus::REJECTED;
    case INVALID_ORDER_TYPE:
      return OrderStatus::REJECTED;
    case INVALID_SIDE:
      return OrderStatus::REJECTED;
    case CLIENT_ORDER_ID_TOO_LONG:
      return OrderStatus::REJECTED;
  }
  return {};
}

static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::UNDEFINED_INTERNAL}} == roq::OrderStatus::UNDEFINED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::PLACED}} == roq::OrderStatus::WORKING);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::PARTIALLY_FILLED}} == roq::OrderStatus::WORKING);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::FILLED}} == roq::OrderStatus::COMPLETED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CANCELLED}} == roq::OrderStatus::CANCELED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::EDITED}} == roq::OrderStatus::WORKING);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MARKET_SUSPENDED}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MARKET_INACTIVE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_PRICE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_SIZE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::TOO_MANY_SMALL_ORDERS}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INSUFFICIENT_AVAILABLE_FUNDS}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_CAUSE_LIQUIDATION}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_ALREADY_EXIST}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_TOO_BIG}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MAX_POSITION_VIOLATION}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::OUTSIDE_PRICE_COLLAR}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_INCREASE_PRICE_DISLOCATION}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::NOT_FOUND}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::ORDER_FOR_EDIT_NOT_A_STOP}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::ORDER_FOR_EDIT_NOT_FOUND}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::POST_WOULD_EXECUTE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::IOC_WOULD_NOT_EXECUTE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::SELF_FILL}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_NOT_REDUCE_POSITION}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::MARKET_IS_POST_ONLY}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::TOO_MANY_ORDERS}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::FIXED_LEVERAGE_TOO_HIGH}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_INVALID}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CANNOT_EDIT_TRIGGER_PRICE_OF_TRAILING_STOP}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CANNOT_EDIT_LIMIT_PRICE_OF_TRAILING_STOP}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::WOULD_PROCESS_AFTER_SPECIFIED_TIME}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::NO_ORDERS_TO_CANCEL}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_ORDER_TYPE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::INVALID_SIDE}} == roq::OrderStatus::REJECTED);
static_assert(Helper{kraken_futures::json::Status{kraken_futures::json::Status::CLIENT_ORDER_ID_TOO_LONG}} == roq::OrderStatus::REJECTED);

template <>
template <>
std::optional<roq::OrderStatus> Map<kraken_futures::json::Status>::helper() const {
  return Helper{args_};
}

// roq => kraken_futures::json

// roq::Side => kraken_futures::json::Side

template <>
template <>
constexpr Helper<roq::Side>::operator std::optional<kraken_futures::json::Side>() const {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return kraken_futures::json::Side::UNDEFINED_INTERNAL;
    case BUY:
      return kraken_futures::json::Side::BUY;
    case SELL:
      return kraken_futures::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == kraken_futures::json::Side{kraken_futures::json::Side::UNDEFINED_INTERNAL});
static_assert(Helper{roq::Side::BUY} == kraken_futures::json::Side{kraken_futures::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == kraken_futures::json::Side{kraken_futures::json::Side::SELL});

template <>
template <>
std::optional<kraken_futures::json::Side> Map<Side>::helper() const {
  return Helper{args_};
}

}  // namespace roq

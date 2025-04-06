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
    case UNDEFINED__:
      return Liquidity::UNDEFINED;
    case UNKNOWN__:
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

static_assert(Helper{kraken_futures::json::FillType{kraken_futures::json::FillType::UNDEFINED__}} == roq::Liquidity::UNDEFINED);
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
    case UNDEFINED__:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN__:
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

static_assert(Helper{kraken_futures::json::OrderEventOrderType{kraken_futures::json::OrderEventOrderType::UNDEFINED__}} == roq::OrderType::UNDEFINED);
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
    case UNDEFINED__:
      return roq::OrderType::UNDEFINED;
    case UNKNOWN__:
      return roq::OrderType::UNDEFINED;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      return roq::OrderType::LIMIT;  // XXX could also be market ???
  }
  return {};
}

static_assert(Helper{kraken_futures::json::OrderType{kraken_futures::json::OrderType::UNDEFINED__}} == roq::OrderType::UNDEFINED);
static_assert(Helper{kraken_futures::json::OrderType{kraken_futures::json::OrderType::LIMIT}} == roq::OrderType::LIMIT);
static_assert(Helper{kraken_futures::json::OrderType{kraken_futures::json::OrderType::STOP}} == roq::OrderType::LIMIT);

template <>
template <>
std::optional<roq::OrderType> Map<kraken_futures::json::OrderType>::helper() const {
  return Helper{args_};
}

// kraken_futures::json::Side => roq::Side

template <>
template <>
constexpr Helper<kraken_futures::json::Side>::operator std::optional<roq::Side>() const {
  switch (std::get<0>(args_)) {
    using enum kraken_futures::json::Side::type_t;
    case UNDEFINED__:
      return roq::Side::UNDEFINED;
    case UNKNOWN__:
      return roq::Side::UNDEFINED;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

static_assert(Helper{kraken_futures::json::Side{kraken_futures::json::Side::UNDEFINED__}} == roq::Side::UNDEFINED);
static_assert(Helper{kraken_futures::json::Side{kraken_futures::json::Side::BUY}} == roq::Side::BUY);
static_assert(Helper{kraken_futures::json::Side{kraken_futures::json::Side::SELL}} == roq::Side::SELL);

template <>
template <>
std::optional<roq::Side> Map<kraken_futures::json::Side>::helper() const {
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
      return kraken_futures::json::Side::UNDEFINED__;
    case BUY:
      return kraken_futures::json::Side::BUY;
    case SELL:
      return kraken_futures::json::Side::SELL;
  }
  return {};
}

static_assert(Helper{roq::Side::UNDEFINED} == kraken_futures::json::Side{kraken_futures::json::Side::UNDEFINED__});
static_assert(Helper{roq::Side::BUY} == kraken_futures::json::Side{kraken_futures::json::Side::BUY});
static_assert(Helper{roq::Side::SELL} == kraken_futures::json::Side{kraken_futures::json::Side::SELL});

template <>
template <>
std::optional<kraken_futures::json::Side> Map<Side>::helper() const {
  return Helper{args_};
}

}  // namespace roq

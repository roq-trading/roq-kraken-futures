/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/kraken_futures/json/map.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace json {

// === HELPERS ===

namespace {
// note! constexpr helper for static testing
template <typename... Args>
struct Helper final {
  explicit constexpr Helper(std::tuple<Args...> const &args) : args_{args} {}
  explicit constexpr Helper(Args &&...args_) : args_{std::forward<Args>(args_)...} {}

  template <typename R>
  constexpr operator R();

 private:
  std::tuple<Args...> const args_;
};

// ==> roq

// FillType ==> roq::OrderType

template <>
template <>
constexpr Helper<FillType>::operator roq::Liquidity() {
  switch (std::get<0>(args_)) {
    using enum json::FillType::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case MAKER:
      return Liquidity::MAKER;
    case TAKER:
      return Liquidity::TAKER;
    case LIQUIDATION:
      return {};  // note!
    case ASSIGNEE:
      return {};  // note!
    case ASSIGNOR:
      return {};  // note!
    case UNWIND_BANKRUPT:
      return {};  // note!
    case UNWIND_COUNTERPARTY:
      return {};  // note!
    case TAKER_AFTER_EDIT:
      return {};  // note!
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::UNDEFINED__}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::MAKER}}) == roq::Liquidity::MAKER);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::TAKER}}) == roq::Liquidity::TAKER);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::LIQUIDATION}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::ASSIGNEE}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::ASSIGNOR}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::UNWIND_BANKRUPT}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::UNWIND_COUNTERPARTY}}) == roq::Liquidity::UNDEFINED);
static_assert(static_cast<roq::Liquidity>(Helper{FillType{FillType::TAKER_AFTER_EDIT}}) == roq::Liquidity::UNDEFINED);

// OrderEventOrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<OrderEventOrderType>::operator roq::OrderType() {
  switch (std::get<0>(args_)) {
    using enum json::OrderEventOrderType::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case LMT:
      return roq::OrderType::LIMIT;
    case MKT:
      return roq::OrderType::MARKET;
    case STP:
      return {};  // note!
    case TAKE_PROFIT:
      return {};  // note!
    case IOC:
      return roq::OrderType::LIMIT;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderType>(Helper{OrderEventOrderType{OrderEventOrderType::UNDEFINED__}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderEventOrderType{OrderEventOrderType::LMT}}) == roq::OrderType::LIMIT);
static_assert(static_cast<roq::OrderType>(Helper{OrderEventOrderType{OrderEventOrderType::MKT}}) == roq::OrderType::MARKET);
static_assert(static_cast<roq::OrderType>(Helper{OrderEventOrderType{OrderEventOrderType::STP}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderEventOrderType{OrderEventOrderType::TAKE_PROFIT}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderEventOrderType{OrderEventOrderType::IOC}}) == roq::OrderType::LIMIT);

// OrderType ==> roq::OrderType

template <>
template <>
constexpr Helper<OrderType>::operator roq::OrderType() {
  switch (std::get<0>(args_)) {
    using enum json::OrderType::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      return roq::OrderType::LIMIT;  // XXX could also be market ???
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::UNDEFINED__}}) == roq::OrderType::UNDEFINED);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::LIMIT}}) == roq::OrderType::LIMIT);
static_assert(static_cast<roq::OrderType>(Helper{OrderType{OrderType::STOP}}) == roq::OrderType::LIMIT);

// Side ==> roq::Side

template <>
template <>
constexpr Helper<Side>::operator roq::Side() {
  switch (std::get<0>(args_)) {
    using enum Side::type_t;
    case UNDEFINED__:
      return {};
    case UNKNOWN__:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<roq::Side>(Helper{Side{Side::UNDEFINED__}}) == roq::Side::UNDEFINED);
static_assert(static_cast<roq::Side>(Helper{Side{Side::BUY}}) == roq::Side::BUY);
static_assert(static_cast<roq::Side>(Helper{Side{Side::SELL}}) == roq::Side::SELL);

// roq ==>

// roq::Side ==> Side

template <>
template <>
constexpr Helper<roq::Side>::operator Side() {
  switch (std::get<0>(args_)) {
    using enum roq::Side;
    case UNDEFINED:
      return {};
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  roq::log::fatal("Unexpected"sv);
}

static_assert(static_cast<Side>(Helper{roq::Side::UNDEFINED}) == json::Side{json::Side::UNDEFINED__});
static_assert(static_cast<Side>(Helper{roq::Side::BUY}) == json::Side{json::Side::BUY});
static_assert(static_cast<Side>(Helper{roq::Side::SELL}) == json::Side{json::Side::SELL});

}  // namespace

// === IMPLEMENTATION ===

// ==> roq

template <>
template <>
Map<FillType>::operator roq::Liquidity() {
  return Helper{args_};
}

template <>
template <>
Map<OrderEventOrderType>::operator roq::OrderType() {
  return Helper{args_};
}

template <>
template <>
Map<OrderType>::operator roq::OrderType() {
  return Helper{args_};
}

template <>
template <>
Map<Side>::operator roq::Side() {
  return Helper{args_};
}

template <>
template <>
Map<roq::Side>::operator Side() {
  return Helper{args_};
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

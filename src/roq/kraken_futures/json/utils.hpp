/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/core/charconv/datetime.hpp"

#include "roq/kraken_futures/json/fill_type.hpp"
#include "roq/kraken_futures/json/order_event_order_type.hpp"
#include "roq/kraken_futures/json/order_type.hpp"
#include "roq/kraken_futures/json/side.hpp"

namespace roq {
namespace kraken_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  return std::visit(
      overloaded{
          [&](core::json::Null const &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = std::chrono::milliseconds{static_cast<uint64_t>(value)}; },
          [&](double value) { result = std::chrono::milliseconds{static_cast<uint64_t>(value)}; },
          [&](std::string_view const &value) {
            result = core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(value);
          },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

inline roq::Liquidity map(json::FillType value) {
  switch (value) {
    using enum json::FillType::type_t;
    case UNDEFINED:
    case UNKNOWN:
      break;
    case MAKER:
      return Liquidity::MAKER;
    case TAKER:
      return Liquidity::TAKER;
    case LIQUIDATION:
      break;
    case ASSIGNEE:
      break;
    case ASSIGNOR:
      break;
    case UNWIND_BANKRUPT:
      break;
    case UNWIND_COUNTERPARTY:
      break;
    case TAKER_AFTER_EDIT:
      break;
  }
  return {};
}

inline roq::OrderType map(json::OrderEventOrderType value) {
  switch (value) {
    using enum json::OrderEventOrderType::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case LMT:
      return roq::OrderType::LIMIT;
    case MKT:
      return roq::OrderType::MARKET;
    case STP:
      break;
    case TAKE_PROFIT:
      break;
    case IOC:
      return roq::OrderType::LIMIT;
  }
  return {};
}

inline roq::OrderType map(json::OrderType value) {
  switch (value) {
    using enum json::OrderType::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case LIMIT:
      return roq::OrderType::LIMIT;
    case STOP:
      return roq::OrderType::LIMIT;  // XXX could also be market ???
  }
  return {};
}

inline roq::Side map(json::Side value) {
  switch (value) {
    using enum json::Side::type_t;
    case UNDEFINED:
      break;
    case UNKNOWN:
      break;
    case BUY:
      return roq::Side::BUY;
    case SELL:
      return roq::Side::SELL;
  }
  return {};
}

inline json::Side map(roq::Side value) {
  switch (value) {
    using enum roq::Side;
    case UNDEFINED:
      break;
    case BUY:
      return json::Side::BUY;
    case SELL:
      return json::Side::SELL;
  }
  return {};
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

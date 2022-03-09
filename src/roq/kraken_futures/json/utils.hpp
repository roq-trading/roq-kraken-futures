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
inline void update(T &result, const core::json::value_t &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, const core::json::value_t &value) {
  return std::visit(
      overloaded{
          [&](const core::json::null_t &) { result = std::chrono::milliseconds{}; },
          [](bool) { throw std::bad_cast(); },
          [&](int64_t value) { result = std::chrono::milliseconds{static_cast<uint64_t>(value)}; },
          [&](double value) { result = std::chrono::milliseconds{static_cast<uint64_t>(value)}; },
          [&](const std::string_view &value) {
            result =
                core::charconv::datetime_from_string<std::remove_reference<decltype(result)>::type>(
                    value);
          },
          [](const core::json::object_t &) { throw std::bad_cast(); },
          [](const core::json::array_t &) { throw std::bad_cast(); },
      },
      value);
}

inline roq::Liquidity map(json::FillType value) {
  switch (value) {
    case json::FillType::UNDEFINED:
    case json::FillType::UNKNOWN:
      break;
    case json::FillType::MAKER:
      return Liquidity::MAKER;
    case json::FillType::TAKER:
      return Liquidity::TAKER;
    case json::FillType::LIQUIDATION:
      break;
    case json::FillType::ASSIGNEE:
      break;
    case json::FillType::ASSIGNOR:
      break;
    case json::FillType::UNWIND_BANKRUPT:
      break;
    case json::FillType::UNWIND_COUNTERPARTY:
      break;
    case json::FillType::TAKER_AFTER_EDIT:
      break;
  }
  return {};
}

inline roq::OrderType map(json::OrderEventOrderType value) {
  switch (value) {
    case json::OrderEventOrderType::UNDEFINED:
      break;
    case json::OrderEventOrderType::UNKNOWN:
      break;
    case json::OrderEventOrderType::LMT:
      return roq::OrderType::LIMIT;
    case json::OrderEventOrderType::MKT:
      return roq::OrderType::MARKET;
    case json::OrderEventOrderType::STP:
      break;
    case json::OrderEventOrderType::TAKE_PROFIT:
      break;
    case json::OrderEventOrderType::IOC:
      return roq::OrderType::LIMIT;
  }
  return {};
}

inline roq::OrderType map(json::OrderType value) {
  switch (value) {
    case json::OrderType::UNDEFINED:
      break;
    case json::OrderType::UNKNOWN:
      break;
    case json::OrderType::LIMIT:
      return roq::OrderType::LIMIT;
    case json::OrderType::STOP:
      return roq::OrderType::LIMIT;  // XXX could also be market ???
  }
  return {};
}

inline roq::Side map(json::Side value) {
  switch (value) {
    case json::Side::UNDEFINED:
      break;
    case json::Side::UNKNOWN:
      break;
    case json::Side::BUY:
      return roq::Side::BUY;
    case json::Side::SELL:
      return roq::Side::SELL;
  }
  return {};
}

inline json::Side map(roq::Side value) {
  switch (value) {
    case roq::Side::UNDEFINED:
      break;
    case roq::Side::BUY:
      return json::Side::BUY;
    case roq::Side::SELL:
      return json::Side::SELL;
  }
  return {};
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

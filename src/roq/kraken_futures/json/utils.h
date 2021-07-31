/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.h"

#include "roq/core/json/parser.h"

#include "roq/core/charconv/datetime.h"

#include "roq/kraken_futures/json/order_event_type.h"
#include "roq/kraken_futures/json/order_type.h"
#include "roq/kraken_futures/json/side.h"

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

inline roq::OrderType map(json::OrderEventType value) {
  switch (value) {
    case json::OrderEventType::UNDEFINED:
      break;
    case json::OrderEventType::UNKNOWN:
      break;
    case json::OrderEventType::LMT:
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

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

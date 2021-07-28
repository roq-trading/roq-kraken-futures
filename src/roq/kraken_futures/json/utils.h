/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/core/utility.h"

#include "roq/core/json/parser.h"

#include "roq/core/charconv/datetime.h"

#include "roq/kraken_futures/json/result.h"

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

template <>
inline void update(Result &result, const core::json::value_t &value) {
  using result_type = std::remove_reference<decltype(result)>::type;
  result = result_type(core::json::get<std::string_view>(value));
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

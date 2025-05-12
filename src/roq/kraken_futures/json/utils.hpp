/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <chrono>

#include "roq/utils/patterns.hpp"

#include "roq/utils/charconv/from_chars.hpp"

#include "roq/core/json/parser.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace kraken_futures {
namespace json {

template <typename T>
inline void update(T &result, core::json::Value const &value) {
  result = core::json::get<T>(value);
}

template <>
inline void update(std::chrono::milliseconds &result, core::json::Value const &value) {
  using result_type = std::remove_cvref_t<decltype(result)>;
  return std::visit(
      utils::overloaded{
          [&](core::json::Null const &) { result = result_type{}; },
          [](bool) { throw std::bad_cast{}; },
          [&](int64_t value) { result = result_type{static_cast<uint64_t>(value)}; },
          [&](double value) { result = result_type{static_cast<uint64_t>(value)}; },
          [&](std::string_view const &value) { result = utils::charconv::from_chars<result_type>(value, utils::charconv::Format::DATETIME); },
          [](core::json::Object const &) { throw std::bad_cast{}; },
          [](core::json::Array const &) { throw std::bad_cast{}; },
      },
      value);
}

extern std::string_view send_order(std::vector<char> &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id);

extern std::string_view edit_order(
    std::vector<char> &buffer,
    ModifyOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id);

extern std::string_view cancel_order(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &,
    std::string_view const &request_id,
    std::string_view const &previous_request_id);

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

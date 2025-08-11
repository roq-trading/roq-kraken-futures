/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/kraken_futures/settings.hpp"

namespace roq {
namespace kraken_futures {

struct API final {
  struct {
    std::string_view instruments;
    std::string_view charts_trade;
  } market_data;
  struct {
    std::string_view send_order;
    std::string_view edit_order;
    std::string_view cancel_order;
    std::string_view cancel_all_orders;
    std::string_view cancel_all_orders_after;
  } order_management;

  // factory
  static API create(Settings const &);
};

}  // namespace kraken_futures
}  // namespace roq

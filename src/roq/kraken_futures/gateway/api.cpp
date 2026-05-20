/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kraken_futures/gateway/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace gateway {

// === IMPLEMENTATION ===

API API::create(Settings const &) {
  return {
      .market_data{
          .instruments = "/derivatives/api/v3/instruments"sv,
          .charts_trade = "/api/charts/v1/trade"sv,
      },
      .order_management{
          .send_order = "/derivatives/api/v3/sendorder"sv,
          .edit_order = "/derivatives/api/v3/editorder"sv,
          .cancel_order = "/derivatives/api/v3/cancelorder"sv,
          .cancel_all_orders = "/derivatives/api/v3/cancelallorders"sv,
          .cancel_all_orders_after = "/derivatives/api/v3/cancelallordersafter"sv,
      },
  };
}

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

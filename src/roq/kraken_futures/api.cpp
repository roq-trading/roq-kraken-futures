/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include "roq/kraken_futures/api.hpp"

#include "roq/exceptions.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

API API::create(Settings const &) {
  return {
      .market_data{
          .instruments = "/api/v3/instruments"sv,
      },
      .order_management{
          .send_order = "/api/v3/sendorder"sv,
          .edit_order = "/api/v3/editorder"sv,
          .cancel_order = "/api/v3/cancelorder"sv,
          .cancel_all_orders = "/api/v3/cancelallorders"sv,
          .cancel_all_orders_after = "/api/v3/cancelallordersafter"sv,
      },
  };
}

}  // namespace kraken_futures
}  // namespace roq

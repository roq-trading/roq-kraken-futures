/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "roq/cancel_all_orders.hpp"
#include "roq/cancel_order.hpp"
#include "roq/create_order.hpp"
#include "roq/modify_order.hpp"

#include "roq/server/oms/order.hpp"

namespace roq {
namespace kraken_futures {
namespace json {

struct Encoder final {
  static std::string_view send_order(std::vector<char> &buffer, CreateOrder const &, server::oms::Order const &, std::string_view const &request_id);

  static std::string_view edit_order(
      std::vector<char> &buffer,
      ModifyOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);

  static std::string_view cancel_order(
      std::vector<char> &buffer,
      roq::CancelOrder const &,
      server::oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

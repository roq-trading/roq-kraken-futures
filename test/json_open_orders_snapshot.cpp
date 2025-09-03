/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kraken_futures/json/open_orders_snapshot.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_open_orders_snapshot_simple", "[json_open_orders_snapshot]") {
  auto message = R"({)"
                 R"("feed":"open_orders_snapshot",)"
                 R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
                 R"("orders":[)"
                 R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627577572583,)"
                 R"("last_update_time":1627577572583,)"
                 R"("qty":1.0,)"
                 R"("filled":0.0,)"
                 R"("limit_price":39528.0,)"
                 R"("stop_price":0.0,)"
                 R"("type":"limit",)"
                 R"("order_id":"494f7cb0-6936-495f-a0c5-663ad9b9fbdd",)"
                 R"("direction":0,)"
                 R"("reduce_only":false)"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::OpenOrdersSnapshot obj{message, buffer};
  CHECK(obj.feed == json::Feed::OPEN_ORDERS_SNAPSHOT);
  CHECK(obj.account == "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  CHECK(std::size(obj.orders) == 1);
  // idx 0
  auto &order = obj.orders[0];
  CHECK(order.instrument == "PI_XBTUSD"sv);
  CHECK(order.time == 1627577572583ms);
  CHECK(order.last_update_time == 1627577572583ms);
  CHECK(order.qty == 1.0_a);
  CHECK(order.filled == 0.0_a);
  CHECK(order.limit_price == 39528.0_a);
  CHECK(order.stop_price == 0.0_a);
  CHECK(order.type == json::OrderType::LIMIT);
  CHECK(order.order_id == "494f7cb0-6936-495f-a0c5-663ad9b9fbdd"sv);
  CHECK(order.direction == 0);
  CHECK(order.reduce_only == false);
}

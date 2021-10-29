/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/open_orders_snapshot.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_open_orders_snapshot, simple) {
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrdersSnapshot>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS_SNAPSHOT);
  EXPECT_EQ(obj.account, "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  EXPECT_EQ(std::size(obj.orders), 1);
  // idx 0
  auto &order = obj.orders[0];
  EXPECT_EQ(order.instrument, "PI_XBTUSD"sv);
  EXPECT_EQ(order.time, 1627577572583ms);
  EXPECT_EQ(order.last_update_time, 1627577572583ms);
  EXPECT_DOUBLE_EQ(order.qty, 1.0);
  EXPECT_DOUBLE_EQ(order.filled, 0.0);
  EXPECT_DOUBLE_EQ(order.limit_price, 39528.0);
  EXPECT_DOUBLE_EQ(order.stop_price, 0.0);
  EXPECT_EQ(order.type, json::OrderType::LIMIT);
  EXPECT_EQ(order.order_id, "494f7cb0-6936-495f-a0c5-663ad9b9fbdd"sv);
  EXPECT_EQ(order.direction, 0);
  EXPECT_EQ(order.reduce_only, false);
}

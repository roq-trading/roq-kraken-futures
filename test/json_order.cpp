/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/order.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_order, simple) {
  auto message = R"({)"
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
                 R"(})";
  auto obj = core::json::Parser::create<json::Order>(message);
  EXPECT_EQ(obj.instrument, "PI_XBTUSD"sv);
  EXPECT_EQ(obj.time, 1627577572583ms);
  EXPECT_EQ(obj.last_update_time, 1627577572583ms);
  EXPECT_DOUBLE_EQ(obj.qty, 1.0);
  EXPECT_DOUBLE_EQ(obj.filled, 0.0);
  EXPECT_DOUBLE_EQ(obj.limit_price, 39528.0);
  EXPECT_DOUBLE_EQ(obj.stop_price, 0.0);
  EXPECT_EQ(obj.type, json::OrderType::LIMIT);
  EXPECT_EQ(obj.order_id, "494f7cb0-6936-495f-a0c5-663ad9b9fbdd"sv);
  EXPECT_EQ(obj.direction, 0);
  EXPECT_EQ(obj.reduce_only, false);
}

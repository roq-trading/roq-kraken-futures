/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/fills_snapshot.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

TEST(json_fills_snapshot, simple) {
  auto message = R"({)"
                 R"("feed":"fills_snapshot",)"
                 R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
                 R"("fills":[)"
                 R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627876320033,)"
                 R"("price":40065.0,)"
                 R"("seq":0,)"
                 R"("buy":true,)"
                 R"("qty":1.0,)"
                 R"("order_id":"f109eb54-a223-4503-99c5-00f053b9411e",)"
                 R"("cli_ord_id":"egAF6gMAAQAAyEmOD8AQ",)"
                 R"("fill_id":"8a5a6c01-926b-4ddd-9017-084df7ca4510",)"
                 R"("fill_type":"maker",)"
                 R"("fee_paid":5e-9,)"
                 R"("fee_currency":"BTC")"
                 R"(},)"
                 R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627876380033,)"
                 R"("price":40066.5,)"
                 R"("seq":1,)"
                 R"("buy":false,)"
                 R"("qty":1.0,)"
                 R"("order_id":"1ba2d300-6f26-4fd6-b573-7050cfbff08d",)"
                 R"("fill_id":"86551968-4417-46b8-9e36-26b05642ecaa",)"
                 R"("fill_type":"maker",)"
                 R"("fee_paid":5e-9,)"
                 R"("fee_currency":"BTC")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::FillsSnapshot>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::FILLS_SNAPSHOT);
  EXPECT_EQ(obj.account, "bdb7a134-386a-45c0-b8e5-76a75537df4c"_sv);
  EXPECT_EQ(std::size(obj.fills), 2);
  // idx 0
  auto &fill_0 = obj.fills[0];
  EXPECT_EQ(fill_0.instrument, "PI_XBTUSD"_sv);
  EXPECT_EQ(fill_0.time, 1627876320033ms);
  EXPECT_DOUBLE_EQ(fill_0.price, 40065.0);
  EXPECT_EQ(fill_0.seq, 0);
  EXPECT_EQ(fill_0.buy, true);
  EXPECT_DOUBLE_EQ(fill_0.qty, 1.0);
  EXPECT_EQ(fill_0.order_id, "f109eb54-a223-4503-99c5-00f053b9411e"_sv);
  EXPECT_EQ(fill_0.cli_ord_id, "egAF6gMAAQAAyEmOD8AQ"_sv);
  EXPECT_EQ(fill_0.fill_id, "8a5a6c01-926b-4ddd-9017-084df7ca4510"_sv);
  EXPECT_EQ(fill_0.fill_type, json::FillType::MAKER);
  EXPECT_DOUBLE_EQ(fill_0.fee_paid, 5.0e-9);
  EXPECT_EQ(fill_0.fee_currency, "BTC"_sv);
  // idx 1
  auto &fill_1 = obj.fills[1];
  EXPECT_EQ(fill_1.instrument, "PI_XBTUSD"_sv);
  EXPECT_EQ(fill_1.time, 1627876380033ms);
  EXPECT_DOUBLE_EQ(fill_1.price, 40066.5);
  EXPECT_EQ(fill_1.seq, 1);
  EXPECT_EQ(fill_1.buy, false);
  EXPECT_DOUBLE_EQ(fill_1.qty, 1.0);
  EXPECT_EQ(fill_1.order_id, "1ba2d300-6f26-4fd6-b573-7050cfbff08d"_sv);
  EXPECT_EQ(fill_1.fill_id, "86551968-4417-46b8-9e36-26b05642ecaa"_sv);
  EXPECT_EQ(fill_1.fill_type, json::FillType::MAKER);
  EXPECT_DOUBLE_EQ(fill_1.fee_paid, 5.0e-9);
  EXPECT_EQ(fill_1.fee_currency, "BTC"_sv);
}

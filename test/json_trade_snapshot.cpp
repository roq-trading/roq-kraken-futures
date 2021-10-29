/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/trade_snapshot.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_trade_snapshot, simple) {
  auto message =
      R"({)"
      R"("feed":"trade_snapshot",)"
      R"("product_id":"PI_XBTUSD",)"
      R"("trades":[)"
      R"({"feed":"trade","product_id":"PI_XBTUSD","uid":"55ac6115-7709-46cd-b90c-74b932cb2cb7","side":"buy","type":"fill","seq":6922,"time":1627483320015,"qty":10.0,"price":39941.5},)"
      R"({"feed":"trade","product_id":"PI_XBTUSD","uid":"77573277-22ae-45a5-af1c-40706348627d","side":"sell","type":"fill","seq":6921,"time":1627483260064,"qty":10.0,"price":39938.0},)"
      R"({"feed":"trade","product_id":"PI_XBTUSD","uid":"25ec0c98-676b-4522-a94e-f4a848f0ed84","side":"sell","type":"fill","seq":6920,"time":1627483200036,"qty":10.0,"price":39963.5})"
      R"(])"
      R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::TradeSnapshot>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::TRADE_SNAPSHOT);
  EXPECT_EQ(obj.product_id, "PI_XBTUSD"sv);
  EXPECT_EQ(std::size(obj.trades), 3);
}

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/trade.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_trade, simple) {
  auto message = R"({)"
                 R"("feed":"trade",)"
                 R"("product_id":"PI_LTCUSD",)"
                 R"("uid":"bee759b2-c264-4a9e-a8a6-07b60556786d",)"
                 R"("side":"buy",)"
                 R"("type":"fill",)"
                 R"("seq":3732,)"
                 R"("time":1627483389597,)"
                 R"("qty":1.0,)"
                 R"("price":136.53)"
                 R"(})";
  auto obj = core::json::Parser::create<json::Trade>(message);
  EXPECT_EQ(obj.feed, json::Feed::TRADE);
  EXPECT_EQ(obj.product_id, "PI_LTCUSD"sv);
  EXPECT_EQ(obj.uid, "bee759b2-c264-4a9e-a8a6-07b60556786d"sv);
  EXPECT_EQ(obj.side, json::Side::BUY);
  EXPECT_EQ(obj.type, json::TradeType::FILL);
  EXPECT_EQ(obj.seq, 3732);
  EXPECT_EQ(obj.time, 1627483389597ms);
  EXPECT_DOUBLE_EQ(obj.qty, 1.0);
  EXPECT_DOUBLE_EQ(obj.price, 136.53);
}

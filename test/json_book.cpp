/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/book.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_book, simple) {
  auto message = R"({)"
                 R"("feed":"book",)"
                 R"("product_id":"PI_XBTUSD",)"
                 R"("side":"buy",)"
                 R"("seq":1766422,)"
                 R"("price":39826.0,)"
                 R"("qty":3951.0,)"
                 R"("timestamp":1627535639684)"
                 R"(})";
  auto obj = core::json::Parser::create<json::Book>(message);
  EXPECT_EQ(obj.feed, json::Feed::BOOK);
  EXPECT_EQ(obj.product_id, "PI_XBTUSD"sv);
  EXPECT_EQ(obj.side, json::Side::BUY);
  EXPECT_EQ(obj.seq, 1766422);
  EXPECT_DOUBLE_EQ(obj.price, 39826.0);
  EXPECT_DOUBLE_EQ(obj.qty, 3951.0);
  EXPECT_EQ(obj.timestamp, 1627535639684ms);
}

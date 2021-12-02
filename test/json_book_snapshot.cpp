/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/book_snapshot.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_book_snapshot, simple) {
  auto message = R"({)"
                 R"("feed":"book_snapshot",)"
                 R"("product_id":"FI_LTCUSD_210924",)"
                 R"("timestamp":1627535620727,)"
                 R"("seq":732887,)"
                 R"("tickSize":null,)"
                 R"("bids":[)"
                 R"({"price":139.54,"qty":5000.0},)"
                 R"({"price":139.53,"qty":5000.0},)"
                 R"({"price":139.52,"qty":5000.0},)"
                 R"({"price":139.51,"qty":5000.0},)"
                 R"({"price":139.5,"qty":5000.0})"
                 R"(],)"
                 R"("asks":[)"
                 R"({"price":139.61,"qty":5000.0},)"
                 R"({"price":139.62,"qty":5000.0},)"
                 R"({"price":139.63,"qty":5000.0},)"
                 R"({"price":139.64,"qty":5000.0},)"
                 R"({"price":139.65,"qty":5000.0})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::BookSnapshot>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::BOOK_SNAPSHOT);
  EXPECT_EQ(obj.product_id, "FI_LTCUSD_210924"sv);
  EXPECT_EQ(obj.timestamp, 1627535620727ms);
  EXPECT_EQ(obj.seq, 732887);
  // tick_size
  EXPECT_EQ(std::size(obj.bids), 5);
  EXPECT_EQ(std::size(obj.asks), 5);
}

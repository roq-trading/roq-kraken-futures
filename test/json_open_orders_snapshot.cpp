/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/open_orders_snapshot.h"

using namespace roq;
using namespace roq::kraken_futures;

TEST(json_open_orders_snapshot, simple) {
  auto message =
      R"({)"
      R"("feed":"open_orders_snapshot",)"
      R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
      R"("orders":[)"
      R"({"instrument":"PI_XBTUSD","time":1627577572583,"last_update_time":1627577572583,"qty":1.0,"filled":0.0,"limit_price":39528.0,"stop_price":0.0,"type":"limit","order_id":"494f7cb0-6936-495f-a0c5-663ad9b9fbdd","direction":0,"reduce_only":false})"
      R"(])"
      R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrdersSnapshot>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS_SNAPSHOT);
  EXPECT_EQ(obj.account, "bdb7a134-386a-45c0-b8e5-76a75537df4c"_sv);
  EXPECT_EQ(std::size(obj.orders), 1);
}

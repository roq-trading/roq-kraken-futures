/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

// #include "roq/kraken_futures/json/cancel_all_orders.h"

using namespace roq;
// using namespace roq::kraken_futures;

using namespace std::chrono_literals;

/*
TEST(json_cancel_all_orders, no_orders) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("receivedTime":"2021-07-30T03:46:22.153Z",)"
                 R"("cancelOnly":"all",)"
                 R"("status":"noOrdersToCancel",)"
                 R"("cancelledOrders":[],)"
                 R"("orderEvents":[])"
                 R"(},)"
                 R"("serverTime":"2021-07-30T03:46:22.154Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CancelAllOrders>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.result.cancel_status.received_time, 1627609582153ms);
  EXPECT_EQ(obj.result.cancel_status.cancel_only, "all"_sv);
  EXPECT_EQ(obj.result.cancel_status.status, "noOrdersToCancel"_sv);
  EXPECT_EQ(std::size(obj.result.cancel_status.cancelled_orders), 0);
  EXPECT_EQ(std::size(obj.result.cancel_status.order_events), 0);
  EXPECT_EQ(obj.server_time, 1627609582154ms);
}
*/

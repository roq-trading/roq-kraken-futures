/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/cancel_order.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

TEST(json_cancel_order, simple) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("status":"cancelled",)"
                 R"("order_id":"85792364-8163-4e13-b62d-695e7f802e22",)"
                 R"("receivedTime":"2021-07-31T04:53:14.376Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("uid":"85792364-8163-4e13-b62d-695e7f802e22",)"
                 R"("order":{)"
                 R"("orderId":"85792364-8163-4e13-b62d-695e7f802e22",)"
                 R"("cliOrdId":"DwAF6QMAAQAAxMacsJgQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":41936,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-31T04:53:04.171Z",)"
                 R"("lastUpdateTimestamp":"2021-07-31T04:53:09.310Z")"
                 R"(},)"
                 R"("type":"CANCEL")"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-07-31T04:53:14.376Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CancelOrder>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.cancel_status.status, json::Status::CANCELLED);
  EXPECT_EQ(obj.cancel_status.order_id, "85792364-8163-4e13-b62d-695e7f802e22"_sv);
  EXPECT_EQ(obj.cancel_status.received_time, 1627707194376ms);
  EXPECT_EQ(std::size(obj.cancel_status.order_events), 1);
  // idx 0
  auto &event = obj.cancel_status.order_events[0];
  EXPECT_EQ(event.uid, "85792364-8163-4e13-b62d-695e7f802e22"_sv);
  EXPECT_EQ(event.order.order_id, "85792364-8163-4e13-b62d-695e7f802e22"_sv);
  EXPECT_EQ(event.order.cli_ord_id, "DwAF6QMAAQAAxMacsJgQ"_sv);
  EXPECT_EQ(event.order.type, json::OrderEventOrderType::LMT);
  EXPECT_EQ(event.order.symbol, "pi_xbtusd"_sv);
  EXPECT_EQ(event.order.side, json::Side::BUY);
  EXPECT_DOUBLE_EQ(event.order.quantity, 1.0);
  EXPECT_DOUBLE_EQ(event.order.filled, 0.0);
  EXPECT_DOUBLE_EQ(event.order.limit_price, 41936.0);
  EXPECT_EQ(event.order.reduce_only, false);
  EXPECT_EQ(event.order.timestamp, 1627707184171ms);
  EXPECT_EQ(event.order.last_update_timestamp, 1627707189310ms);
  // ...
  EXPECT_EQ(event.type, json::OrderEventType::CANCEL);
  // ...
  EXPECT_EQ(obj.server_time, 1627707194376ms);
}

TEST(json_cancel_order, not_found) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("status":"notFound",)"
                 R"("receivedTime":"2021-08-02T07:46:05.900Z")"
                 R"(},)"
                 R"("serverTime":"2021-08-02T07:46:05.900Z")"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CancelOrder>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.cancel_status.status, json::Status::NOT_FOUND);
  EXPECT_EQ(obj.cancel_status.received_time, 1627890365900ms);
  EXPECT_EQ(obj.server_time, 1627890365900ms);
}

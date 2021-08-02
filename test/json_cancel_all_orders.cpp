/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/cancel_all_orders.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

// {"result":"success","cancelStatus":{"receivedTime":"2021-07-31T06:20:10.487Z","cancelOnly":"all","status":"cancelled","cancelledOrders":[{"cliOrdId":"ewAF6QMAAQAAly0J6JkQ","order_id":"74f63887-f69c-491e-937d-3e53d038c806"}],"orderEvents":[{"uid":"74f63887-f69c-491e-937d-3e53d038c806","order":{"orderId":"74f63887-f69c-491e-937d-3e53d038c806","cliOrdId":"ewAF6QMAAQAAly0J6JkQ","type":"lmt","symbol":"pi_xbtusd","side":"buy","quantity":1,"filled":0,"limitPrice":41753.5,"reduceOnly":false,"timestamp":"2021-07-31T06:20:08.997Z","lastUpdateTimestamp":"2021-07-31T06:20:08.997Z"},"type":"CANCEL"}]},"serverTime":"2021-07-31T06:20:10.488Z"}

TEST(json_cancel_all_orders, no_orders) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("receivedTime":"2021-07-31T06:18:18.468Z",)"
                 R"("cancelOnly":"all",)"
                 R"("status":"noOrdersToCancel",)"
                 R"("cancelledOrders":[],)"
                 R"("orderEvents":[])"
                 R"(},)"
                 R"("serverTime":"2021-07-31T06:18:18.468Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CancelAllOrders>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.cancel_status.received_time, 1627712298468ms);
  EXPECT_EQ(obj.cancel_status.cancel_only, "all"_sv);
  EXPECT_EQ(obj.cancel_status.status, json::Status::NO_ORDERS_TO_CANCEL);
  // EXPECT_EQ(std::size(obj.cancel_status.cancelled_orders), 0);
  EXPECT_EQ(std::size(obj.cancel_status.order_events), 0);
  // ...
  EXPECT_EQ(obj.server_time, 1627712298468ms);
}

TEST(json_cancel_all_orders, cancelled) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("receivedTime":"2021-07-31T06:20:10.487Z",)"
                 R"("cancelOnly":"all",)"
                 R"("status":"cancelled",)"
                 R"("cancelledOrders":[)"
                 R"({)"
                 R"("cliOrdId":"ewAF6QMAAQAAly0J6JkQ",)"
                 R"("order_id":"74f63887-f69c-491e-937d-3e53d038c806")"
                 R"(})"
                 R"(],)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("uid":"74f63887-f69c-491e-937d-3e53d038c806",)"
                 R"("order":{)"
                 R"("orderId":"74f63887-f69c-491e-937d-3e53d038c806",)"
                 R"("cliOrdId":"ewAF6QMAAQAAly0J6JkQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":41753.5,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-31T06:20:08.997Z",)"
                 R"("lastUpdateTimestamp":"2021-07-31T06:20:08.997Z")"
                 R"(})"
                 R"(,"type":"CANCEL"})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-07-31T06:20:10.488Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CancelAllOrders>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.cancel_status.received_time, 1627712410487ms);
  EXPECT_EQ(obj.cancel_status.cancel_only, "all"_sv);
  EXPECT_EQ(obj.cancel_status.status, json::Status::CANCELLED);
  // EXPECT_EQ(std::size(obj.cancel_status.cancelled_orders), 0);
  EXPECT_EQ(std::size(obj.cancel_status.order_events), 1);
  // idx 0
  auto &event = obj.cancel_status.order_events[0];
  EXPECT_EQ(event.uid, "74f63887-f69c-491e-937d-3e53d038c806"_sv);
  EXPECT_EQ(event.order.order_id, "74f63887-f69c-491e-937d-3e53d038c806"_sv);
  EXPECT_EQ(event.order.cli_ord_id, "ewAF6QMAAQAAly0J6JkQ"_sv);
  EXPECT_EQ(event.order.type, json::OrderEventOrderType::LMT);
  EXPECT_EQ(event.order.symbol, "pi_xbtusd"_sv);
  EXPECT_EQ(event.order.side, json::Side::BUY);
  EXPECT_DOUBLE_EQ(event.order.quantity, 1.0);
  EXPECT_DOUBLE_EQ(event.order.filled, 0.0);
  EXPECT_DOUBLE_EQ(event.order.limit_price, 41753.5);
  EXPECT_EQ(event.order.reduce_only, false);
  EXPECT_EQ(event.order.timestamp, 1627712408997ms);
  EXPECT_EQ(event.order.last_update_timestamp, 1627712408997ms);
  EXPECT_EQ(event.type, json::OrderEventType::CANCEL);
  // ...
  EXPECT_EQ(obj.cancel_status.received_time, 1627712410487ms);
}

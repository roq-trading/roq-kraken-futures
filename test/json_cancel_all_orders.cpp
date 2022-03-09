/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/buffer.hpp"

#include "roq/core/json/buffer.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/cancel_all_orders.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_cancel_all_orders_no_orders", "[json_cancel_all_orders]") {
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
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.cancel_status.received_time == 1627712298468ms);
  CHECK(obj.cancel_status.cancel_only == "all"sv);
  CHECK(obj.cancel_status.status == json::Status::NO_ORDERS_TO_CANCEL);
  // CHECK(std::size(obj.cancel_status.cancelled_orders) == 0);
  CHECK(std::size(obj.cancel_status.order_events) == 0);
  // ...
  CHECK(obj.server_time == 1627712298468ms);
}

TEST_CASE("json_cancel_all_orders_cancelled", "[json_cancel_all_orders]") {
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
                 R"(},)"
                 R"("type":"CANCEL"})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-07-31T06:20:10.488Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::CancelAllOrders>(message, buffer_);
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.cancel_status.received_time == 1627712410487ms);
  CHECK(obj.cancel_status.cancel_only == "all"sv);
  CHECK(obj.cancel_status.status == json::Status::CANCELLED);
  // CHECK(std::size(obj.cancel_status.cancelled_orders) == 0);
  CHECK(std::size(obj.cancel_status.order_events) == 1);
  // idx 0
  auto &event = obj.cancel_status.order_events[0];
  CHECK(event.uid == "74f63887-f69c-491e-937d-3e53d038c806"sv);
  CHECK(event.order.order_id == "74f63887-f69c-491e-937d-3e53d038c806"sv);
  CHECK(event.order.cli_ord_id == "ewAF6QMAAQAAly0J6JkQ"sv);
  CHECK(event.order.type == json::OrderEventOrderType::LMT);
  CHECK(event.order.symbol == "pi_xbtusd"sv);
  CHECK(event.order.side == json::Side::BUY);
  CHECK(event.order.quantity == 1.0_a);
  CHECK(event.order.filled == 0.0_a);
  CHECK(event.order.limit_price == 41753.5_a);
  CHECK(event.order.reduce_only == false);
  CHECK(event.order.timestamp == 1627712408997ms);
  CHECK(event.order.last_update_timestamp == 1627712408997ms);
  CHECK(event.type == json::OrderEventType::CANCEL);
  // ...
  CHECK(obj.cancel_status.received_time == 1627712410487ms);
}

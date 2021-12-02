/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/send_order.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_send_order, simple) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("sendStatus":{)"
                 R"("order_id":"f2af600b-5fe8-49be-8983-de874071563b",)"
                 R"("cliOrdId":"TgAF6QMAAQAAbl1bG4QQ",)"
                 R"("status":"placed",)"
                 R"("receivedTime":"2021-07-30T04:19:40.804Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("order":{)"
                 R"("orderId":"f2af600b-5fe8-49be-8983-de874071563b",)"
                 R"("cliOrdId":"TgAF6QMAAQAAbl1bG4QQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":40166,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-30T04:19:40.804Z",)"
                 R"("lastUpdateTimestamp":"2021-07-30T04:19:40.804Z")"
                 R"(},)"
                 R"("reducedQuantity":null,)"
                 R"("type":"PLACE")"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-07-30T04:19:40.804Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::SendOrder>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.send_status.order_id, "f2af600b-5fe8-49be-8983-de874071563b"sv);
  EXPECT_EQ(obj.send_status.cli_ord_id, "TgAF6QMAAQAAbl1bG4QQ"sv);
  EXPECT_EQ(obj.send_status.status, json::Status::PLACED);
  EXPECT_EQ(obj.send_status.received_time, 1627618780804ms);
  EXPECT_EQ(std::size(obj.send_status.order_events), 1);
  // idx 0
  auto &order_event_0 = obj.send_status.order_events[0];
  EXPECT_EQ(order_event_0.order.order_id, "f2af600b-5fe8-49be-8983-de874071563b"sv);
  EXPECT_EQ(order_event_0.order.cli_ord_id, "TgAF6QMAAQAAbl1bG4QQ"sv);
  EXPECT_EQ(order_event_0.order.type, json::OrderEventOrderType::LMT);
  EXPECT_EQ(order_event_0.order.symbol, "pi_xbtusd"sv);
  EXPECT_EQ(order_event_0.order.side, json::Side::BUY);
  EXPECT_DOUBLE_EQ(order_event_0.order.quantity, 1.0);
  EXPECT_DOUBLE_EQ(order_event_0.order.filled, 0.0);
  EXPECT_DOUBLE_EQ(order_event_0.order.limit_price, 40166.0);
  EXPECT_EQ(order_event_0.order.reduce_only, false);
  EXPECT_EQ(order_event_0.order.timestamp, 1627618780804ms);
  EXPECT_EQ(order_event_0.order.last_update_timestamp, 1627618780804ms);
  EXPECT_DOUBLE_EQ(order_event_0.reduced_quantity, 0.0);
  EXPECT_EQ(order_event_0.type, json::OrderEventType::PLACE);
  EXPECT_EQ(obj.server_time, 1627618780804ms);
}

TEST(json_send_order, order_prior_execution) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("sendStatus":{)"
                 R"("order_id":"9d97b7ba-4d2e-439a-97ac-a59dea6f1eff",)"
                 R"("cliOrdId":"WwAF6QMAAQAAPOEH7dUQ",)"
                 R"("status":"placed",)"
                 R"("receivedTime":"2021-08-03T05:56:30.892Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("executionId":"685571f7-4ba2-4e6d-92f5-0c3c9428ac3f",)"
                 R"("price":38539.5,)"
                 R"("amount":1,)"
                 R"("orderPriorEdit":null,)"
                 R"("orderPriorExecution":{)"
                 R"("orderId":"9d97b7ba-4d2e-439a-97ac-a59dea6f1eff",)"
                 R"("cliOrdId":"WwAF6QMAAQAAPOEH7dUQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":38541,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-08-03T05:56:30.892Z",)"
                 R"("lastUpdateTimestamp":"2021-08-03T05:56:30.892Z")"
                 R"(},)"
                 R"("takerReducedQuantity":null,)"
                 R"("type":"EXECUTION")"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-08-03T05:56:30.992Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::SendOrder>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.send_status.order_id, "9d97b7ba-4d2e-439a-97ac-a59dea6f1eff"sv);
  EXPECT_EQ(obj.send_status.cli_ord_id, "WwAF6QMAAQAAPOEH7dUQ"sv);
  EXPECT_EQ(obj.send_status.status, json::Status::PLACED);
  EXPECT_EQ(obj.send_status.received_time, 1627970190892ms);
  EXPECT_EQ(std::size(obj.send_status.order_events), 1);
  // idx 0
  auto &order_event_0 = obj.send_status.order_events[0];
  EXPECT_EQ(order_event_0.execution_id, "685571f7-4ba2-4e6d-92f5-0c3c9428ac3f"sv);
  EXPECT_DOUBLE_EQ(order_event_0.price, 38539.5);
  EXPECT_DOUBLE_EQ(order_event_0.amount, 1.0);
  // EXPECT_EQ(order_event_0.order_prior_edit,
  EXPECT_EQ(order_event_0.order_prior_execution.order_id, "9d97b7ba-4d2e-439a-97ac-a59dea6f1eff"sv);
  EXPECT_EQ(order_event_0.order_prior_execution.cli_ord_id, "WwAF6QMAAQAAPOEH7dUQ"sv);
  EXPECT_EQ(order_event_0.order_prior_execution.type, json::OrderEventOrderType::LMT);
  EXPECT_EQ(order_event_0.order_prior_execution.symbol, "pi_xbtusd"sv);
  EXPECT_EQ(order_event_0.order_prior_execution.side, json::Side::BUY);
  EXPECT_DOUBLE_EQ(order_event_0.order_prior_execution.quantity, 1.0);
  EXPECT_DOUBLE_EQ(order_event_0.order_prior_execution.filled, 0.0);
  EXPECT_DOUBLE_EQ(order_event_0.order_prior_execution.limit_price, 38541.0);
  EXPECT_EQ(order_event_0.order_prior_execution.reduce_only, false);
  EXPECT_EQ(order_event_0.order_prior_execution.timestamp, 1627970190892ms);
  EXPECT_EQ(order_event_0.order_prior_execution.last_update_timestamp, 1627970190892ms);
  // EXPECT_DOUBLE_EQ(order_event_0.takerReducedQuantity, 0.0);
  EXPECT_EQ(order_event_0.type, json::OrderEventType::EXECUTION);
  EXPECT_EQ(obj.server_time, 1627970190992ms);
}

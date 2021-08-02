/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/send_order.h"

using namespace roq;
using namespace roq::kraken_futures;

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
  EXPECT_EQ(obj.send_status.order_id, "f2af600b-5fe8-49be-8983-de874071563b"_sv);
  EXPECT_EQ(obj.send_status.cli_ord_id, "TgAF6QMAAQAAbl1bG4QQ"_sv);
  EXPECT_EQ(obj.send_status.status, json::Status::PLACED);
  EXPECT_EQ(obj.send_status.received_time, 1627618780804ms);
  EXPECT_EQ(std::size(obj.send_status.order_events), 1);
  // idx 0
  auto &event = obj.send_status.order_events[0];
  EXPECT_EQ(event.order.order_id, "f2af600b-5fe8-49be-8983-de874071563b"_sv);
  EXPECT_EQ(event.order.cli_ord_id, "TgAF6QMAAQAAbl1bG4QQ"_sv);
  EXPECT_EQ(event.order.type, json::OrderEventOrderType::LMT);
  EXPECT_EQ(event.order.symbol, "pi_xbtusd"_sv);
  EXPECT_EQ(event.order.side, json::Side::BUY);
  EXPECT_DOUBLE_EQ(event.order.quantity, 1.0);
  EXPECT_DOUBLE_EQ(event.order.filled, 0.0);
  EXPECT_DOUBLE_EQ(event.order.limit_price, 40166.0);
  EXPECT_EQ(event.order.reduce_only, false);
  EXPECT_EQ(event.order.timestamp, 1627618780804ms);
  EXPECT_EQ(event.order.last_update_timestamp, 1627618780804ms);
  EXPECT_DOUBLE_EQ(event.reduced_quantity, 0.0);
  EXPECT_EQ(event.type, json::OrderEventType::PLACE);
  EXPECT_EQ(obj.server_time, 1627618780804ms);
}

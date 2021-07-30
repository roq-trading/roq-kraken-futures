/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/edit_order.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

TEST(json_edit_order, simple) {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("serverTime":"2021-07-30T12:36:59.235Z",)"
                 R"("editStatus":{)"
                 R"("status":"edited",)"
                 R"("orderId":"018eb846-5962-430e-af9f-31ee03cf1460",)"
                 R"("receivedTime":"2021-07-30T12:36:59.235Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("old":{)"
                 R"("orderId":"018eb846-5962-430e-af9f-31ee03cf1460",)"
                 R"("cliOrdId":"2AAF6QMAAQAAHugQDIsQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":39033,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-30T12:36:29.044Z",)"
                 R"("lastUpdateTimestamp":"2021-07-30T12:36:29.044Z")"
                 R"(},)"
                 R"("new":{)"
                 R"("orderId":"018eb846-5962-430e-af9f-31ee03cf1460",)"
                 R"("cliOrdId":"2AAF6QMAAQAAHugQDIsQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":38981.5,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-30T12:36:29.044Z",)"
                 R"("lastUpdateTimestamp":"2021-07-30T12:36:59.124Z")"
                 R"(},)"
                 R"("reducedQuantity":null,)"
                 R"("type":"EDIT")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::EditOrder>(message, buffer_);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.server_time, 1627648619235ms);
  EXPECT_EQ(obj.edit_status.status, json::Status::EDITED);
  EXPECT_EQ(obj.edit_status.order_id, "018eb846-5962-430e-af9f-31ee03cf1460"_sv);
  EXPECT_EQ(obj.edit_status.received_time, 1627648619235ms);
  EXPECT_EQ(std::size(obj.edit_status.order_events), 1);
  auto &old_event = obj.edit_status.order_events[0].old;
  EXPECT_EQ(old_event.order_id, "018eb846-5962-430e-af9f-31ee03cf1460"_sv);
  EXPECT_DOUBLE_EQ(old_event.limit_price, 39033.0);
  auto &new_event = obj.edit_status.order_events[0].new_;
  EXPECT_EQ(new_event.order_id, "018eb846-5962-430e-af9f-31ee03cf1460"_sv);
  // ...
  EXPECT_DOUBLE_EQ(new_event.limit_price, 38981.5);
}

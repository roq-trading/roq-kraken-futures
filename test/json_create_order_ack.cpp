/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

// #include "roq/kraken_futures/json/create_order_ack.h"

using namespace roq;
// using namespace roq::kraken_futures;

using namespace std::chrono_literals;

/*
TEST(json_create_order_ack, error_404) {
  auto message =
      R"({)"
      R"("timestamp":1627618268981,)"
      R"("path":"/derivatives/api/v3/sendorderorderType=lmt&symbol=PI_XBTUSD&side=buy&size=1&limitPrice=40153&stopPrice=nan&cliOrdId=swAF6QMAAQAAEb7a/IMQ&reduceOnly=false",)"
      R"("status":404,)"
      R"("error":"Not Found",)"
      R"("message":"",)"
      R"("requestId":"7ad2fe97-69108954")"
      R"(})";
  auto obj = core::json::Parser::create<json::Error>(message);
  EXPECT_EQ(obj.timestamp, 1627618268981ms);
  EXPECT_EQ(obj.status, 404);
  EXPECT_EQ(obj.error, "Not Found"_sv);
  EXPECT_EQ(obj.message, ""_sv);
  EXPECT_EQ(obj.request_id, "7ad2fe97-69108954"_sv);
}
*/

/*
TEST(json_create_order_ack, simple) {
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
  auto obj = core::json::Parser::create<json::CreateOrderAck>(message);
  EXPECT_EQ(obj.result, json::Result::SUCCESS);
  EXPECT_EQ(obj.send_status.order_id, "f2af600b-5fe8-49be-8983-de874071563b"_sv);
  EXPECT_EQ(obj.send_status.cli_ord_id, "TgAF6QMAAQAAbl1bG4QQ"_sv);
  EXPECT_EQ(obj.send_status.status, "placed"_sv);
  EXPECT_EQ(obj.send_status.received_time, 1627611580804ms);
  EXPECT_EQ(std::size(obj.send_status.order_events), 1);
  // XXX order_events
  // XXX reduced_quantity
  EXPECT_EQ(obj.send_status.type, "PLACE"_sv);
  EXPECT_EQ(obj.server_time, 1627611580804ms);
}
*/

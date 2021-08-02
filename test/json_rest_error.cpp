/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/rest_error.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

TEST(json_rest_error, error_400) {
  auto message = R"({)"
                 R"("status":"BAD_REQUEST",)"
                 R"("result":"error",)"
                 R"("errors":[)"
                 R"({)"
                 R"("code":11,)"
                 R"("message":"Argument invalid: orderType")"
                 R"(})"
                 R"(],)"
                 R"("serverTime":"2021-08-02T08:06:27.896Z")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::RestError>(message, buffer_);
  // EXPECT_EQ(obj.status, "BAD_REQUEST"_sv);
  EXPECT_EQ(std::size(obj.errors), 1);
  // idx 0
  auto &error_0 = obj.errors[0];
  EXPECT_EQ(error_0.code, 11);
  EXPECT_EQ(error_0.message, "Argument invalid: orderType"_sv);
  // ...
  EXPECT_EQ(obj.server_time, 1627891587896ms);
}

TEST(json_rest_error, error_404) {
  auto message =
      R"({)"
      R"("timestamp":1627618268981,)"
      R"("path":"/derivatives/api/v3/sendorderorderType=lmt&symbol=PI_XBTUSD&side=buy&size=1&limitPrice=40153&stopPrice=nan&cliOrdId=swAF6QMAAQAAEb7a/IMQ&reduceOnly=false",)"
      R"("status":404,)"
      R"("error":"Not Found",)"
      R"("message":"",)"
      R"("requestId":"7ad2fe97-69108954")"
      R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::RestError>(message, buffer_);
  EXPECT_EQ(obj.timestamp, 1627618268981ms);
  // EXPECT_EQ(obj.status, 404);
  EXPECT_EQ(obj.error, "Not Found"_sv);
  EXPECT_EQ(obj.message, ""_sv);
  EXPECT_EQ(obj.request_id, "7ad2fe97-69108954"_sv);
}

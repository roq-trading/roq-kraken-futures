/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/rest_error.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

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
  auto obj = core::json::Parser::create<json::RestError>(message);
  EXPECT_EQ(obj.timestamp, 1627618268981ms);
  EXPECT_EQ(obj.status, 404);
  EXPECT_EQ(obj.error, "Not Found"_sv);
  EXPECT_EQ(obj.message, ""_sv);
  EXPECT_EQ(obj.request_id, "7ad2fe97-69108954"_sv);
}

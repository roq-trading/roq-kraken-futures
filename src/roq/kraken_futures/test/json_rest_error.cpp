/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kraken_futures/json/rest_error.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_rest_error_error_400", "[json_rest_error]") {
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
  std::vector<std::byte> buffer(8192);
  auto obj = json::RestError::create(message, buffer);
  // CHECK(obj.status == "BAD_REQUEST"sv);
  CHECK(std::size(obj.errors) == 1);
  // idx 0
  auto &error_0 = obj.errors[0];
  CHECK(error_0.code == 11);
  CHECK(error_0.message == "Argument invalid: orderType"sv);
  // ...
  CHECK(obj.server_time == 1627891587896ms);
}

TEST_CASE("json_rest_error_error_404", "[json_rest_error]") {
  auto message =
      R"({)"
      R"("timestamp":1627618268981,)"
      R"("path":"/derivatives/api/v3/sendorderorderType=lmt&symbol=PI_XBTUSD&side=buy&size=1&limitPrice=40153&stopPrice=nan&cliOrdId=swAF6QMAAQAAEb7a/IMQ&reduceOnly=false",)"
      R"("status":404,)"
      R"("error":"Not Found",)"
      R"("message":"",)"
      R"("requestId":"7ad2fe97-69108954")"
      R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::RestError::create(message, buffer);
  CHECK(obj.timestamp == 1627618268981ms);
  // CHECK(obj.status == 404);
  CHECK(obj.error == "Not Found"sv);
  CHECK(obj.message == ""sv);
  CHECK(obj.request_id == "7ad2fe97-69108954"sv);
}

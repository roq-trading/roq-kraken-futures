/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kraken_futures/json/fills.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_fills_simple", "[json_fills]") {
  auto message = R"({)"
                 R"("feed":"fills",)"
                 R"("username":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
                 R"("fills":[)"
                 R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627968170746,)"
                 R"("price":38508.5,)"
                 R"("seq":5,)"
                 R"("buy":true,)"
                 R"("qty":1.0,)"
                 R"("order_id":"df2fa719-23bc-4cd7-8a84-5c3c41d75757",)"
                 R"("cli_ord_id":"EwAF7gMAAQAAnNBFcdUQ",)"
                 R"("fill_id":"21851c83-c490-40d0-8219-15f0c0c7a3cb",)"
                 R"("fill_type":"maker",)"
                 R"("fee_paid":5.2e-9,)"
                 R"("fee_currency":"BTC")"
                 R"(})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::Fills obj{message, buffer};
  CHECK(obj.feed == json::Feed::FILLS);
  CHECK(obj.username == "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  CHECK(std::size(obj.fills) == 1);
  // idx 0
  auto &fill_0 = obj.fills[0];
  CHECK(fill_0.instrument == "PI_XBTUSD"sv);
  CHECK(fill_0.time == 1627968170746ms);
  CHECK(fill_0.price == 38508.5_a);
  CHECK(fill_0.seq == 5);
  CHECK(fill_0.buy == true);
  CHECK(fill_0.qty == 1.0_a);
  CHECK(fill_0.order_id == "df2fa719-23bc-4cd7-8a84-5c3c41d75757"sv);
  CHECK(fill_0.cli_ord_id == "EwAF7gMAAQAAnNBFcdUQ"sv);
  CHECK(fill_0.fill_id == "21851c83-c490-40d0-8219-15f0c0c7a3cb"sv);
  CHECK(fill_0.fill_type == json::FillType::MAKER);
  CHECK(fill_0.fee_paid == 5.2e-9_a);
  CHECK(fill_0.fee_currency == "BTC"sv);
}

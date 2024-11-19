/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kraken_futures/json/trade_snapshot.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trade_snapshot_simple", "[json_trade_snapshot]") {
  auto message =
      R"({)"
      R"("feed":"trade_snapshot",)"
      R"("product_id":"PI_XBTUSD",)"
      R"("trades":[)"
      R"({"feed":"trade","product_id":"PI_XBTUSD","uid":"55ac6115-7709-46cd-b90c-74b932cb2cb7","side":"buy","type":"fill","seq":6922,"time":1627483320015,"qty":10.0,"price":39941.5},)"
      R"({"feed":"trade","product_id":"PI_XBTUSD","uid":"77573277-22ae-45a5-af1c-40706348627d","side":"sell","type":"fill","seq":6921,"time":1627483260064,"qty":10.0,"price":39938.0},)"
      R"({"feed":"trade","product_id":"PI_XBTUSD","uid":"25ec0c98-676b-4522-a94e-f4a848f0ed84","side":"sell","type":"fill","seq":6920,"time":1627483200036,"qty":10.0,"price":39963.5})"
      R"(])"
      R"(})";
  std::vector<std::byte> buffer(8192);
  json::TradeSnapshot obj{message, buffer};
  CHECK(obj.feed == json::Feed::TRADE_SNAPSHOT);
  CHECK(obj.product_id == "PI_XBTUSD"sv);
  CHECK(std::size(obj.trades) == 3);
}

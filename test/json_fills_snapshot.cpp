/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/buffer.hpp"

#include "roq/core/json/buffer.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/fills_snapshot.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_fills_snapshot_simple", "[json_fills_snapshot]") {
  auto message =
      R"({)"
      R"("feed":"fills_snapshot",)"
      R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
      R"("fills":[)"
      R"({"instrument":"PI_XBTUSD","time":1627876320033,"price":40065.0,"seq":0,"buy":true,"qty":1.0,"order_id":"f109eb54-a223-4503-99c5-00f053b9411e","cli_ord_id":"egAF6gMAAQAAyEmOD8AQ","fill_id":"8a5a6c01-926b-4ddd-9017-084df7ca4510","fill_type":"maker","fee_paid":5e-9,"fee_currency":"BTC"},)"
      R"({"instrument":"PI_XBTUSD","time":1627876380033,"price":40066.5,"seq":1,"buy":false,"qty":1.0,"order_id":"1ba2d300-6f26-4fd6-b573-7050cfbff08d","fill_id":"86551968-4417-46b8-9e36-26b05642ecaa","fill_type":"maker","fee_paid":5e-9,"fee_currency":"BTC"},)"
      R"({"instrument":"PI_XBTUSD","time":1627880760678,"price":40012.5,"seq":2,"buy":true,"qty":1.0,"order_id":"ff0fd378-9657-4110-90d2-f4b58e6b676e","cli_ord_id":"6wAF6gMAAQAAxsqEGMEQ","fill_id":"bb5190bb-62b6-4ecc-be49-e64651bcbc76","fill_type":"maker","fee_paid":5e-9,"fee_currency":"BTC"},)"
      R"({"instrument":"PI_XBTUSD","time":1627883000688,"price":40066.0,"seq":3,"buy":false,"qty":1.0,"order_id":"ec04e863-d534-4f2b-b27b-5427b65eb5d2","fill_id":"03ff37fa-0d8c-41a0-b30d-35ce1e5503ce","fill_type":"maker","fee_paid":5e-9,"fee_currency":"BTC"},)"
      R"({"instrument":"PI_XBTUSD","time":1627966660729,"price":38660.5,"seq":4,"buy":true,"qty":1.0,"order_id":"de5d0b8d-2977-4a50-8276-13464fc5c031","cli_ord_id":"rAAF6QMAAQAAKr9PGtUQ","fill_id":"938f8e66-1698-4d6e-9707-e629a79f9c07","fill_type":"maker","fee_paid":5.18e-9,"fee_currency":"BTC"})"
      R"(])"
      R"(})";
  core::Buffer buffer(16384);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::FillsSnapshot>(message, buffer_);
  CHECK(obj.feed == json::Feed::FILLS_SNAPSHOT);
  CHECK(obj.account == "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  CHECK(std::size(obj.fills) == 5);
}

/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/instruments_item.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_instruments_item_simple", "[json_instruments_item]") {
  auto message = R"({)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("type":"futures_inverse",)"
                 R"("underlying":"rr_xbtusd",)"
                 R"("tickSize":0.5,)"
                 R"("contractSize":1,)"
                 R"("tradeable":true,)"
                 R"("marginLevels":[)"
                 R"({"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01})"
                 R"(],)"
                 R"("fundingRateCoefficient":8,)"
                 R"("maxRelativeFundingRate":0.001,)"
                 R"("retailMarginLevels":[)"
                 R"({"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01})"
                 R"(])"
                 R"(})";
  json::InstrumentsItem obj{message};
  CHECK(obj.symbol == "pi_xbtusd"sv);
  CHECK(obj.type == json::InstrumentType::FUTURES_INVERSE);
  CHECK(obj.underlying == "rr_xbtusd"sv);
  CHECK(obj.tick_size == 0.5_a);
  CHECK(obj.contract_size == 1_a);
  CHECK(obj.tradeable == true);
  CHECK(obj.funding_rate_coefficient == 8.0_a);
  CHECK(obj.max_relative_funding_rate == 0.001_a);
}

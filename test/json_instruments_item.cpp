/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/instruments_item.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;

TEST(json_instruments_item, simple) {
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
  auto obj = core::json::Parser::create<json::InstrumentsItem>(message);
  EXPECT_EQ(obj.symbol, "pi_xbtusd"sv);
  EXPECT_EQ(obj.type, "futures_inverse"sv);
  EXPECT_EQ(obj.underlying, "rr_xbtusd"sv);
  EXPECT_DOUBLE_EQ(obj.tick_size, 0.5);
  EXPECT_DOUBLE_EQ(obj.contract_size, 1);
  EXPECT_EQ(obj.tradeable, true);
  EXPECT_DOUBLE_EQ(obj.funding_rate_coefficient, 8.0);
  EXPECT_DOUBLE_EQ(obj.max_relative_funding_rate, 0.001);
}

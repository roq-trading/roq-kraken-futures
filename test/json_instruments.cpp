/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/buffer.hpp"

#include "roq/core/json/buffer.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/instruments.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_instruments_simple", "[json_instruments]") {
  auto message =
      R"({)"
      R"("result":"success",)"
      R"("instruments":[)"
      R"({"symbol":"pi_xbtusd","type":"futures_inverse","underlying":"rr_xbtusd","tickSize":0.5,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"fundingRateCoefficient":8,"maxRelativeFundingRate":0.001,"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"pi_ethusd","type":"futures_inverse","underlying":"rr_ethusd","tickSize":0.025,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"fundingRateCoefficient":8,"maxRelativeFundingRate":0.001,"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"pi_xrpusd","type":"futures_inverse","underlying":"rr_xrpusd","tickSize":0.0001,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"fundingRateCoefficient":8,"maxRelativeFundingRate":0.001,"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"pi_ltcusd","type":"futures_inverse","underlying":"rr_ltcusd","tickSize":0.01,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"fundingRateCoefficient":8,"maxRelativeFundingRate":0.001,"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"pi_bchusd","type":"futures_inverse","underlying":"rr_bchusd","tickSize":0.1,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"fundingRateCoefficient":8,"maxRelativeFundingRate":0.001,"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_ethusd_210924","type":"futures_inverse","underlying":"rr_ethusd","lastTradingTime":"2021-09-24T15:00:00.000Z","tickSize":1,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_xbtusd_210924","type":"futures_inverse","underlying":"rr_xbtusd","lastTradingTime":"2021-09-24T15:00:00.000Z","tickSize":1,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_bchusd_210924","type":"futures_inverse","underlying":"rr_bchusd","lastTradingTime":"2021-09-24T15:00:00.000Z","tickSize":0.1,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_xrpusd_210924","type":"futures_inverse","underlying":"rr_xrpusd","lastTradingTime":"2021-09-24T15:00:00.000Z","tickSize":0.0001,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_ltcusd_210924","type":"futures_inverse","underlying":"rr_ltcusd","lastTradingTime":"2021-09-24T15:00:00.000Z","tickSize":0.01,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_ltcusd_210730","type":"futures_inverse","underlying":"rr_ltcusd","lastTradingTime":"2021-07-30T15:00:00.000Z","tickSize":0.01,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_xrpusd_210730","type":"futures_inverse","underlying":"rr_xrpusd","lastTradingTime":"2021-07-30T15:00:00.000Z","tickSize":0.0001,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_xbtusd_210730","type":"futures_inverse","underlying":"rr_xbtusd","lastTradingTime":"2021-07-30T15:00:00.000Z","tickSize":0.5,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_ethusd_210730","type":"futures_inverse","underlying":"rr_ethusd","lastTradingTime":"2021-07-30T15:00:00.000Z","tickSize":0.025,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"fi_bchusd_210730","type":"futures_inverse","underlying":"rr_bchusd","lastTradingTime":"2021-07-30T15:00:00.000Z","tickSize":0.1,"contractSize":1,"tradeable":true,"marginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}],"retailMarginLevels":[{"contracts":0,"initialMargin":0.02,"maintenanceMargin":0.01}]},)"
      R"({"symbol":"in_xbtusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"in_xrpusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"in_ethusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"in_ltcusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"in_bchusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"rr_xbtusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"rr_xrpusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"rr_ethusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"rr_ltcusd","type":"spot index","tradeable":false},)"
      R"({"symbol":"rr_bchusd","type":"spot index","tradeable":false})"
      R"(],)"
      R"("serverTime":"2021-07-28T05:32:46.371Z")"
      R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::Instruments>(message, buffer_);
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(std::size(obj.instruments) == 25);
}

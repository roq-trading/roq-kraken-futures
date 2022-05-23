/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/ticker.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_ticker_simple", "[json_ticker]") {
  auto message = R"({)"
                 R"("time":1627460663568,)"
                 R"("feed":"ticker",)"
                 R"("product_id":"FI_ETHUSD_210730",)"
                 R"("bid":2312.15,)"
                 R"("ask":2313.3,)"
                 R"("bid_size":5000.0,)"
                 R"("ask_size":5000.0,)"
                 R"("volume":14400.0,)"
                 R"("dtm":2,)"
                 R"("leverage":"50x",)"
                 R"("index":2306.28,)"
                 R"("premium":0.3,)"
                 R"("last":2313.05,)"
                 R"("change":4.008723413822568,)"
                 R"("suspended":false,)"
                 R"("tag":"month",)"
                 R"("pair":"ETH:USD",)"
                 R"("openInterest":38089.0,)"
                 R"("markPrice":2312.725,)"
                 R"("maturityTime":1627657200000)"
                 R"(})";
  auto obj = core::json::Parser::create<json::Ticker>(message);
  CHECK(obj.time == 1627460663568ms);
  CHECK(obj.feed == json::Feed::TICKER);
  CHECK(obj.product_id == "FI_ETHUSD_210730"sv);
  CHECK(obj.bid == 2312.15_a);
  CHECK(obj.ask == 2313.3_a);
  CHECK(obj.bid_size == 5000.0_a);
  CHECK(obj.ask_size == 5000.0_a);
  CHECK(obj.volume == 14400.0_a);
  CHECK(obj.dtm == 2.0_a);
  CHECK(obj.leverage == "50x"sv);
  CHECK(obj.index == 2306.28_a);
  CHECK(obj.premium == 0.3_a);
  CHECK(obj.last == 2313.05_a);
  CHECK(obj.change == 4.008723413822568_a);
  CHECK(obj.suspended == false);
  CHECK(obj.tag == json::Tag::MONTH);
  CHECK(obj.pair == "ETH:USD"sv);
  CHECK(obj.open_interest == 38089.0_a);
  CHECK(obj.mark_price == 2312.725_a);
  CHECK(obj.maturity_time == 1627657200000ms);
}

TEST_CASE("json_ticker_funding_rates", "[json_ticker]") {
  auto message = R"({)"
                 R"("time":1627462526224,)"
                 R"("feed":"ticker",)"
                 R"("product_id":"PI_XBTUSD",)"
                 R"("bid":39780.5,)"
                 R"("ask":39800.5,)"
                 R"("bid_size":5000.0,)"
                 R"("ask_size":5000.0,)"
                 R"("volume":1168580.0,)"
                 R"("dtm":0,)"
                 R"("leverage":"50x",)"
                 R"("index":39511.15,)"
                 R"("premium":0.7,)"
                 R"("last":39804.0,)"
                 R"("change":5.89408994772338,)"
                 R"("funding_rate":2.1914353639e-8,)"
                 R"("funding_rate_prediction":2.2099858377e-8,)"
                 R"("suspended":false,)"
                 R"("tag":"perpetual",)"
                 R"("pair":"XBT:USD",)"
                 R"("openInterest":32093438.0,)"
                 R"("markPrice":39790.5,)"
                 R"("maturityTime":0,)"
                 R"("relative_funding_rate":0.000874516826041667,)"
                 R"("relative_funding_rate_prediction":0.000874329625,)"
                 R"("next_funding_rate_time":1627473600000)"
                 R"(})";
  auto obj = core::json::Parser::create<json::Ticker>(message);
  CHECK(obj.time == 1627462526224ms);
  CHECK(obj.feed == json::Feed::TICKER);
  CHECK(obj.product_id == "PI_XBTUSD"sv);
  CHECK(obj.bid == 39780.5_a);
  CHECK(obj.ask == 39800.5_a);
  CHECK(obj.bid_size == 5000.0_a);
  CHECK(obj.ask_size == 5000.0_a);
  CHECK(obj.volume == 1168580.0_a);
  CHECK(obj.dtm == 0.0_a);
  CHECK(obj.leverage == "50x"sv);
  CHECK(obj.index == 39511.15_a);
  CHECK(obj.premium == 0.7_a);
  CHECK(obj.last == 39804.0_a);
  CHECK(obj.change == 5.89408994772338_a);
  CHECK(obj.funding_rate == 2.1914353639e-8_a);
  CHECK(obj.funding_rate_prediction == 2.2099858377e-8_a);
  CHECK(obj.suspended == false);
  CHECK(obj.tag == json::Tag::PERPETUAL);
  CHECK(obj.pair == "XBT:USD"sv);
  CHECK(obj.open_interest == 32093438.0_a);
  CHECK(obj.mark_price == 39790.5_a);
  CHECK(obj.maturity_time == std::chrono::milliseconds{});
  CHECK(obj.relative_funding_rate == 0.000874516826041667_a);
  CHECK(obj.relative_funding_rate_prediction == 0.000874329625_a);
  CHECK(obj.next_funding_rate_time == 1627473600000ms);
}

/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/ticker.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_ticker, simple) {
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
  EXPECT_EQ(obj.time, 1627460663568ms);
  EXPECT_EQ(obj.feed, json::Feed::TICKER);
  EXPECT_EQ(obj.product_id, "FI_ETHUSD_210730"sv);
  EXPECT_DOUBLE_EQ(obj.bid, 2312.15);
  EXPECT_DOUBLE_EQ(obj.ask, 2313.3);
  EXPECT_DOUBLE_EQ(obj.bid_size, 5000.0);
  EXPECT_DOUBLE_EQ(obj.ask_size, 5000.0);
  EXPECT_DOUBLE_EQ(obj.volume, 14400.0);
  EXPECT_DOUBLE_EQ(obj.dtm, 2.0);
  EXPECT_EQ(obj.leverage, "50x"sv);
  EXPECT_DOUBLE_EQ(obj.index, 2306.28);
  EXPECT_DOUBLE_EQ(obj.premium, 0.3);
  EXPECT_DOUBLE_EQ(obj.last, 2313.05);
  EXPECT_DOUBLE_EQ(obj.change, 4.008723413822568);
  EXPECT_EQ(obj.suspended, false);
  EXPECT_EQ(obj.tag, json::Tag::MONTH);
  EXPECT_EQ(obj.pair, "ETH:USD"sv);
  EXPECT_DOUBLE_EQ(obj.open_interest, 38089.0);
  EXPECT_DOUBLE_EQ(obj.mark_price, 2312.725);
  EXPECT_EQ(obj.maturity_time, 1627657200000ms);
}

TEST(json_ticker, funding_rates) {
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
  EXPECT_EQ(obj.time, 1627462526224ms);
  EXPECT_EQ(obj.feed, json::Feed::TICKER);
  EXPECT_EQ(obj.product_id, "PI_XBTUSD"sv);
  EXPECT_DOUBLE_EQ(obj.bid, 39780.5);
  EXPECT_DOUBLE_EQ(obj.ask, 39800.5);
  EXPECT_DOUBLE_EQ(obj.bid_size, 5000.0);
  EXPECT_DOUBLE_EQ(obj.ask_size, 5000.0);
  EXPECT_DOUBLE_EQ(obj.volume, 1168580.0);
  EXPECT_DOUBLE_EQ(obj.dtm, 0.0);
  EXPECT_EQ(obj.leverage, "50x"sv);
  EXPECT_DOUBLE_EQ(obj.index, 39511.15);
  EXPECT_DOUBLE_EQ(obj.premium, 0.7);
  EXPECT_DOUBLE_EQ(obj.last, 39804.0);
  EXPECT_DOUBLE_EQ(obj.change, 5.89408994772338);
  EXPECT_DOUBLE_EQ(obj.funding_rate, 2.1914353639e-8);
  EXPECT_DOUBLE_EQ(obj.funding_rate_prediction, 2.2099858377e-8);
  EXPECT_EQ(obj.suspended, false);
  EXPECT_EQ(obj.tag, json::Tag::PERPETUAL);
  EXPECT_EQ(obj.pair, "XBT:USD"sv);
  EXPECT_DOUBLE_EQ(obj.open_interest, 32093438.0);
  EXPECT_DOUBLE_EQ(obj.mark_price, 39790.5);
  EXPECT_EQ(obj.maturity_time, std::chrono::milliseconds{});
  EXPECT_DOUBLE_EQ(obj.relative_funding_rate, 0.000874516826041667);
  EXPECT_DOUBLE_EQ(obj.relative_funding_rate_prediction, 0.000874329625);
  EXPECT_EQ(obj.next_funding_rate_time, 1627473600000ms);
}

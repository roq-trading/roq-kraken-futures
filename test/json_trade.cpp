/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"
#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/trade.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_trade_simple", "[json_trade]") {
  auto message = R"({)"
                 R"("feed":"trade",)"
                 R"("product_id":"PI_LTCUSD",)"
                 R"("uid":"bee759b2-c264-4a9e-a8a6-07b60556786d",)"
                 R"("side":"buy",)"
                 R"("type":"fill",)"
                 R"("seq":3732,)"
                 R"("time":1627483389597,)"
                 R"("qty":1.0,)"
                 R"("price":136.53)"
                 R"(})";
  json::Trade obj{message};
  CHECK(obj.feed == json::Feed::TRADE);
  CHECK(obj.product_id == "PI_LTCUSD"sv);
  CHECK(obj.uid == "bee759b2-c264-4a9e-a8a6-07b60556786d"sv);
  CHECK(obj.side == json::Side::BUY);
  CHECK(obj.type == json::TradeType::FILL);
  CHECK(obj.seq == 3732);
  CHECK(obj.time == 1627483389597ms);
  CHECK(obj.qty == 1.0_a);
  CHECK(obj.price == 136.53_a);
}

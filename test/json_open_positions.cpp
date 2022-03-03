/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/open_positions.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_open_positions_simple_1", "json_open_positions") {
  auto message = R"({)"
                 R"("feed":"open_positions",)"
                 R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
                 R"("positions":[)"
                 R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("balance":1.0,)"
                 R"("pnl":6.244534934262959e-9,)"
                 R"("entry_price":40012.5,)"
                 R"("mark_price":40022.5,)"
                 R"("index_price":39766.96,)"
                 R"("liquidation_threshold":2.522396666144259,)"
                 R"("effective_leverage":0.00006246486479409476,)"
                 R"("return_on_equity":0.012496094970326962)"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenPositions>(message, buffer_);
  CHECK(obj.feed == json::Feed::OPEN_POSITIONS);
  CHECK(obj.account == "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  CHECK(std::size(obj.positions) == 1);
  // idx 0
  auto &position_0 = obj.positions[0];
  CHECK(position_0.instrument == "PI_XBTUSD"sv);
  CHECK(position_0.balance == 1.0_a);
  CHECK(position_0.pnl == 6.244534934262959e-9_a);
  CHECK(position_0.entry_price == 40012.5_a);
  CHECK(position_0.mark_price == 40022.5_a);
  CHECK(position_0.index_price == 39766.96_a);
  CHECK(position_0.liquidation_threshold == 2.522396666144259_a);
  CHECK(position_0.effective_leverage == 0.00006246486479409476_a);
  CHECK(position_0.return_on_equity == 0.012496094970326962_a);
}

TEST_CASE("json_open_positions_simple_2", "json_open_positions") {
  auto message = R"({)"
                 R"("feed":"open_positions",)"
                 R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
                 R"("positions":[)"
                 R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("balance":1.0,)"
                 R"("pnl":-3.126955263139583e-8,)"
                 R"("entry_price":40012.5,)"
                 R"("mark_price":39962.5,)"
                 R"("index_price":39683.17,)"
                 R"("liquidation_threshold":2.5248423786767753,)"
                 R"("effective_leverage":0.00006255865689491961,)"
                 R"("return_on_equity":-0.06248047485160779,)"
                 R"("unrealized_funding":-6.4769646600617945e-9)"
                 R"(})"
                 R"(])"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenPositions>(message, buffer_);
  CHECK(obj.feed == json::Feed::OPEN_POSITIONS);
  CHECK(obj.account == "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  CHECK(std::size(obj.positions) == 1);
  // idx 0
  auto &position_0 = obj.positions[0];
  CHECK(position_0.instrument == "PI_XBTUSD"sv);
  CHECK(position_0.balance == 1.0_a);
  CHECK(position_0.pnl == -3.126955263139583e-8_a);
  CHECK(position_0.entry_price == 40012.5_a);
  CHECK(position_0.mark_price == 39962.5_a);
  CHECK(position_0.index_price == 39683.17_a);
  CHECK(position_0.liquidation_threshold == 2.5248423786767753_a);
  CHECK(position_0.effective_leverage == 0.00006255865689491961_a);
  CHECK(position_0.return_on_equity == -0.06248047485160779_a);
  CHECK(position_0.unrealized_funding == -6.4769646600617945e-9_a);
}

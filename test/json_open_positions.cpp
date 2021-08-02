/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/open_positions.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

TEST(json_open_positions, simple) {
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
}

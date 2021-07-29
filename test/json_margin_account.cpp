/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/margin_account.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::chrono_literals;

TEST(json_margin_account, simple) {
  auto message = R"({)"
                 R"("name":"bch",)"
                 R"("balance":20.0,)"
                 R"("pnl":0.0,)"
                 R"("funding":0.0,)"
                 R"("pv":0.0,)"
                 R"("am":0.0,)"
                 R"("im":0.0,)"
                 R"("mm":0.0)"
                 R"(})";
  auto obj = core::json::Parser::create<json::MarginAccount>(message);
  EXPECT_EQ(obj.name, "bch"_sv);
  EXPECT_DOUBLE_EQ(obj.balance, 20.0);
  EXPECT_DOUBLE_EQ(obj.pnl, 0.0);
  EXPECT_DOUBLE_EQ(obj.funding, 0.0);
  EXPECT_DOUBLE_EQ(obj.pv, 0.0);
  EXPECT_DOUBLE_EQ(obj.am, 0.0);
  EXPECT_DOUBLE_EQ(obj.im, 0.0);
  EXPECT_DOUBLE_EQ(obj.mm, 0.0);
}

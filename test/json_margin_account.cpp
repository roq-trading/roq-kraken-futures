/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/protocol/json/margin_account.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_margin_account_simple", "[json_margin_account]") {
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
  protocol::json::MarginAccount obj{message};
  CHECK(obj.name == "bch"sv);
  CHECK(obj.balance == 20.0_a);
  CHECK(obj.pnl == 0.0_a);
  CHECK(obj.funding == 0.0_a);
  CHECK(obj.pv == 0.0_a);
  CHECK(obj.am == 0.0_a);
  CHECK(obj.im == 0.0_a);
  CHECK(obj.mm == 0.0_a);
}

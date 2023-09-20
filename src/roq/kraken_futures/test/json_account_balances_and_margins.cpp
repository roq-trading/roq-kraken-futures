/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kraken_futures/json/account_balances_and_margins.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;

using namespace Catch::literals;

TEST_CASE("json_account_balances_and_margins_simple", "[json_account_balances_and_margins]") {
  auto message = R"({)"
                 R"("feed":"account_balances_and_margins",)"
                 R"("account":"bdb7a134-386a-45c0-b8e5-76a75537df4c",)"
                 R"("margin_accounts":[)"
                 R"({"name":"bch","balance":20.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"xrp","balance":3000.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"eth","balance":10.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"ltc","balance":20.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"xbt","balance":0.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"f-xrp:usd","balance":0.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"f-eth:usd","balance":0.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"f-xbt:usd","balance":0.4,"pnl":0.0,"funding":0.0,"pv":0.4,"am":0.4,"im":0.0,"mm":0.0},)"
                 R"({"name":"f-ltc:usd","balance":0.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"f-xrp:xbt","balance":0.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0},)"
                 R"({"name":"f-bch:usd","balance":0.0,"pnl":0.0,"funding":0.0,"pv":0.0,"am":0.0,"im":0.0,"mm":0.0})"
                 R"(],)"
                 R"("seq":0)"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::AccountBalancesAndMargins::create(message, buffer);
  CHECK(obj.feed == json::Feed::ACCOUNT_BALANCES_AND_MARGINS);
  CHECK(obj.account == "bdb7a134-386a-45c0-b8e5-76a75537df4c"sv);
  CHECK(std::size(obj.margin_accounts) == 11);
  CHECK(obj.seq == 0);
}

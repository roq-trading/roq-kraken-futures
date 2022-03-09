/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include <catch2/catch.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/order.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_order_simple", "[json_order]") {
  auto message = R"({)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627577572583,)"
                 R"("last_update_time":1627577572583,)"
                 R"("qty":1.0,)"
                 R"("filled":0.0,)"
                 R"("limit_price":39528.0,)"
                 R"("stop_price":0.0,)"
                 R"("type":"limit",)"
                 R"("order_id":"494f7cb0-6936-495f-a0c5-663ad9b9fbdd",)"
                 R"("direction":0,)"
                 R"("reduce_only":false)"
                 R"(})";
  auto obj = core::json::Parser::create<json::Order>(message);
  CHECK(obj.instrument == "PI_XBTUSD"sv);
  CHECK(obj.time == 1627577572583ms);
  CHECK(obj.last_update_time == 1627577572583ms);
  CHECK(obj.qty == 1.0_a);
  CHECK(obj.filled == 0.0_a);
  CHECK(obj.limit_price == 39528.0_a);
  CHECK(obj.stop_price == 0.0_a);
  CHECK(obj.type == json::OrderType::LIMIT);
  CHECK(obj.order_id == "494f7cb0-6936-495f-a0c5-663ad9b9fbdd"sv);
  CHECK(obj.direction == 0);
  CHECK(obj.reduce_only == false);
}

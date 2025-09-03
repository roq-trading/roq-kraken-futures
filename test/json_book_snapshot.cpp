/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kraken_futures/json/book_snapshot.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_book_snapshot_simple", "[json_book_snapshot]") {
  auto message = R"({)"
                 R"("feed":"book_snapshot",)"
                 R"("product_id":"FI_LTCUSD_210924",)"
                 R"("timestamp":1627535620727,)"
                 R"("seq":732887,)"
                 R"("tickSize":null,)"
                 R"("bids":[)"
                 R"({"price":139.54,"qty":5000.0},)"
                 R"({"price":139.53,"qty":5000.0},)"
                 R"({"price":139.52,"qty":5000.0},)"
                 R"({"price":139.51,"qty":5000.0},)"
                 R"({"price":139.5,"qty":5000.0})"
                 R"(],)"
                 R"("asks":[)"
                 R"({"price":139.61,"qty":5000.0},)"
                 R"({"price":139.62,"qty":5000.0},)"
                 R"({"price":139.63,"qty":5000.0},)"
                 R"({"price":139.64,"qty":5000.0},)"
                 R"({"price":139.65,"qty":5000.0})"
                 R"(])"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::BookSnapshot obj{message, buffer};
  CHECK(obj.feed == json::Feed::BOOK_SNAPSHOT);
  CHECK(obj.product_id == "FI_LTCUSD_210924"sv);
  CHECK(obj.timestamp == 1627535620727ms);
  CHECK(obj.seq == 732887);
  // tick_size
  CHECK(std::size(obj.bids) == 5);
  CHECK(std::size(obj.asks) == 5);
}

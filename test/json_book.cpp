/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/parser.hpp"

#include "roq/kraken_futures/json/book.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_book_simple", "[json_book]") {
  auto message = R"({)"
                 R"("feed":"book",)"
                 R"("product_id":"PI_XBTUSD",)"
                 R"("side":"buy",)"
                 R"("seq":1766422,)"
                 R"("price":39826.0,)"
                 R"("qty":3951.0,)"
                 R"("timestamp":1627535639684)"
                 R"(})";
  json::Book obj{message};
  CHECK(obj.feed == json::Feed::BOOK);
  CHECK(obj.product_id == "PI_XBTUSD"sv);
  CHECK(obj.side == json::Side::BUY);
  CHECK(obj.seq == 1766422);
  CHECK(obj.price == 39826.0_a);
  CHECK(obj.qty == 3951.0_a);
  CHECK(obj.timestamp == 1627535639684ms);
}

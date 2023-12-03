/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kraken_futures/json/open_orders.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_open_orders_new_placed_order_by_user", "[json_open_orders]") {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order":{)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627578394576,)"
                 R"("last_update_time":1627578394576,)"
                 R"("qty":1.0,)"
                 R"("filled":0.0,)"
                 R"("limit_price":39547.0,)"
                 R"("stop_price":0.0,)"
                 R"("type":"limit",)"
                 R"("order_id":"85d803ca-da36-4231-846a-fd3979770d67",)"
                 R"("direction":0,)"
                 R"("reduce_only":false)"
                 R"(},)"
                 R"("is_cancel":false,)"
                 R"("reason":"new_placed_order_by_user")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::OpenOrders::create(message, buffer);
  CHECK(obj.feed == json::Feed::OPEN_ORDERS);
  CHECK(obj.order.instrument == "PI_XBTUSD"sv);
  CHECK(obj.order.time == 1627578394576ms);
  CHECK(obj.order.last_update_time == 1627578394576ms);
  CHECK(obj.order.qty == 1.0_a);
  CHECK(obj.order.filled == 0.0_a);
  CHECK(obj.order.limit_price == 39547.0_a);
  CHECK(obj.order.stop_price == 0.0_a);
  CHECK(obj.order.type == json::OrderType::LIMIT);
  CHECK(obj.order.order_id == "85d803ca-da36-4231-846a-fd3979770d67"sv);
  CHECK(obj.order.direction == 0);
  CHECK(obj.order.reduce_only == false);
  CHECK(obj.is_cancel == false);
  CHECK(obj.reason == json::Reason::NEW_PLACED_ORDER_BY_USER);
}

TEST_CASE("json_open_orders_new_placed_order_by_user_with_cli_ord_id", "[json_open_orders]") {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order":{)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627647670603,)"
                 R"("last_update_time":1627647670603,)"
                 R"("qty":1.0,)"
                 R"("filled":0.0,)"
                 R"("limit_price":39057.5,)"
                 R"("stop_price":0.0,)"
                 R"("type":"limit",)"
                 R"("order_id":"6f360089-1837-4551-a8d4-f6b648b11992",)"
                 R"("cli_ord_id":"UAAF6QMAAQAAWr5S1YoQ",)"
                 R"("direction":0,)"
                 R"("reduce_only":false)"
                 R"(},)"
                 R"("is_cancel":false,)"
                 R"("reason":"new_placed_order_by_user")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::OpenOrders::create(message, buffer);
  CHECK(obj.feed == json::Feed::OPEN_ORDERS);
  CHECK(obj.order.instrument == "PI_XBTUSD"sv);
  CHECK(obj.order.time == 1627647670603ms);
  CHECK(obj.order.last_update_time == 1627647670603ms);
  CHECK(obj.order.qty == 1.0_a);
  CHECK(obj.order.filled == 0.0_a);
  CHECK(obj.order.limit_price == 39057.5_a);
  CHECK(obj.order.stop_price == 0.0_a);
  CHECK(obj.order.type == json::OrderType::LIMIT);
  CHECK(obj.order.order_id == "6f360089-1837-4551-a8d4-f6b648b11992"sv);
  CHECK(obj.order.cli_ord_id == "UAAF6QMAAQAAWr5S1YoQ"sv);
  CHECK(obj.order.direction == 0);
  CHECK(obj.order.reduce_only == false);
  CHECK(obj.is_cancel == false);
  CHECK(obj.reason == json::Reason::NEW_PLACED_ORDER_BY_USER);
}

TEST_CASE("json_open_orders_edited_by_user", "[json_open_orders]") {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order":{)"
                 R"("instrument":"PI_XBTUSD",)"
                 R"("time":1627906165185,)"
                 R"("last_update_time":1627906170258,)"
                 R"("qty":1.0,)"
                 R"("filled":0.0,)"
                 R"("limit_price":39726.5,)"
                 R"("stop_price":0.0,)"
                 R"("type":"limit",)"
                 R"("order_id":"8a9a24e8-7d90-4b3c-9838-0474d06bfd40",)"
                 R"("cli_ord_id":"hwAF6wMAAQAAPg/OBMcQ",)"
                 R"("direction":0,)"
                 R"("reduce_only":false)"
                 R"(},)"
                 R"("is_cancel":false,)"
                 R"("reason":"edited_by_user")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::OpenOrders::create(message, buffer);
  CHECK(obj.feed == json::Feed::OPEN_ORDERS);
  CHECK(obj.order.instrument == "PI_XBTUSD"sv);
  CHECK(obj.order.time == 1627906165185ms);
  CHECK(obj.order.last_update_time == 1627906170258ms);
  CHECK(obj.order.qty == 1.0_a);
  CHECK(obj.order.limit_price == 39726.5_a);
  CHECK(obj.order.stop_price == 0.0_a);
  CHECK(obj.order.type == json::OrderType::LIMIT);
  CHECK(obj.order.order_id == "8a9a24e8-7d90-4b3c-9838-0474d06bfd40"sv);
  CHECK(obj.order.cli_ord_id == "hwAF6wMAAQAAPg/OBMcQ"sv);
  CHECK(obj.order.direction == 0);
  CHECK(obj.order.reduce_only == false);
  CHECK(obj.is_cancel == false);
  CHECK(obj.reason == json::Reason::EDITED_BY_USER);
}

TEST_CASE("json_open_orders_cancelled_by_user", "[json_open_orders]") {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order_id":"494f7cb0-6936-495f-a0c5-663ad9b9fbdd",)"
                 R"("is_cancel":true,)"
                 R"("reason":"cancelled_by_user")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::OpenOrders::create(message, buffer);
  CHECK(obj.feed == json::Feed::OPEN_ORDERS);
  CHECK(obj.order_id == "494f7cb0-6936-495f-a0c5-663ad9b9fbdd"sv);
  CHECK(obj.is_cancel == true);
  CHECK(obj.reason == json::Reason::CANCELLED_BY_USER);
}

TEST_CASE("json_open_orders_cancelled_by_user_with_cli_ord_id", "[json_open_orders]") {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order_id":"f18e006d-c95e-4d89-b470-4402949d5a15",)"
                 R"("cli_ord_id":"QAAF6QMAAQAA7DBK5YoQ",)"
                 R"("is_cancel":true,)"
                 R"("reason":"cancelled_by_user")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::OpenOrders::create(message, buffer);
  CHECK(obj.feed == json::Feed::OPEN_ORDERS);
  CHECK(obj.order_id == "f18e006d-c95e-4d89-b470-4402949d5a15"sv);
  CHECK(obj.cli_ord_id == "QAAF6QMAAQAA7DBK5YoQ"sv);
  CHECK(obj.is_cancel == true);
  CHECK(obj.reason == json::Reason::CANCELLED_BY_USER);
}

TEST_CASE("json_open_orders_full_fill", "[json_open_orders]") {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order_id":"df2fa719-23bc-4cd7-8a84-5c3c41d75757",)"
                 R"("cli_ord_id":"EwAF7gMAAQAAnNBFcdUQ",)"
                 R"("is_cancel":true,)"
                 R"("reason":"full_fill")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::OpenOrders::create(message, buffer);
  CHECK(obj.feed == json::Feed::OPEN_ORDERS);
  CHECK(obj.order_id == "df2fa719-23bc-4cd7-8a84-5c3c41d75757"sv);
  CHECK(obj.cli_ord_id == "EwAF7gMAAQAAnNBFcdUQ"sv);
  CHECK(obj.is_cancel == true);  // note!
  CHECK(obj.reason == json::Reason::FULL_FILL);
}

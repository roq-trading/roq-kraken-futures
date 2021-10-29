/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/core/buffer.h"

#include "roq/core/json/buffer.h"
#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/open_orders.h"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

TEST(json_open_orders, new_placed_order_by_user) {
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS);
  EXPECT_EQ(obj.order.instrument, "PI_XBTUSD"sv);
  EXPECT_EQ(obj.order.time, 1627578394576ms);
  EXPECT_EQ(obj.order.last_update_time, 1627578394576ms);
  EXPECT_DOUBLE_EQ(obj.order.qty, 1.0);
  EXPECT_DOUBLE_EQ(obj.order.filled, 0.0);
  EXPECT_DOUBLE_EQ(obj.order.limit_price, 39547.0);
  EXPECT_DOUBLE_EQ(obj.order.stop_price, 0.0);
  EXPECT_EQ(obj.order.type, json::OrderType::LIMIT);
  EXPECT_EQ(obj.order.order_id, "85d803ca-da36-4231-846a-fd3979770d67"sv);
  EXPECT_EQ(obj.order.direction, 0);
  EXPECT_EQ(obj.order.reduce_only, false);
  EXPECT_EQ(obj.is_cancel, false);
  EXPECT_EQ(obj.reason, json::Reason::NEW_PLACED_ORDER_BY_USER);
}

TEST(json_open_orders, new_placed_order_by_user_with_cli_ord_id) {
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS);
  EXPECT_EQ(obj.order.instrument, "PI_XBTUSD"sv);
  EXPECT_EQ(obj.order.time, 1627647670603ms);
  EXPECT_EQ(obj.order.last_update_time, 1627647670603ms);
  EXPECT_DOUBLE_EQ(obj.order.qty, 1.0);
  EXPECT_DOUBLE_EQ(obj.order.filled, 0.0);
  EXPECT_DOUBLE_EQ(obj.order.limit_price, 39057.5);
  EXPECT_DOUBLE_EQ(obj.order.stop_price, 0.0);
  EXPECT_EQ(obj.order.type, json::OrderType::LIMIT);
  EXPECT_EQ(obj.order.order_id, "6f360089-1837-4551-a8d4-f6b648b11992"sv);
  EXPECT_EQ(obj.order.cli_ord_id, "UAAF6QMAAQAAWr5S1YoQ"sv);
  EXPECT_EQ(obj.order.direction, 0);
  EXPECT_EQ(obj.order.reduce_only, false);
  EXPECT_EQ(obj.is_cancel, false);
  EXPECT_EQ(obj.reason, json::Reason::NEW_PLACED_ORDER_BY_USER);
}

TEST(json_open_orders, edited_by_user) {
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
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS);
  EXPECT_EQ(obj.order.instrument, "PI_XBTUSD"sv);
  EXPECT_EQ(obj.order.time, 1627906165185ms);
  EXPECT_EQ(obj.order.last_update_time, 1627906170258ms);
  EXPECT_DOUBLE_EQ(obj.order.qty, 1.0);
  EXPECT_DOUBLE_EQ(obj.order.limit_price, 39726.5);
  EXPECT_DOUBLE_EQ(obj.order.stop_price, 0.0);
  EXPECT_DOUBLE_EQ(obj.order.type, json::OrderType::LIMIT);
  EXPECT_EQ(obj.order.order_id, "8a9a24e8-7d90-4b3c-9838-0474d06bfd40"sv);
  EXPECT_EQ(obj.order.cli_ord_id, "hwAF6wMAAQAAPg/OBMcQ"sv);
  EXPECT_EQ(obj.order.direction, 0);
  EXPECT_EQ(obj.order.reduce_only, false);
  EXPECT_EQ(obj.is_cancel, false);
  EXPECT_EQ(obj.reason, json::Reason::EDITED_BY_USER);
}

TEST(json_open_orders, cancelled_by_user) {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order_id":"494f7cb0-6936-495f-a0c5-663ad9b9fbdd",)"
                 R"("is_cancel":true,)"
                 R"("reason":"cancelled_by_user")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS);
  EXPECT_EQ(obj.order_id, "494f7cb0-6936-495f-a0c5-663ad9b9fbdd"sv);
  EXPECT_EQ(obj.is_cancel, true);
  EXPECT_EQ(obj.reason, json::Reason::CANCELLED_BY_USER);
}

TEST(json_open_orders, cancelled_by_user_with_cli_ord_id) {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order_id":"f18e006d-c95e-4d89-b470-4402949d5a15",)"
                 R"("cli_ord_id":"QAAF6QMAAQAA7DBK5YoQ",)"
                 R"("is_cancel":true,)"
                 R"("reason":"cancelled_by_user")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS);
  EXPECT_EQ(obj.order_id, "f18e006d-c95e-4d89-b470-4402949d5a15"sv);
  EXPECT_EQ(obj.cli_ord_id, "QAAF6QMAAQAA7DBK5YoQ"sv);
  EXPECT_EQ(obj.is_cancel, true);
  EXPECT_EQ(obj.reason, json::Reason::CANCELLED_BY_USER);
}

TEST(json_open_orders, full_fill) {
  auto message = R"({)"
                 R"("feed":"open_orders",)"
                 R"("order_id":"df2fa719-23bc-4cd7-8a84-5c3c41d75757",)"
                 R"("cli_ord_id":"EwAF7gMAAQAAnNBFcdUQ",)"
                 R"("is_cancel":true,)"
                 R"("reason":"full_fill")"
                 R"(})";
  core::Buffer buffer(8192);
  core::json::Buffer buffer_(buffer);
  auto obj = core::json::Parser::create<json::OpenOrders>(message, buffer_);
  EXPECT_EQ(obj.feed, json::Feed::OPEN_ORDERS);
  EXPECT_EQ(obj.order_id, "df2fa719-23bc-4cd7-8a84-5c3c41d75757"sv);
  EXPECT_EQ(obj.cli_ord_id, "EwAF7gMAAQAAnNBFcdUQ"sv);
  EXPECT_EQ(obj.is_cancel, true);  // note!
  EXPECT_EQ(obj.reason, json::Reason::FULL_FILL);
}

/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/kraken_futures/json/cancel_order.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_cancel_order_simple", "[json_cancel_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("status":"cancelled",)"
                 R"("order_id":"85792364-8163-4e13-b62d-695e7f802e22",)"
                 R"("receivedTime":"2021-07-31T04:53:14.376Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("uid":"85792364-8163-4e13-b62d-695e7f802e22",)"
                 R"("order":{)"
                 R"("orderId":"85792364-8163-4e13-b62d-695e7f802e22",)"
                 R"("cliOrdId":"DwAF6QMAAQAAxMacsJgQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":41936,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-31T04:53:04.171Z",)"
                 R"("lastUpdateTimestamp":"2021-07-31T04:53:09.310Z")"
                 R"(},)"
                 R"("type":"CANCEL")"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-07-31T04:53:14.376Z")"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::CancelOrder::create(message, buffer);
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.cancel_status.status == json::Status::CANCELLED);
  CHECK(obj.cancel_status.order_id == "85792364-8163-4e13-b62d-695e7f802e22"sv);
  CHECK(obj.cancel_status.received_time == 1627707194376ms);
  CHECK(std::size(obj.cancel_status.order_events) == 1);
  // idx 0
  auto &event = obj.cancel_status.order_events[0];
  CHECK(event.uid == "85792364-8163-4e13-b62d-695e7f802e22"sv);
  CHECK(event.order.order_id == "85792364-8163-4e13-b62d-695e7f802e22"sv);
  CHECK(event.order.cli_ord_id == "DwAF6QMAAQAAxMacsJgQ"sv);
  CHECK(event.order.type == json::OrderEventOrderType::LMT);
  CHECK(event.order.symbol == "pi_xbtusd"sv);
  CHECK(event.order.side == json::Side::BUY);
  CHECK(event.order.quantity == 1.0_a);
  CHECK(event.order.filled == 0.0_a);
  CHECK(event.order.limit_price == 41936.0_a);
  CHECK(event.order.reduce_only == false);
  CHECK(event.order.timestamp == 1627707184171ms);
  CHECK(event.order.last_update_timestamp == 1627707189310ms);
  // ...
  CHECK(event.type == json::OrderEventType::CANCEL);
  // ...
  CHECK(obj.server_time == 1627707194376ms);
}

TEST_CASE("json_cancel_order_not_found", "[json_cancel_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("cancelStatus":{)"
                 R"("status":"notFound",)"
                 R"("receivedTime":"2021-08-02T07:46:05.900Z")"
                 R"(},)"
                 R"("serverTime":"2021-08-02T07:46:05.900Z")"
                 R"(})"
                 R"(})";
  std::vector<std::byte> buffer(8192);
  auto obj = json::CancelOrder::create(message, buffer);
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.cancel_status.status == json::Status::NOT_FOUND);
  CHECK(obj.cancel_status.received_time == 1627890365900ms);
  CHECK(obj.server_time == 1627890365900ms);
}

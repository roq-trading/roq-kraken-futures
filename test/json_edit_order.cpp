/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kraken_futures/json/edit_order.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_edit_order_simple", "[json_edit_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("serverTime":"2021-07-30T12:36:59.235Z",)"
                 R"("editStatus":{)"
                 R"("status":"edited",)"
                 R"("orderId":"018eb846-5962-430e-af9f-31ee03cf1460",)"
                 R"("receivedTime":"2021-07-30T12:36:59.235Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("old":{)"
                 R"("orderId":"018eb846-5962-430e-af9f-31ee03cf1460",)"
                 R"("cliOrdId":"2AAF6QMAAQAAHugQDIsQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":39033,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-30T12:36:29.044Z",)"
                 R"("lastUpdateTimestamp":"2021-07-30T12:36:29.044Z")"
                 R"(},)"
                 R"("new":{)"
                 R"("orderId":"018eb846-5962-430e-af9f-31ee03cf1460",)"
                 R"("cliOrdId":"2AAF6QMAAQAAHugQDIsQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":38981.5,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-30T12:36:29.044Z",)"
                 R"("lastUpdateTimestamp":"2021-07-30T12:36:59.124Z")"
                 R"(},)"
                 R"("reducedQuantity":null,)"
                 R"("type":"EDIT")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::EditOrder obj{message, buffer};
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.server_time == 1627648619235ms);
  CHECK(obj.edit_status.status == json::Status::EDITED);
  CHECK(obj.edit_status.order_id == "018eb846-5962-430e-af9f-31ee03cf1460"sv);
  CHECK(obj.edit_status.received_time == 1627648619235ms);
  CHECK(std::size(obj.edit_status.order_events) == 1);
  // idx 0
  auto &event = obj.edit_status.order_events[0];
  // ... old
  CHECK(event.old.order_id == "018eb846-5962-430e-af9f-31ee03cf1460"sv);
  CHECK(event.old.cli_ord_id == "2AAF6QMAAQAAHugQDIsQ"sv);
  CHECK(event.old.type == json::OrderEventOrderType::LMT);
  CHECK(event.old.symbol == "pi_xbtusd"sv);
  CHECK(event.old.side == json::Side::BUY);
  CHECK(event.old.quantity == 1.0_a);
  CHECK(event.old.filled == 0.0_a);
  CHECK(event.old.limit_price == 39033.0_a);
  CHECK(event.old.timestamp == 1627648589044ms);
  CHECK(event.old.last_update_timestamp == 1627648589044ms);
  // ... new
  CHECK(event.new_.order_id == "018eb846-5962-430e-af9f-31ee03cf1460"sv);
  CHECK(event.new_.cli_ord_id == "2AAF6QMAAQAAHugQDIsQ"sv);
  CHECK(event.new_.type == json::OrderEventOrderType::LMT);
  CHECK(event.new_.symbol == "pi_xbtusd"sv);
  CHECK(event.new_.side == json::Side::BUY);
  CHECK(event.new_.quantity == 1.0_a);
  CHECK(event.new_.filled == 0.0_a);
  CHECK(event.new_.limit_price == 38981.5_a);
  CHECK(event.new_.timestamp == 1627648589044ms);
  CHECK(event.new_.last_update_timestamp == 1627648619124ms);
  // ...
  CHECK(std::isnan(event.reduced_quantity) == true);
  CHECK(event.type == json::OrderEventType::EDIT);
}

TEST_CASE("json_edit_order_authentication_error", "[json_edit_order]") {
  auto message = R"({)"
                 R"("result":"error",)"
                 R"("error":"authenticationError",)"
                 R"("serverTime":"2021-07-31T04:30:20.840Z")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::EditOrder obj{message, buffer};
  CHECK(obj.result == json::Result::ERROR);
  CHECK(obj.error == "authenticationError"sv);
  CHECK(obj.server_time == 1627705820840ms);
}

TEST_CASE("json_edit_order_edit_has_no_effect", "[json_edit_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("serverTime":"2021-08-02T03:51:50.939Z",)"
                 R"("editStatus":{)"
                 R"("status":"edited",)"
                 R"("orderId":"f109eb54-a223-4503-99c5-00f053b9411e",)"
                 R"("receivedTime":"2021-08-02T03:51:50.939Z",)"
                 R"("orderEvents":[{"uid":"f109eb54-a223-4503-99c5-00f053b9411e",)"
                 R"("order":{"orderId":"f109eb54-a223-4503-99c5-00f053b9411e",)"
                 R"("cliOrdId":"egAF6gMAAQAAyEmOD8AQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":40065,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-08-02T03:51:20.856Z",)"
                 R"("lastUpdateTimestamp":"2021-08-02T03:51:20.856Z")"
                 R"(},)"
                 R"("reason":"EDIT_HAS_NO_EFFECT",)"
                 R"("type":"REJECT")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::EditOrder obj{message, buffer};
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.server_time == 1627876310939ms);
  // edit_status
  auto &edit_status = obj.edit_status;
  CHECK(edit_status.order_id == "f109eb54-a223-4503-99c5-00f053b9411e"sv);
  CHECK(edit_status.received_time == 1627876310939ms);
  CHECK(std::size(edit_status.order_events) == 1);
  // idx 0
  auto &order_event = edit_status.order_events[0];
  CHECK(order_event.uid == "f109eb54-a223-4503-99c5-00f053b9411e"sv);
  CHECK(order_event.order.order_id == "f109eb54-a223-4503-99c5-00f053b9411e"sv);
  CHECK(order_event.order.cli_ord_id == "egAF6gMAAQAAyEmOD8AQ"sv);
  CHECK(order_event.order.type == json::OrderEventOrderType::LMT);
  CHECK(order_event.order.symbol == "pi_xbtusd"sv);
  CHECK(order_event.order.side == json::Side::BUY);
  CHECK(order_event.order.quantity == 1.0_a);
  CHECK(order_event.order.filled == 0.0_a);
  CHECK(order_event.order.limit_price == 40065.0_a);
  CHECK(order_event.order.reduce_only == false);
  CHECK(order_event.order.timestamp == 1627876280856ms);
  CHECK(order_event.order.last_update_timestamp == 1627876280856ms);
  CHECK(order_event.reason == "EDIT_HAS_NO_EFFECT");
  CHECK(order_event.type == json::OrderEventType::REJECT);
}

TEST_CASE("json_edit_order_execution", "[json_edit_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("serverTime":"2021-08-03T06:42:05.376Z",)"
                 R"("editStatus":{)"
                 R"("status":"filled",)"
                 R"("orderId":"4178c9d1-b033-4113-afaf-610c97631d07",)"
                 R"("receivedTime":"2021-08-03T06:42:05.376Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("executionId":"7d484ed2-0dbe-48de-9002-45f6ac5f3a90",)"
                 R"("price":38621.0,)"
                 R"("amount":1,)"
                 R"("orderPriorEdit":{)"
                 R"("orderId":"4178c9d1-b033-4113-afaf-610c97631d07",)"
                 R"("cliOrdId":"ewAF6QMAAQAAXXO1j9YQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":38562.5,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-08-03T06:42:00.184Z",)"
                 R"("lastUpdateTimestamp":"2021-08-03T06:42:00.184Z")"
                 R"(},)"
                 R"("orderPriorExecution":{)"
                 R"("orderId":"4178c9d1-b033-4113-afaf-610c97631d07",)"
                 R"("cliOrdId":"ewAF6QMAAQAAXXO1j9YQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":38652,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-08-03T06:42:00.184Z",)"
                 R"("lastUpdateTimestamp":"2021-08-03T06:42:05.259Z")"
                 R"(},)"
                 R"("takerReducedQuantity":null,)"
                 R"("type":"EXECUTION")"
                 R"(})"
                 R"(])"
                 R"(})"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  json::EditOrder obj{message, buffer};
  CHECK(obj.result == json::Result::SUCCESS);
  CHECK(obj.server_time == 1627972925376ms);
  // edit_status
  auto &edit_status = obj.edit_status;
  CHECK(edit_status.status == json::Status::FILLED);
  CHECK(edit_status.order_id == "4178c9d1-b033-4113-afaf-610c97631d07"sv);
  CHECK(edit_status.received_time == 1627972925376ms);
  CHECK(std::size(edit_status.order_events) == 1);
  // idx 0
  auto &order_event = edit_status.order_events[0];
  CHECK(order_event.execution_id == "7d484ed2-0dbe-48de-9002-45f6ac5f3a90"sv);
  CHECK(order_event.price == 38621.0_a);
  CHECK(order_event.amount == 1.0_a);
  // ... order_prior_edit
  CHECK(order_event.order_prior_edit.order_id == "4178c9d1-b033-4113-afaf-610c97631d07"sv);
  CHECK(order_event.order_prior_edit.cli_ord_id == "ewAF6QMAAQAAXXO1j9YQ"sv);
  CHECK(order_event.order_prior_edit.type == json::OrderEventOrderType::LMT);
  CHECK(order_event.order_prior_edit.symbol == "pi_xbtusd"sv);
  CHECK(order_event.order_prior_edit.side == json::Side::BUY);
  CHECK(order_event.order_prior_edit.quantity == 1.0_a);
  CHECK(order_event.order_prior_edit.filled == 0.0_a);
  CHECK(order_event.order_prior_edit.limit_price == 38562.5_a);
  CHECK(order_event.order_prior_edit.reduce_only == false);
  CHECK(order_event.order_prior_edit.timestamp == 1627972920184ms);
  CHECK(order_event.order_prior_edit.last_update_timestamp == 1627972920184ms);
  // ... order_prior_execution
  CHECK(order_event.order_prior_execution.order_id == "4178c9d1-b033-4113-afaf-610c97631d07"sv);
  CHECK(order_event.order_prior_execution.cli_ord_id == "ewAF6QMAAQAAXXO1j9YQ"sv);
  CHECK(order_event.order_prior_execution.type == json::OrderEventOrderType::LMT);
  CHECK(order_event.order_prior_execution.symbol == "pi_xbtusd"sv);
  CHECK(order_event.order_prior_execution.side == json::Side::BUY);
  CHECK(order_event.order_prior_execution.quantity == 1.0_a);
  CHECK(order_event.order_prior_execution.filled == 0.0_a);
  CHECK(order_event.order_prior_execution.limit_price == 38652.0_a);
  CHECK(order_event.order_prior_execution.reduce_only == false);
  CHECK(order_event.order_prior_execution.timestamp == 1627972920184ms);
  CHECK(order_event.order_prior_execution.last_update_timestamp == 1627972925259ms);
  // ...
  CHECK(std::isnan(order_event.taker_reduced_quantity) == true);
  CHECK(order_event.type == json::OrderEventType::EXECUTION);
}

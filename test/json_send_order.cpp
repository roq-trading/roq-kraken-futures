/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include <catch2/catch_all.hpp>

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kraken_futures/protocol/json/send_order.hpp"

using namespace roq;
using namespace roq::kraken_futures;

using namespace std::literals;
using namespace std::chrono_literals;

using namespace Catch::literals;

TEST_CASE("json_send_order_simple", "[json_send_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("sendStatus":{)"
                 R"("order_id":"f2af600b-5fe8-49be-8983-de874071563b",)"
                 R"("cliOrdId":"TgAF6QMAAQAAbl1bG4QQ",)"
                 R"("status":"placed",)"
                 R"("receivedTime":"2021-07-30T04:19:40.804Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("order":{)"
                 R"("orderId":"f2af600b-5fe8-49be-8983-de874071563b",)"
                 R"("cliOrdId":"TgAF6QMAAQAAbl1bG4QQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":40166,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-07-30T04:19:40.804Z",)"
                 R"("lastUpdateTimestamp":"2021-07-30T04:19:40.804Z")"
                 R"(},)"
                 R"("reducedQuantity":null,)"
                 R"("type":"PLACE")"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-07-30T04:19:40.804Z")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  protocol::json::SendOrder obj{message, buffer};
  CHECK(obj.result == protocol::json::Result::SUCCESS);
  CHECK(obj.send_status.order_id == "f2af600b-5fe8-49be-8983-de874071563b"sv);
  CHECK(obj.send_status.cli_ord_id == "TgAF6QMAAQAAbl1bG4QQ"sv);
  CHECK(obj.send_status.status == protocol::json::Status::PLACED);
  CHECK(obj.send_status.received_time == 1627618780804ms);
  CHECK(std::size(obj.send_status.order_events) == 1);
  // idx 0
  auto &order_event_0 = obj.send_status.order_events[0];
  CHECK(order_event_0.order.order_id == "f2af600b-5fe8-49be-8983-de874071563b"sv);
  CHECK(order_event_0.order.cli_ord_id == "TgAF6QMAAQAAbl1bG4QQ"sv);
  CHECK(order_event_0.order.type == protocol::json::OrderEventOrderType::LMT);
  CHECK(order_event_0.order.symbol == "pi_xbtusd"sv);
  CHECK(order_event_0.order.side == protocol::json::Side::BUY);
  CHECK(order_event_0.order.quantity == 1.0_a);
  CHECK(order_event_0.order.filled == 0.0_a);
  CHECK(order_event_0.order.limit_price == 40166.0_a);
  CHECK(order_event_0.order.reduce_only == false);
  CHECK(order_event_0.order.timestamp == 1627618780804ms);
  CHECK(order_event_0.order.last_update_timestamp == 1627618780804ms);
  CHECK(std::isnan(order_event_0.reduced_quantity) == true);
  CHECK(order_event_0.type == protocol::json::OrderEventType::PLACE);
  CHECK(obj.server_time == 1627618780804ms);
}

TEST_CASE("json_send_order_order_prior_execution", "[json_send_order]") {
  auto message = R"({)"
                 R"("result":"success",)"
                 R"("sendStatus":{)"
                 R"("order_id":"9d97b7ba-4d2e-439a-97ac-a59dea6f1eff",)"
                 R"("cliOrdId":"WwAF6QMAAQAAPOEH7dUQ",)"
                 R"("status":"placed",)"
                 R"("receivedTime":"2021-08-03T05:56:30.892Z",)"
                 R"("orderEvents":[)"
                 R"({)"
                 R"("executionId":"685571f7-4ba2-4e6d-92f5-0c3c9428ac3f",)"
                 R"("price":38539.5,)"
                 R"("amount":1,)"
                 R"("orderPriorEdit":null,)"
                 R"("orderPriorExecution":{)"
                 R"("orderId":"9d97b7ba-4d2e-439a-97ac-a59dea6f1eff",)"
                 R"("cliOrdId":"WwAF6QMAAQAAPOEH7dUQ",)"
                 R"("type":"lmt",)"
                 R"("symbol":"pi_xbtusd",)"
                 R"("side":"buy",)"
                 R"("quantity":1,)"
                 R"("filled":0,)"
                 R"("limitPrice":38541,)"
                 R"("reduceOnly":false,)"
                 R"("timestamp":"2021-08-03T05:56:30.892Z",)"
                 R"("lastUpdateTimestamp":"2021-08-03T05:56:30.892Z")"
                 R"(},)"
                 R"("takerReducedQuantity":null,)"
                 R"("type":"EXECUTION")"
                 R"(})"
                 R"(])"
                 R"(},)"
                 R"("serverTime":"2021-08-03T05:56:30.992Z")"
                 R"(})";
  core::json::BufferStack buffer{8192, 1};
  protocol::json::SendOrder obj{message, buffer};
  CHECK(obj.result == protocol::json::Result::SUCCESS);
  CHECK(obj.send_status.order_id == "9d97b7ba-4d2e-439a-97ac-a59dea6f1eff"sv);
  CHECK(obj.send_status.cli_ord_id == "WwAF6QMAAQAAPOEH7dUQ"sv);
  CHECK(obj.send_status.status == protocol::json::Status::PLACED);
  CHECK(obj.send_status.received_time == 1627970190892ms);
  CHECK(std::size(obj.send_status.order_events) == 1);
  // idx 0
  auto &order_event_0 = obj.send_status.order_events[0];
  CHECK(order_event_0.execution_id == "685571f7-4ba2-4e6d-92f5-0c3c9428ac3f"sv);
  CHECK(order_event_0.price == 38539.5_a);
  CHECK(order_event_0.amount == 1.0_a);
  // EXPECT_EQ(order_event_0.order_prior_edit,
  CHECK(order_event_0.order_prior_execution.order_id == "9d97b7ba-4d2e-439a-97ac-a59dea6f1eff"sv);
  CHECK(order_event_0.order_prior_execution.cli_ord_id == "WwAF6QMAAQAAPOEH7dUQ"sv);
  CHECK(order_event_0.order_prior_execution.type == protocol::json::OrderEventOrderType::LMT);
  CHECK(order_event_0.order_prior_execution.symbol == "pi_xbtusd"sv);
  CHECK(order_event_0.order_prior_execution.side == protocol::json::Side::BUY);
  CHECK(order_event_0.order_prior_execution.quantity == 1.0_a);
  CHECK(order_event_0.order_prior_execution.filled == 0.0_a);
  CHECK(order_event_0.order_prior_execution.limit_price == 38541.0_a);
  CHECK(order_event_0.order_prior_execution.reduce_only == false);
  CHECK(order_event_0.order_prior_execution.timestamp == 1627970190892ms);
  CHECK(order_event_0.order_prior_execution.last_update_timestamp == 1627970190892ms);
  // CHECK(order_event_0.takerReducedQuantity == 0.0_a);
  CHECK(order_event_0.type == protocol::json::OrderEventType::EXECUTION);
  CHECK(obj.server_time == 1627970190992ms);
}

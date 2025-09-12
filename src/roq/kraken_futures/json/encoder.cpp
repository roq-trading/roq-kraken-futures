/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/json/encoder.hpp"

#include <fmt/format.h>

#include "roq/decimal.hpp"

#include "roq/kraken_futures/json/map.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace json {

// === HELPERS ===

namespace {
json::OrderEventOrderType compute_order_type(auto const &order_type, auto const &time_in_force, auto const &execution_instructions, auto const &stop_price) {
  if (time_in_force == TimeInForce::IOC) {
    return json::OrderEventOrderType::IOC;
  }
  switch (order_type) {
    using enum roq::OrderType;
    case UNDEFINED:
      break;
    case MARKET:
      return json::OrderEventOrderType::MKT;
    case LIMIT:
      if (std::isnan(stop_price)) {
        return json::OrderEventOrderType::LMT;
      } else {
        return json::OrderEventOrderType::STP;
      }
      break;
  }
  throw RuntimeError{
      "Unexpected combination of order_type={}, time_in_force={}, execution_instructions={}, stop_price={}"sv,
      order_type,
      time_in_force,
      execution_instructions,
      stop_price};
}
}  // namespace

// === IMPLEMENTATION ===

std::string_view Encoder::send_order(
    std::vector<char> &buffer, CreateOrder const &create_order, server::oms::Order const &order, std::string_view const &request_id) {
  auto order_type = compute_order_type(create_order.order_type, create_order.time_in_force, create_order.execution_instructions, create_order.stop_price);
  auto side = map(create_order.side).template get<json::Side>();
  auto reduce_only = create_order.execution_instructions.has(ExecutionInstruction::DO_NOT_INCREASE);
  buffer.clear();
  if (!std::isnan(create_order.price)) {
    if (std::isnan(create_order.stop_price)) {
      // limit
      fmt::format_to(
          std::back_inserter(buffer),
          "?orderType={}"
          "&symbol={}"
          "&side={}"
          "&size={}"
          "&limitPrice={}"
          "&cliOrdId={}"
          "&reduceOnly={}"sv,
          order_type.as_raw_text(),
          create_order.symbol,
          side.as_raw_text(),
          Decimal{create_order.quantity, order.quantity_precision.precision},
          Decimal{create_order.price, order.price_precision.precision},
          request_id,
          reduce_only);
    } else {
      // limit + stop
      fmt::format_to(
          std::back_inserter(buffer),
          "?orderType={}"
          "&symbol={}"
          "&side={}"
          "&size={}"
          "&limitPrice={}"
          "&stopPrice={}"
          "&cliOrdId={}"
          "&reduceOnly={}"sv,
          order_type.as_raw_text(),
          create_order.symbol,
          side.as_raw_text(),
          Decimal{create_order.quantity, order.quantity_precision.precision},
          Decimal{create_order.price, order.price_precision.precision},
          Decimal{create_order.stop_price, order.price_precision.precision},
          request_id,
          reduce_only);
    }
  } else {
    // market
    fmt::format_to(
        std::back_inserter(buffer),
        "?orderType={}"
        "&symbol={}"
        "&side={}"
        "&size={}"
        "&cliOrdId={}"
        "&reduceOnly={}"sv,
        order_type.as_raw_text(),
        create_order.symbol,
        side.as_raw_text(),
        Decimal{create_order.quantity, order.quantity_precision.precision},
        request_id,
        reduce_only);
  }
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

std::string_view Encoder::edit_order(
    std::vector<char> &buffer,
    ModifyOrder const &modify_order,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  buffer.clear();
  // note! price has max 2 decimals, size is integer
  fmt::format_to(
      std::back_inserter(buffer),
      "?orderId={}"
      "&size={}"
      "&limitPrice={}"sv,
      order.external_order_id,
      Decimal{modify_order.quantity, order.quantity_precision.precision},
      Decimal{modify_order.price, order.price_precision.precision});
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

std::string_view Encoder::cancel_order(
    std::vector<char> &buffer,
    roq::CancelOrder const &,
    server::oms::Order const &order,
    [[maybe_unused]] std::string_view const &request_id,
    [[maybe_unused]] std::string_view const &previous_request_id) {
  buffer.clear();
  // note! price has max 2 decimals, size is integer
  fmt::format_to(std::back_inserter(buffer), "?order_id={}"sv, order.external_order_id);
  std::string_view result{std::data(buffer), std::size(buffer)};
  return result;
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

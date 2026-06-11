/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include "roq/kraken_futures/protocol/json/fill_type.hpp"
#include "roq/kraken_futures/protocol/json/order_event_order_type.hpp"
#include "roq/kraken_futures/protocol/json/order_type.hpp"
#include "roq/kraken_futures/protocol/json/reason.hpp"
#include "roq/kraken_futures/protocol/json/side.hpp"
#include "roq/kraken_futures/protocol/json/status.hpp"

#include "roq/error.hpp"
#include "roq/liquidity.hpp"
#include "roq/order_status.hpp"
#include "roq/order_type.hpp"
#include "roq/side.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Liquidity> Map<kraken_futures::protocol::json::FillType>::helper() const;

template <>
template <>
std::optional<OrderType> Map<kraken_futures::protocol::json::OrderEventOrderType>::helper() const;

template <>
template <>
std::optional<OrderType> Map<kraken_futures::protocol::json::OrderType>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<kraken_futures::protocol::json::Reason>::helper() const;

template <>
template <>
std::optional<Side> Map<kraken_futures::protocol::json::Side>::helper() const;

template <>
template <>
std::optional<Error> Map<kraken_futures::protocol::json::Status>::helper() const;

template <>
template <>
std::optional<OrderStatus> Map<kraken_futures::protocol::json::Status>::helper() const;

// ===

template <>
template <>
std::optional<kraken_futures::protocol::json::Side> Map<Side>::helper() const;

}  // namespace roq

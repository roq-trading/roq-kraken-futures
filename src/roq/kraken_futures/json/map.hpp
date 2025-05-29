/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include "roq/kraken_futures/json/fill_type.hpp"
#include "roq/kraken_futures/json/order_event_order_type.hpp"
#include "roq/kraken_futures/json/order_type.hpp"
#include "roq/kraken_futures/json/side.hpp"
#include "roq/kraken_futures/json/status.hpp"

#include "roq/error.hpp"
#include "roq/liquidity.hpp"
#include "roq/order_type.hpp"
#include "roq/side.hpp"

#include "roq/map.hpp"

namespace roq {

template <>
template <>
std::optional<Liquidity> Map<kraken_futures::json::FillType>::helper() const;

template <>
template <>
std::optional<OrderType> Map<kraken_futures::json::OrderEventOrderType>::helper() const;

template <>
template <>
std::optional<OrderType> Map<kraken_futures::json::OrderType>::helper() const;

template <>
template <>
std::optional<Side> Map<kraken_futures::json::Side>::helper() const;

template <>
template <>
std::optional<Error> Map<kraken_futures::json::Status>::helper() const;

// ===

template <>
template <>
std::optional<kraken_futures::json::Side> Map<Side>::helper() const;

}  // namespace roq

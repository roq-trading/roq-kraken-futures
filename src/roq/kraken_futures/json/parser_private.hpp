/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.hpp"

#include "roq/server.hpp"

#include "roq/kraken_futures/json/alert.hpp"
#include "roq/kraken_futures/json/error.hpp"
#include "roq/kraken_futures/json/info.hpp"

#include "roq/kraken_futures/json/challenge.hpp"

#include "roq/kraken_futures/json/subscribed.hpp"

#include "roq/kraken_futures/json/heartbeat.hpp"

#include "roq/kraken_futures/json/account_balances_and_margins.hpp"
#include "roq/kraken_futures/json/open_positions.hpp"

#include "roq/kraken_futures/json/open_orders.hpp"
#include "roq/kraken_futures/json/open_orders_snapshot.hpp"

#include "roq/kraken_futures/json/fills.hpp"
#include "roq/kraken_futures/json/fills_snapshot.hpp"

namespace roq {
namespace kraken_futures {
namespace json {

struct ParserPrivate final {
  struct Handler {
    virtual void operator()(const server::Trace<Info> &) = 0;
    virtual void operator()(const server::Trace<Alert> &) = 0;
    virtual void operator()(const server::Trace<Error> &) = 0;

    virtual void operator()(const server::Trace<Challenge> &) = 0;

    virtual void operator()(const server::Trace<Subscribed> &) = 0;

    virtual void operator()(const server::Trace<Heartbeat> &) = 0;

    virtual void operator()(const server::Trace<AccountBalancesAndMargins> &) = 0;
    virtual void operator()(const server::Trace<OpenPositions> &) = 0;

    virtual void operator()(const server::Trace<OpenOrdersSnapshot> &) = 0;
    virtual void operator()(const server::Trace<OpenOrders> &) = 0;

    virtual void operator()(const server::Trace<FillsSnapshot> &) = 0;
    virtual void operator()(const server::Trace<Fills> &) = 0;
  };

  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const server::TraceInfo &trace_info);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

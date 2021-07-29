/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/kraken_futures/json/alert.h"
#include "roq/kraken_futures/json/error.h"
#include "roq/kraken_futures/json/info.h"

#include "roq/kraken_futures/json/challenge.h"

#include "roq/kraken_futures/json/subscribed.h"

#include "roq/kraken_futures/json/heartbeat.h"

#include "roq/kraken_futures/json/account_balances_and_margins.h"
#include "roq/kraken_futures/json/open_positions.h"

#include "roq/kraken_futures/json/open_orders.h"
#include "roq/kraken_futures/json/open_orders_snapshot.h"

#include "roq/kraken_futures/json/fills.h"
#include "roq/kraken_futures/json/fills_snapshot.h"

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

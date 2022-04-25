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
    virtual void operator()(const Trace<Info const> &) = 0;
    virtual void operator()(const Trace<Alert const> &) = 0;
    virtual void operator()(const Trace<Error const> &) = 0;

    virtual void operator()(const Trace<Challenge const> &) = 0;

    virtual void operator()(const Trace<Subscribed const> &) = 0;

    virtual void operator()(const Trace<Heartbeat const> &) = 0;

    virtual void operator()(const Trace<AccountBalancesAndMargins const> &) = 0;
    virtual void operator()(const Trace<OpenPositions const> &) = 0;

    virtual void operator()(const Trace<OpenOrdersSnapshot const> &) = 0;
    virtual void operator()(const Trace<OpenOrders const> &) = 0;

    virtual void operator()(const Trace<FillsSnapshot const> &) = 0;
    virtual void operator()(const Trace<Fills const> &) = 0;
  };

  static bool dispatch(
      Handler &, const std::string_view &message, core::json::Buffer &buffer, const TraceInfo &);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/kraken_futures/json/error.h"
#include "roq/kraken_futures/json/heartbeat.h"
#include "roq/kraken_futures/json/pong.h"
#include "roq/kraken_futures/json/subscription_status.h"
#include "roq/kraken_futures/json/system_status.h"

#include "roq/kraken_futures/json/add_order_status.h"
#include "roq/kraken_futures/json/cancel_order_status.h"

#include "roq/kraken_futures/json/open_orders.h"
#include "roq/kraken_futures/json/own_trades.h"

namespace roq {
namespace kraken_futures {
namespace json {

struct ParserPrivate final {
  struct Handler {
    virtual void operator()(const Error &, const server::TraceInfo &) = 0;
    virtual void operator()(const SystemStatus &, const server::TraceInfo &) = 0;
    virtual void operator()(const Pong &, const server::TraceInfo &) = 0;
    virtual void operator()(const Heartbeat &, const server::TraceInfo &) = 0;
    virtual void operator()(const SubscriptionStatus &, const server::TraceInfo &) = 0;

    virtual void operator()(const AddOrderStatus &, const server::TraceInfo &) = 0;
    virtual void operator()(const CancelOrderStatus &, const server::TraceInfo &) = 0;

    virtual void operator()(const OpenOrders &, const server::TraceInfo &) = 0;
    virtual void operator()(const OwnTrades &, const server::TraceInfo &) = 0;
  };

  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      const server::TraceInfo &trace_info);

 protected:
  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      core::json::object_t &root,
      const server::TraceInfo &trace_info);

  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      core::json::array_t &root,
      const server::TraceInfo &trace_info);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

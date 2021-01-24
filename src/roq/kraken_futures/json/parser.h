/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/json/error.h"
#include "roq/kraken_futures/json/heartbeat.h"
#include "roq/kraken_futures/json/pong.h"
#include "roq/kraken_futures/json/subscription_status.h"
#include "roq/kraken_futures/json/system_status.h"

#include "roq/kraken_futures/json/book.h"
#include "roq/kraken_futures/json/spread.h"
#include "roq/kraken_futures/json/trade.h"

namespace roq {
namespace kraken_futures {
namespace json {

struct Parser final {
  struct Handler {
    virtual void operator()(const Error &) = 0;
    virtual void operator()(const SystemStatus &) = 0;
    virtual void operator()(const Pong &) = 0;
    virtual void operator()(const Heartbeat &) = 0;
    virtual void operator()(const SubscriptionStatus &) = 0;

    virtual void operator()(const Trade &trade, const std::string_view &pair) = 0;
    virtual void operator()(const Spread &spread, const std::string_view &pair) = 0;
    virtual void operator()(const Book &book, const std::string_view &pair) = 0;
  };

  static bool dispatch(
      Handler &handler, const std::string_view &message, core::json::Buffer &buffer);

 protected:
  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      core::json::object_t &root);

  static bool dispatch(
      Handler &handler,
      const std::string_view &message,
      core::json::Buffer &buffer,
      core::json::array_t &root);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/kraken_futures/json/alert.h"
#include "roq/kraken_futures/json/info.h"
#include "roq/kraken_futures/json/subscribed.h"

#include "roq/kraken_futures/json/ticker.h"

namespace roq {
namespace kraken_futures {
namespace json {

struct ParserPublic final {
  struct Handler {
    virtual void operator()(const Info &, const server::TraceInfo &) = 0;
    virtual void operator()(const Alert &, const server::TraceInfo &) = 0;
    virtual void operator()(const Subscribed &, const server::TraceInfo &) = 0;

    virtual void operator()(const Ticker &, const server::TraceInfo &) = 0;
  };

  static bool dispatch(
      Handler &handler, const std::string_view &message, const server::TraceInfo &trace_info);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

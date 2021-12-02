/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

#include "roq/kraken_futures/json/alert.h"
#include "roq/kraken_futures/json/error.h"
#include "roq/kraken_futures/json/info.h"

#include "roq/kraken_futures/json/subscribed.h"

#include "roq/kraken_futures/json/heartbeat.h"

#include "roq/kraken_futures/json/book.h"
#include "roq/kraken_futures/json/book_snapshot.h"
#include "roq/kraken_futures/json/ticker.h"
#include "roq/kraken_futures/json/trade.h"
#include "roq/kraken_futures/json/trade_snapshot.h"

namespace roq {
namespace kraken_futures {
namespace json {

struct ParserPublic final {
  struct Handler {
    virtual void operator()(const server::Trace<Info> &) = 0;
    virtual void operator()(const server::Trace<Alert> &) = 0;
    virtual void operator()(const server::Trace<Error> &) = 0;

    virtual void operator()(const server::Trace<Subscribed> &) = 0;

    virtual void operator()(const server::Trace<Heartbeat> &) = 0;

    virtual void operator()(const server::Trace<Ticker> &) = 0;
    virtual void operator()(const server::Trace<BookSnapshot> &) = 0;
    virtual void operator()(const server::Trace<Book> &) = 0;
    virtual void operator()(const server::Trace<TradeSnapshot> &) = 0;
    virtual void operator()(const server::Trace<Trade> &) = 0;
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

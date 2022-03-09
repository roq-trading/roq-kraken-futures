/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.hpp"

#include "roq/server.hpp"

#include "roq/kraken_futures/json/alert.hpp"
#include "roq/kraken_futures/json/error.hpp"
#include "roq/kraken_futures/json/info.hpp"

#include "roq/kraken_futures/json/subscribed.hpp"

#include "roq/kraken_futures/json/heartbeat.hpp"

#include "roq/kraken_futures/json/book.hpp"
#include "roq/kraken_futures/json/book_snapshot.hpp"
#include "roq/kraken_futures/json/ticker.hpp"
#include "roq/kraken_futures/json/trade.hpp"
#include "roq/kraken_futures/json/trade_snapshot.hpp"

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

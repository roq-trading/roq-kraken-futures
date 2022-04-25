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
    virtual void operator()(const Trace<Info const> &) = 0;
    virtual void operator()(const Trace<Alert const> &) = 0;
    virtual void operator()(const Trace<Error const> &) = 0;

    virtual void operator()(const Trace<Subscribed const> &) = 0;

    virtual void operator()(const Trace<Heartbeat const> &) = 0;

    virtual void operator()(const Trace<Ticker const> &) = 0;
    virtual void operator()(const Trace<BookSnapshot const> &) = 0;
    virtual void operator()(const Trace<Book const> &) = 0;
    virtual void operator()(const Trace<TradeSnapshot const> &) = 0;
    virtual void operator()(const Trace<Trade const> &) = 0;
  };

  static bool dispatch(
      Handler &, const std::string_view &message, core::json::Buffer &, const TraceInfo &);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

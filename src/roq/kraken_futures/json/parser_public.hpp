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
    virtual void operator()(Trace<Info const> const &) = 0;
    virtual void operator()(Trace<Alert const> const &) = 0;
    virtual void operator()(Trace<Error const> const &) = 0;

    virtual void operator()(Trace<Subscribed const> const &) = 0;

    virtual void operator()(Trace<Heartbeat const> const &) = 0;

    virtual void operator()(Trace<Ticker const> const &) = 0;
    virtual void operator()(Trace<BookSnapshot const> const &) = 0;
    virtual void operator()(Trace<Book const> const &) = 0;
    virtual void operator()(Trace<TradeSnapshot const> const &) = 0;
    virtual void operator()(Trace<Trade const> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::Buffer &, TraceInfo const &);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/trace_info.hpp"

#include "roq/core/json/buffer_stack.hpp"

#include "roq/kraken_futures/protocol/json/alert.hpp"
#include "roq/kraken_futures/protocol/json/error.hpp"
#include "roq/kraken_futures/protocol/json/info.hpp"

#include "roq/kraken_futures/protocol/json/subscribed.hpp"

#include "roq/kraken_futures/protocol/json/heartbeat.hpp"

#include "roq/kraken_futures/protocol/json/book.hpp"
#include "roq/kraken_futures/protocol/json/book_snapshot.hpp"
#include "roq/kraken_futures/protocol/json/ticker.hpp"
#include "roq/kraken_futures/protocol/json/trade.hpp"
#include "roq/kraken_futures/protocol/json/trade_snapshot.hpp"

namespace roq {
namespace kraken_futures {
namespace protocol {
namespace json {

struct ParserPublic final {
  struct Handler {
    virtual void operator()(Trace<Info> const &) = 0;
    virtual void operator()(Trace<Alert> const &) = 0;
    virtual void operator()(Trace<Error> const &) = 0;

    virtual void operator()(Trace<Subscribed> const &) = 0;

    virtual void operator()(Trace<Heartbeat> const &) = 0;

    virtual void operator()(Trace<Ticker> const &) = 0;
    virtual void operator()(Trace<BookSnapshot> const &) = 0;
    virtual void operator()(Trace<Book> const &) = 0;
    virtual void operator()(Trace<TradeSnapshot> const &) = 0;
    virtual void operator()(Trace<Trade> const &) = 0;
  };

  static bool dispatch(Handler &, std::string_view const &message, core::json::BufferStack &, TraceInfo const &, bool allow_unknown_event_types);
};

}  // namespace json
}  // namespace protocol
}  // namespace kraken_futures
}  // namespace roq

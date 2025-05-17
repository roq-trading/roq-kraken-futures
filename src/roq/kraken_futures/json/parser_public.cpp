/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_public.hpp"

#include "roq/logging.hpp"

#include "roq/kraken_futures/json/event.hpp"
#include "roq/kraken_futures/json/feed.hpp"
#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace json {

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer, auto &trace_info) {
  T obj{message, buffer};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool ParserPublic::dispatch(Handler &handler, std::string_view const &message, std::span<std::byte> const &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    // event
    if (key.compare("event"sv) == 0) {
      Event event{value};
      switch (event) {
        using enum Event::type_t;
        case UNDEFINED_INTERNAL:
          assert(false);
          [[fallthrough]];
        case UNKNOWN_INTERNAL:
          log::warn(R"(Unknown event="{}")"sv, event);
          return false;
        case INFO:
          dispatch_helper<Info>(handler, message, buffer, trace_info);
          return true;
        case ALERT:
          dispatch_helper<Alert>(handler, message, buffer, trace_info);
          return true;
        case ERROR:
          dispatch_helper<Error>(handler, message, buffer, trace_info);
          return true;
        case CHALLENGE:
          log::fatal("Unexpected: event={}"sv, event);
          break;
        case SUBSCRIBED:
          dispatch_helper<Subscribed>(handler, message, buffer, trace_info);
          return true;
        default:
          assert(false);
      }
    }
    // feed
    if (key.compare("feed"sv) == 0) {
      Feed feed(value);
      switch (feed) {
        using enum Feed::type_t;
        case UNDEFINED_INTERNAL:
          assert(false);
          [[fallthrough]];
        case UNKNOWN_INTERNAL:
          log::warn(R"(Unknown feed="{}")"sv, feed);
          return false;
        case HEARTBEAT:
          dispatch_helper<Heartbeat>(handler, message, buffer, trace_info);
          return true;
        case TICKER:
          dispatch_helper<Ticker>(handler, message, buffer, trace_info);
          return true;
        case BOOK_SNAPSHOT:
          dispatch_helper<BookSnapshot>(handler, message, buffer, trace_info);
          return true;
        case BOOK:
          dispatch_helper<Book>(handler, message, buffer, trace_info);
          return true;
        case TRADE_SNAPSHOT:
          dispatch_helper<TradeSnapshot>(handler, message, buffer, trace_info);
          return true;
        case TRADE:
          dispatch_helper<Trade>(handler, message, buffer, trace_info);
          return true;
        case CHALLENGE:
        case ACCOUNT_BALANCES_AND_MARGINS:
        case OPEN_POSITIONS:
        case OPEN_ORDERS_SNAPSHOT:
        case OPEN_ORDERS:
        case OPEN_ORDERS_VERBOSE:
        case FILLS_SNAPSHOT:
        case FILLS:
          log::fatal("Unexpected: feed={}"sv, feed);
          break;
        default:
          assert(false);
      }
    }
  }
  return false;
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

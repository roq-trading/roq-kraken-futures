/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_public.hpp"

#include "roq/logging.hpp"

#include "roq/kraken_futures/json/event.hpp"
#include "roq/kraken_futures/json/feed.hpp"
#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace json {

// === CONSTANTS ===

namespace {
auto const FIELD_EVENT = "event"sv;
auto const FIELD_FEED = "feed"sv;
}  // namespace

// === HELPERS ===

namespace {
template <typename T>
void dispatch_helper(auto &handler, auto &message, auto &buffer_stack, auto &trace_info) {
  T obj{message, buffer_stack};
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool ParserPublic::dispatch(
    Handler &handler, std::string_view const &message, core::json::BufferStack &buffer_stack, TraceInfo const &trace_info, bool allow_unknown_event_types) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    if (key == FIELD_EVENT) {
      Event event{value};
      switch (event) {
        using enum Event::type_t;
        case UNDEFINED_INTERNAL:
          break;
        case UNKNOWN_INTERNAL:
          if (allow_unknown_event_types) {
            return false;
          }
          break;
        case INFO:
          dispatch_helper<Info>(handler, message, buffer_stack, trace_info);
          return true;
        case ALERT:
          dispatch_helper<Alert>(handler, message, buffer_stack, trace_info);
          return true;
        case ERROR:
          dispatch_helper<Error>(handler, message, buffer_stack, trace_info);
          return true;
        case CHALLENGE:
          break;
        case SUBSCRIBED:
          dispatch_helper<Subscribed>(handler, message, buffer_stack, trace_info);
          return true;
        case SUBSCRIBED_FAILED:
        case UNSUBSCRIBED:
        case UNSUBSCRIBED_FAILED:
          break;
      }
    } else if (key == FIELD_FEED) {
      Feed feed(value);
      switch (feed) {
        using enum Feed::type_t;
        case UNDEFINED_INTERNAL:
          break;
        case UNKNOWN_INTERNAL:
          if (allow_unknown_event_types) {
            return false;
          }
          break;
        case HEARTBEAT:
          dispatch_helper<Heartbeat>(handler, message, buffer_stack, trace_info);
          return true;
        case TICKER:
          dispatch_helper<Ticker>(handler, message, buffer_stack, trace_info);
          return true;
        case BOOK_SNAPSHOT:
          dispatch_helper<BookSnapshot>(handler, message, buffer_stack, trace_info);
          return true;
        case BOOK:
          dispatch_helper<Book>(handler, message, buffer_stack, trace_info);
          return true;
        case TRADE_SNAPSHOT:
          dispatch_helper<TradeSnapshot>(handler, message, buffer_stack, trace_info);
          return true;
        case TRADE:
          dispatch_helper<Trade>(handler, message, buffer_stack, trace_info);
          return true;
        case CHALLENGE:
        case ACCOUNT_BALANCES_AND_MARGINS:
        case OPEN_POSITIONS:
        case OPEN_ORDERS_SNAPSHOT:
        case OPEN_ORDERS:
        case OPEN_ORDERS_VERBOSE:
        case FILLS_SNAPSHOT:
        case FILLS:
          break;
      }
    }
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_private.hpp"

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

bool ParserPrivate::dispatch(
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
          dispatch_helper<Challenge>(handler, message, buffer_stack, trace_info);
          return true;
        case SUBSCRIBED:
          dispatch_helper<Subscribed>(handler, message, buffer_stack, trace_info);
          return true;
        case SUBSCRIBED_FAILED:
        case UNSUBSCRIBED:
        case UNSUBSCRIBED_FAILED:
          break;
      }
    } else if (key == FIELD_FEED) {
      Feed feed{value};
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
        case BOOK_SNAPSHOT:
        case BOOK:
        case TRADE_SNAPSHOT:
        case TRADE:
          break;
        case CHALLENGE:
          dispatch_helper<Challenge>(handler, message, buffer_stack, trace_info);
          return true;
        case ACCOUNT_BALANCES_AND_MARGINS:
          dispatch_helper<AccountBalancesAndMargins>(handler, message, buffer_stack, trace_info);
          return true;
        case OPEN_POSITIONS:
          dispatch_helper<OpenPositions>(handler, message, buffer_stack, trace_info);
          return true;
        case OPEN_ORDERS_SNAPSHOT:
          dispatch_helper<OpenOrdersSnapshot>(handler, message, buffer_stack, trace_info);
          return true;
        case OPEN_ORDERS:
          dispatch_helper<OpenOrders>(handler, message, buffer_stack, trace_info);
          return true;
        case OPEN_ORDERS_VERBOSE:
          break;
        case FILLS_SNAPSHOT:
          dispatch_helper<FillsSnapshot>(handler, message, buffer_stack, trace_info);
          return true;
        case FILLS:
          dispatch_helper<Fills>(handler, message, buffer_stack, trace_info);
          return true;
          break;
      }
    }
  }
  log::fatal(R"(Unexpected: message="{}")"sv, message);
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

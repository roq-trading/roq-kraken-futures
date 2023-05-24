/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_private.hpp"

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
  auto obj = T::create(message, buffer);
  create_trace_and_dispatch(handler, trace_info, obj);
}
}  // namespace

// === IMPLEMENTATION ===

bool ParserPrivate::dispatch(
    Handler &handler,
    std::string_view const &message,
    std::span<std::byte> const &buffer,
    TraceInfo const &trace_info) {
  core::json::Parser parser{message};
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    // event
    if (key.compare("event"sv) == 0) {
      Event event{value};
      switch (event) {
        using enum Event::type_t;
        case UNDEFINED__:
          assert(false);
          [[fallthrough]];
        case UNKNOWN__:
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
          dispatch_helper<Challenge>(handler, message, buffer, trace_info);
          return true;
        case SUBSCRIBED:
          dispatch_helper<Subscribed>(handler, message, buffer, trace_info);
          return true;
        default:
          assert(false);
      }
    }
    // feed
    if (key.compare("feed"sv) == 0) {
      Feed feed{value};
      switch (feed) {
        using enum Feed::type_t;
        case UNDEFINED__:
          assert(false);
          [[fallthrough]];
        case UNKNOWN__:
          log::warn(R"(Unknown feed="{}")"sv, feed);
          return false;
        case HEARTBEAT:
          dispatch_helper<Heartbeat>(handler, message, buffer, trace_info);
          return true;
        case TICKER:
        case BOOK_SNAPSHOT:
        case BOOK:
        case TRADE_SNAPSHOT:
        case TRADE:
          log::fatal("Unexpected: feed={}"sv, feed);
          break;
        case CHALLENGE:
          dispatch_helper<Challenge>(handler, message, buffer, trace_info);
          return true;
        case ACCOUNT_BALANCES_AND_MARGINS:
          dispatch_helper<AccountBalancesAndMargins>(handler, message, buffer, trace_info);
          return true;
        case OPEN_POSITIONS:
          dispatch_helper<OpenPositions>(handler, message, buffer, trace_info);
          return true;
        case OPEN_ORDERS_SNAPSHOT:
          dispatch_helper<OpenOrdersSnapshot>(handler, message, buffer, trace_info);
          return true;
        case OPEN_ORDERS:
          dispatch_helper<OpenOrders>(handler, message, buffer, trace_info);
          return true;
        case OPEN_ORDERS_VERBOSE:
          // XXX
          break;
        case FILLS_SNAPSHOT:
          dispatch_helper<FillsSnapshot>(handler, message, buffer, trace_info);
          return true;
        case FILLS:
          dispatch_helper<Fills>(handler, message, buffer, trace_info);
          return true;
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

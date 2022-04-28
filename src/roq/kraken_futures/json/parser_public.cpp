/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_public.hpp"

#include "roq/logging.hpp"

#include "roq/kraken_futures/json/event.hpp"
#include "roq/kraken_futures/json/feed.hpp"
#include "roq/kraken_futures/json/utils.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace json {

namespace {
template <typename H>
void dispatch_info(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Info info(root);
  create_trace_and_dispatch(handler, trace_info, info);
}

template <typename H>
void dispatch_alert(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Alert alert(root);
  create_trace_and_dispatch(handler, trace_info, alert);
}

template <typename H>
void dispatch_error(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Error error(root);
  create_trace_and_dispatch(handler, trace_info, error);
}

template <typename H>
void dispatch_subscribed(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Subscribed subscribed(root, buffer);
  create_trace_and_dispatch(handler, trace_info, subscribed);
}

template <typename H>
void dispatch_heartbeat(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Heartbeat heartbeat(root);
  create_trace_and_dispatch(handler, trace_info, heartbeat);
}

template <typename H>
void dispatch_ticker(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Ticker ticker(root);
  create_trace_and_dispatch(handler, trace_info, ticker);
}

template <typename H>
void dispatch_book_snapshot(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const BookSnapshot book_snapshot(root, buffer);
  create_trace_and_dispatch(handler, trace_info, book_snapshot);
}

template <typename H>
void dispatch_book(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Book book(root);
  create_trace_and_dispatch(handler, trace_info, book);
}

template <typename H>
void dispatch_trade_snapshot(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const TradeSnapshot trade_snapshot(root, buffer);
  create_trace_and_dispatch(handler, trace_info, trade_snapshot);
}

template <typename H>
void dispatch_trade(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Trade trade(root);
  create_trace_and_dispatch(handler, trace_info, trade);
}
}  // namespace

bool ParserPublic::dispatch(
    Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::Object>(root)) {
    // event
    if (key.compare("event"sv) == 0) {
      Event event(value);
      switch (event) {
        using enum Event::type_t;
        case UNDEFINED:
          assert(false);
          [[fallthrough]];
        case UNKNOWN:
          log::warn(R"(Unknown event="{}")"sv, event);
          return false;
        case INFO:
          dispatch_info(handler, message, trace_info);
          return true;
        case ALERT:
          dispatch_alert(handler, message, trace_info);
          return true;
        case ERROR:
          dispatch_error(handler, message, trace_info);
          return true;
        case CHALLENGE:
          log::fatal("Unexpected: event={}"sv, event);
          break;
        case SUBSCRIBED:
          dispatch_subscribed(handler, message, buffer, trace_info);
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
        case UNDEFINED:
          assert(false);
          [[fallthrough]];
        case UNKNOWN:
          log::warn(R"(Unknown feed="{}")"sv, feed);
          return false;
        case HEARTBEAT:
          dispatch_heartbeat(handler, message, trace_info);
          return true;
        case TICKER:
          dispatch_ticker(handler, message, trace_info);
          return true;
        case BOOK_SNAPSHOT:
          dispatch_book_snapshot(handler, message, buffer, trace_info);
          return true;
        case BOOK:
          dispatch_book(handler, message, trace_info);
          return true;
        case TRADE_SNAPSHOT:
          dispatch_trade_snapshot(handler, message, buffer, trace_info);
          return true;
        case TRADE:
          dispatch_trade(handler, message, trace_info);
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

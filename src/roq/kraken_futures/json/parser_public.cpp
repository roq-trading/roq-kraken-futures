/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_public.h"

#include "roq/logging.h"

#include "roq/kraken_futures/json/event.h"
#include "roq/kraken_futures/json/feed.h"
#include "roq/kraken_futures/json/utils.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {
namespace json {

namespace {
template <typename H>
static void dispatch_info(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Info info(root);
  server::create_trace_and_dispatch(trace_info, info, handler);
}

template <typename H>
static void dispatch_alert(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Alert alert(root);
  server::create_trace_and_dispatch(trace_info, alert, handler);
}

template <typename H>
static void dispatch_error(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Error error(root);
  server::create_trace_and_dispatch(trace_info, error, handler);
}

template <typename H>
static void dispatch_subscribed(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Subscribed subscribed(root, buffer);
  server::create_trace_and_dispatch(trace_info, subscribed, handler);
}

template <typename H>
static void dispatch_heartbeat(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Heartbeat heartbeat(root);
  server::create_trace_and_dispatch(trace_info, heartbeat, handler);
}

template <typename H>
static void dispatch_ticker(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Ticker ticker(root);
  server::create_trace_and_dispatch(trace_info, ticker, handler);
}

template <typename H>
static void dispatch_book_snapshot(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  BookSnapshot book_snapshot(root, buffer);
  server::create_trace_and_dispatch(trace_info, book_snapshot, handler);
}

template <typename H>
static void dispatch_book(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Book book(root);
  server::create_trace_and_dispatch(trace_info, book, handler);
}

template <typename H>
static void dispatch_trade_snapshot(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  TradeSnapshot trade_snapshot(root, buffer);
  server::create_trace_and_dispatch(trace_info, trade_snapshot, handler);
}

template <typename H>
static void dispatch_trade(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Trade trade(root);
  server::create_trace_and_dispatch(trace_info, trade, handler);
}
}  // namespace

bool ParserPublic::dispatch(
    Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::object_t>(root)) {
    // event
    if (key.compare("event"_sv) == 0) {
      Event event = {};
      update(event, value);
      switch (event) {
        case Event::UNDEFINED:
          assert(false);
          [[fallthrough]];
        case Event::UNKNOWN:
          log::warn(R"(Unknown event="{}")"_sv, event);
          return false;
        case Event::INFO:
          dispatch_info(handler, message, trace_info);
          return true;
        case Event::ALERT:
          dispatch_alert(handler, message, trace_info);
          return true;
        case Event::ERROR:
          dispatch_error(handler, message, trace_info);
          return true;
        case Event::CHALLENGE:
          log::fatal("Unexpected: event={}"_sv, event);
          break;
        case Event::SUBSCRIBED:
          dispatch_subscribed(handler, message, buffer, trace_info);
          return true;
        default:
          assert(false);
      }
    }
    // feed
    if (key.compare("feed"_sv) == 0) {
      Feed feed = {};
      update(feed, value);
      switch (feed) {
        case Feed::UNDEFINED:
          assert(false);
          [[fallthrough]];
        case Feed::UNKNOWN:
          log::warn(R"(Unknown feed="{}")"_sv, feed);
          return false;
        case Feed::HEARTBEAT:
          dispatch_heartbeat(handler, message, trace_info);
          return true;
        case Feed::TICKER:
          dispatch_ticker(handler, message, trace_info);
          return true;
        case Feed::BOOK_SNAPSHOT:
          dispatch_book_snapshot(handler, message, buffer, trace_info);
          return true;
        case Feed::BOOK:
          dispatch_book(handler, message, trace_info);
          return true;
        case Feed::TRADE_SNAPSHOT:
          dispatch_trade_snapshot(handler, message, buffer, trace_info);
          return true;
        case Feed::TRADE:
          dispatch_trade(handler, message, trace_info);
          return true;
        case Feed::CHALLENGE:
        case Feed::ACCOUNT_BALANCES_AND_MARGINS:
        case Feed::OPEN_POSITIONS:
        case Feed::OPEN_ORDERS_SNAPSHOT:
        case Feed::OPEN_ORDERS:
        case Feed::OPEN_ORDERS_VERBOSE:
        case Feed::FILLS:
          log::fatal("Unexpected: feed={}"_sv, feed);
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

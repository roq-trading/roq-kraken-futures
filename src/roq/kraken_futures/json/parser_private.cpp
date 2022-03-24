/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_private.hpp"

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
  Info info(root);
  create_trace_and_dispatch(handler, trace_info, info);
}

template <typename H>
void dispatch_alert(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Alert alert(root);
  create_trace_and_dispatch(handler, trace_info, alert);
}

template <typename H>
void dispatch_error(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Error error(root);
  create_trace_and_dispatch(handler, trace_info, error);
}

template <typename H>
void dispatch_challenge(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Challenge challenge(root);
  create_trace_and_dispatch(handler, trace_info, challenge);
}

template <typename H>
void dispatch_subscribed(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Subscribed subscribed(root, buffer);
  create_trace_and_dispatch(handler, trace_info, subscribed);
}

template <typename H>
void dispatch_heartbeat(H &handler, const std::string_view &message, const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Heartbeat heartbeat(root);
  create_trace_and_dispatch(handler, trace_info, heartbeat);
}

template <typename H>
void dispatch_account_balances_and_margins(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  AccountBalancesAndMargins account_balances_and_margins(root, buffer);
  create_trace_and_dispatch(handler, trace_info, account_balances_and_margins);
}

template <typename H>
void dispatch_open_positions(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  OpenPositions open_positions(root, buffer);
  create_trace_and_dispatch(handler, trace_info, open_positions);
}

template <typename H>
void dispatch_open_orders_snapshot(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  OpenOrdersSnapshot open_orders_snapshot(root, buffer);
  create_trace_and_dispatch(handler, trace_info, open_orders_snapshot);
}

template <typename H>
void dispatch_open_orders(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  OpenOrders open_orders(root, buffer);
  create_trace_and_dispatch(handler, trace_info, open_orders);
}

template <typename H>
void dispatch_fills_snapshot(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  FillsSnapshot fills_snapshot(root, buffer);
  create_trace_and_dispatch(handler, trace_info, fills_snapshot);
}

template <typename H>
void dispatch_fills(
    H &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Fills fills(root, buffer);
  create_trace_and_dispatch(handler, trace_info, fills);
}
}  // namespace

bool ParserPrivate::dispatch(
    Handler &handler,
    const std::string_view &message,
    core::json::Buffer &buffer,
    const TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::object_t>(root)) {
    // event
    if (key.compare("event"sv) == 0) {
      Event event(value);
      switch (event) {
        case Event::UNDEFINED:
          assert(false);
          [[fallthrough]];
        case Event::UNKNOWN:
          log::warn(R"(Unknown event="{}")"sv, event);
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
          dispatch_challenge(handler, message, trace_info);
          return true;
        case Event::SUBSCRIBED:
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
        case Feed::UNDEFINED:
          assert(false);
          [[fallthrough]];
        case Feed::UNKNOWN:
          log::warn(R"(Unknown feed="{}")"sv, feed);
          return false;
        case Feed::HEARTBEAT:
          dispatch_heartbeat(handler, message, trace_info);
          return true;
        case Feed::TICKER:
        case Feed::BOOK_SNAPSHOT:
        case Feed::BOOK:
        case Feed::TRADE_SNAPSHOT:
        case Feed::TRADE:
          log::fatal("Unexpected: feed={}"sv, feed);
          break;
        case Feed::CHALLENGE:
          dispatch_challenge(handler, message, trace_info);
          return true;
        case Feed::ACCOUNT_BALANCES_AND_MARGINS:
          dispatch_account_balances_and_margins(handler, message, buffer, trace_info);
          return true;
        case Feed::OPEN_POSITIONS:
          dispatch_open_positions(handler, message, buffer, trace_info);
          return true;
        case Feed::OPEN_ORDERS_SNAPSHOT:
          dispatch_open_orders_snapshot(handler, message, buffer, trace_info);
          return true;
        case Feed::OPEN_ORDERS:
          dispatch_open_orders(handler, message, buffer, trace_info);
          return true;
        case Feed::OPEN_ORDERS_VERBOSE:
          // XXX
          break;
        case Feed::FILLS_SNAPSHOT:
          dispatch_fills_snapshot(handler, message, buffer, trace_info);
          return true;
        case Feed::FILLS:
          dispatch_fills(handler, message, buffer, trace_info);
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

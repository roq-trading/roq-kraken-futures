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
void dispatch_info(H &handler, std::string_view const &message, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Info info(root);
  create_trace_and_dispatch(handler, trace_info, info);
}

template <typename H>
void dispatch_alert(H &handler, std::string_view const &message, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Alert alert(root);
  create_trace_and_dispatch(handler, trace_info, alert);
}

template <typename H>
void dispatch_error(H &handler, std::string_view const &message, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Error error(root);
  create_trace_and_dispatch(handler, trace_info, error);
}

template <typename H>
void dispatch_challenge(H &handler, std::string_view const &message, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Challenge challenge(root);
  create_trace_and_dispatch(handler, trace_info, challenge);
}

template <typename H>
void dispatch_subscribed(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Subscribed subscribed(root, buffer);
  create_trace_and_dispatch(handler, trace_info, subscribed);
}

template <typename H>
void dispatch_heartbeat(H &handler, std::string_view const &message, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Heartbeat heartbeat(root);
  create_trace_and_dispatch(handler, trace_info, heartbeat);
}

template <typename H>
void dispatch_account_balances_and_margins(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const AccountBalancesAndMargins account_balances_and_margins(root, buffer);
  create_trace_and_dispatch(handler, trace_info, account_balances_and_margins);
}

template <typename H>
void dispatch_open_positions(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const OpenPositions open_positions(root, buffer);
  create_trace_and_dispatch(handler, trace_info, open_positions);
}

template <typename H>
void dispatch_open_orders_snapshot(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const OpenOrdersSnapshot open_orders_snapshot(root, buffer);
  create_trace_and_dispatch(handler, trace_info, open_orders_snapshot);
}

template <typename H>
void dispatch_open_orders(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const OpenOrders open_orders(root, buffer);
  create_trace_and_dispatch(handler, trace_info, open_orders);
}

template <typename H>
void dispatch_fills_snapshot(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const FillsSnapshot fills_snapshot(root, buffer);
  create_trace_and_dispatch(handler, trace_info, fills_snapshot);
}

template <typename H>
void dispatch_fills(
    H &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  const Fills fills(root, buffer);
  create_trace_and_dispatch(handler, trace_info, fills);
}
}  // namespace

bool ParserPrivate::dispatch(
    Handler &handler, std::string_view const &message, core::json::Buffer &buffer, TraceInfo const &trace_info) {
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
          dispatch_challenge(handler, message, trace_info);
          return true;
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
        case BOOK_SNAPSHOT:
        case BOOK:
        case TRADE_SNAPSHOT:
        case TRADE:
          log::fatal("Unexpected: feed={}"sv, feed);
          break;
        case CHALLENGE:
          dispatch_challenge(handler, message, trace_info);
          return true;
        case ACCOUNT_BALANCES_AND_MARGINS:
          dispatch_account_balances_and_margins(handler, message, buffer, trace_info);
          return true;
        case OPEN_POSITIONS:
          dispatch_open_positions(handler, message, buffer, trace_info);
          return true;
        case OPEN_ORDERS_SNAPSHOT:
          dispatch_open_orders_snapshot(handler, message, buffer, trace_info);
          return true;
        case OPEN_ORDERS:
          dispatch_open_orders(handler, message, buffer, trace_info);
          return true;
        case OPEN_ORDERS_VERBOSE:
          // XXX
          break;
        case FILLS_SNAPSHOT:
          dispatch_fills_snapshot(handler, message, buffer, trace_info);
          return true;
        case FILLS:
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

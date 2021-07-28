/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_public.h"

#include "roq/logging.h"

// #include "roq/kraken_futures/json/event.h"

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
  handler(info, trace_info);
}

template <typename H>
static void dispatch_alert(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Alert alert(root);
  handler(alert, trace_info);
}

template <typename H>
static void dispatch_subscribed(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Subscribed subscribed(root);
  handler(subscribed, trace_info);
}

template <typename H>
static void dispatch_ticker(
    H &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  Ticker ticker(root);
  handler(ticker, trace_info);
}
}  // namespace

bool ParserPublic::dispatch(
    Handler &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::object_t>(root)) {
    if (key.compare("event"_sv) == 0) {
      auto event = core::json::get<std::string_view>(value);
      if (event.compare("info") == 0) {
        dispatch_info(handler, message, trace_info);
        return true;
      }
      if (event.compare("alert") == 0) {
        dispatch_alert(handler, message, trace_info);
        return true;
      }
      if (event.compare("subscribed") == 0) {
        dispatch_subscribed(handler, message, trace_info);
        return true;
      }
      if (event.compare("ticker") == 0) {
        dispatch_ticker(handler, message, trace_info);
        return true;
      }
    }
  }
  return false;
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

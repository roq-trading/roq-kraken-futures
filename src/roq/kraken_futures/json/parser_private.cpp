/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/json/parser_private.h"

#include "roq/logging.h"

// #include "roq/kraken_futures/json/event.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {
namespace json {

bool ParserPrivate::dispatch(
    Handler &handler, const std::string_view &message, const server::TraceInfo &trace_info) {
  core::json::Parser parser(message);
  auto root = parser.root();
  for (auto [key, value] : std::get<core::json::object_t>(root)) {
    if (key.compare("event"_sv) == 0) {
    }
  }
  return false;
}

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

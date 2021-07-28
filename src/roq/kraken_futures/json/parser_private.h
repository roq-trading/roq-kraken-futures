/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/core/json/parser.h"

#include "roq/server.h"

namespace roq {
namespace kraken_futures {
namespace json {

struct ParserPrivate final {
  struct Handler {};

  static bool dispatch(
      Handler &handler, const std::string_view &message, const server::TraceInfo &trace_info);
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

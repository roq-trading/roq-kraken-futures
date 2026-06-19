/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/kraken_futures/gateway/api.hpp"
#include "roq/kraken_futures/gateway/settings.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  server::Dispatcher &dispatcher;

  Settings const &settings;
  API const api;

  core::Symbols symbols;
  utils::unordered_set<std::string> all_symbols;

  core::limit::RateLimiter rate_limiter;

  core::TimerQueue<std::string> time_series_request_queue;

  std::vector<MBPUpdate> bids, asks, final_bids, final_asks;
  std::vector<Fill> fills;
  std::vector<Bar> bars;
};

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

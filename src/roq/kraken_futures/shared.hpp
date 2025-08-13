/* Copyright (c) 2017-2025, Hans Erik Thrane */

#pragma once

#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"
#include "roq/core/timer_queue.hpp"

#include "roq/core/limit/rate_limiter.hpp"

#include "roq/kraken_futures/api.hpp"
#include "roq/kraken_futures/settings.hpp"

namespace roq {
namespace kraken_futures {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared const &) = delete;

  auto discard_symbol(std::string_view const &name) const { return dispatcher_.discard_symbol(name); }

  template <typename... Args>
  auto find_order(Args &&...args) {
    return dispatcher_.find_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto update_order(Args &&...args) {
    return dispatcher_.update_order(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto operator()(Args &&...args) {
    return dispatcher_(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto get_ref_data(Args &&...args) {
    return dispatcher_.get_ref_data(std::forward<Args>(args)...);
  }

 public:
  std::vector<MBPUpdate> bids, asks;
  std::vector<Fill> fills;

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;
  Interval const settings_time_series_interval;
  API const api;
  core::Symbols symbols;

  core::limit::RateLimiter rate_limiter;
  core::TimerQueue<std::string> time_series_request_queue;

  std::vector<Bar> bars;
};

}  // namespace kraken_futures
}  // namespace roq

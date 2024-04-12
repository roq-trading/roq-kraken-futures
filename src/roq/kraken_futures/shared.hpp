/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <utility>
#include <vector>

#include "roq/api.hpp"
#include "roq/server.hpp"

#include "roq/core/symbols.hpp"

#include "roq/kraken_futures/api.hpp"
#include "roq/kraken_futures/settings.hpp"

namespace roq {
namespace kraken_futures {

struct Shared final {
  Shared(server::Dispatcher &, Settings const &);

  Shared(Shared &&) = default;
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

 public:
  std::vector<MBPUpdate> bids, asks;

 private:
  server::Dispatcher &dispatcher_;

 public:
  Settings const &settings;
  API const api;
  core::Symbols symbols;
};

}  // namespace kraken_futures
}  // namespace roq

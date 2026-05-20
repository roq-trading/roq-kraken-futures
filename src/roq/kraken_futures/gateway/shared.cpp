/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kraken_futures/gateway/shared.hpp"

namespace roq {
namespace kraken_futures {
namespace gateway {

// === HELPERS ===

namespace {
auto create_rate_limiter(auto &settings) {
  return core::limit::RateLimiter{settings.request.limit, settings.request.limit_interval};
}
}  // namespace

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher_{dispatcher}, settings{settings}, api{API::create(settings)}, symbols{settings.ws.max_subscriptions_per_stream},
      rate_limiter{create_rate_limiter(settings)} {
}

}  // namespace gateway
}  // namespace kraken_futures
}  // namespace roq

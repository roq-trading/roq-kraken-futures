/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/shared.hpp"

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher, Settings const &settings)
    : dispatcher_{dispatcher}, settings{settings}, symbols{settings.ws.max_subscriptions_per_stream} {
}

}  // namespace kraken_futures
}  // namespace roq

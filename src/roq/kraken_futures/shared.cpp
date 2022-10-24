/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/shared.hpp"

#include "roq/kraken_futures/flags.hpp"

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

Shared::Shared(server::Dispatcher &dispatcher)
    : fills(server::Flags::cache_fills_max_depth()), bids(server::Flags::cache_mbp_max_depth()),
      asks(server::Flags::cache_mbp_max_depth()), final_bids(server::Flags::cache_mbp_max_depth()),
      final_asks(server::Flags::cache_mbp_max_depth()), trades(server::Flags::cache_trades_max_depth()),
      dispatcher_{dispatcher}, symbols{Flags::ws_max_subscriptions_per_stream()} {
}

}  // namespace kraken_futures
}  // namespace roq

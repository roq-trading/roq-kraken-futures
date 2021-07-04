/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/shared.h"

#include "roq/kraken_futures/flags.h"

namespace roq {
namespace kraken_futures {

Shared::Shared(server::Dispatcher &dispatcher)
    : bids(server::Flags::cache_mbp_max_depth()), asks(server::Flags::cache_mbp_max_depth()),
      trades(server::Flags::cache_trades_max_depth()), dispatcher_(dispatcher) {
}

}  // namespace kraken_futures
}  // namespace roq

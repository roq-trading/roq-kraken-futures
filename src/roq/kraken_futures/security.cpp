/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/security.h"

#include <algorithm>

#include "roq/core/clock.h"

#include "roq/kraken_futures/flags.h"

using namespace std::chrono_literals;  // NOLINT

namespace roq {
namespace kraken_futures {

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), key_(config.get_access_key(account)),
      hasher_(config.get_access_secret(account)) {
}

std::string Security::create_headers(const std::string_view &path, const std::string_view &query) {
  return hasher_.create_headers(path, query, key_, get_nonce());
}

std::string Security::signed_challenge(const std::string_view &original_challenge) {
  return hasher_.signed_challenge(original_challenge);
}

std::chrono::milliseconds Security::get_nonce() {
  if (Flags::rest_use_nonce()) {
    auto now = std::chrono::duration_cast<decltype(nonce_)>(core::get_realtime_clock());
    nonce_ = std::max(now, nonce_ + 1ms);  // note! can't reuse
  }
  return nonce_;
}

}  // namespace kraken_futures
}  // namespace roq

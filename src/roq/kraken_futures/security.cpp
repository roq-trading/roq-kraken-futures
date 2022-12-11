/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/security.hpp"

#include <algorithm>

#include "roq/clock.hpp"

#include "roq/kraken_futures/flags.hpp"

using namespace std::chrono_literals;  // NOLINT

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

Security::Security(Config const &config, std::string_view const &account)
    : account_{account}, key_{config.get_access_key(account)}, hasher_{config.get_access_secret(account)} {
}

std::string Security::create_headers(std::string_view const &path, std::string_view const &query) {
  return hasher_.create_headers(path, query, key_, get_nonce());
}

std::string Security::signed_challenge(std::string_view const &original_challenge) {
  return hasher_.signed_challenge(original_challenge);
}

std::chrono::milliseconds Security::get_nonce() {
  if (Flags::rest_use_nonce()) {
    auto now = std::chrono::duration_cast<decltype(nonce_)>(clock::get_realtime());
    nonce_ = std::max(now, nonce_ + 1ms);  // note! can't reuse
  }
  return nonce_;
}

}  // namespace kraken_futures
}  // namespace roq

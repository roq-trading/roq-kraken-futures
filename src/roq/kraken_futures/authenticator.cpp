/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/authenticator.hpp"

#include <algorithm>

#include "roq/clock.hpp"

#include "roq/kraken_futures/flags.hpp"

using namespace std::chrono_literals;  // NOLINT

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

Authenticator::Authenticator(Config const &config, std::string_view const &account)
    : account_{account}, key_{config.get_access_key(account)}, crypto_{config.get_access_secret(account)} {
}

std::string Authenticator::create_headers(std::string_view const &path, std::string_view const &query) {
  return crypto_.create_headers(path, query, key_, get_nonce());
}

std::string Authenticator::signed_challenge(std::string_view const &original_challenge) {
  return crypto_.signed_challenge(original_challenge);
}

std::chrono::milliseconds Authenticator::get_nonce() {
  if (Flags::rest_use_nonce()) {
    auto now = std::chrono::duration_cast<decltype(nonce_)>(clock::get_realtime());
    nonce_ = std::max(now, nonce_ + 1ms);  // note! can't reuse
  }
  return nonce_;
}

}  // namespace kraken_futures
}  // namespace roq

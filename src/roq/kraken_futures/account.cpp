/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/account.hpp"

#include <algorithm>

#include "roq/clock.hpp"

using namespace std::chrono_literals;

namespace roq {
namespace kraken_futures {

// === IMPLEMENTATION ===

Account::Account(Config const &config, std::string_view const &name, bool use_nonce)
    : name{name}, key{config.get_access_key(name)}, crypto_{config.get_access_secret(name)}, use_nonce_{use_nonce} {
}

std::string Account::create_headers(std::string_view const &path, std::string_view const &query) {
  return crypto_.create_headers(path, query, key, get_nonce());
}

std::string Account::signed_challenge(std::string_view const &original_challenge) {
  return crypto_.signed_challenge(original_challenge);
}

std::chrono::milliseconds Account::get_nonce() {
  if (use_nonce_) {
    auto now = std::chrono::duration_cast<decltype(nonce_)>(clock::get_realtime());
    nonce_ = std::max(now, nonce_ + 1ms);  // note! can't reuse
  }
  return nonce_;
}

}  // namespace kraken_futures
}  // namespace roq

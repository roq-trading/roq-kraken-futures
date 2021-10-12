/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>

#include "roq/kraken_futures/config.h"

#include "roq/kraken_futures/tools/hasher.h"

namespace roq {
namespace kraken_futures {

class Security final {
 public:
  Security(const Config &, const std::string_view &account);

  Security(Security &&) = delete;
  Security(const Security &) = delete;

  std::string_view get_account() const { return account_; }
  std::string_view get_key() const { return key_; }

  std::string create_headers(const std::string_view &path, const std::string_view &body);

  std::string signed_challenge(const std::string_view &original_challenge);

 protected:
  std::chrono::milliseconds get_nonce();

 private:
  const std::string account_;
  const std::string key_;
  tools::Hasher hasher_;
  std::chrono::milliseconds nonce_ = {};
};

}  // namespace kraken_futures
}  // namespace roq

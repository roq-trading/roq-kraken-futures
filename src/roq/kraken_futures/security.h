/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>

#include "roq/core/crypto/hmac.h"
#include "roq/core/crypto/sha.h"

#include "roq/core/http/method.h"

#include "roq/kraken_futures/config.h"

namespace roq {
namespace kraken_futures {

class Security final {
 public:
  Security(const Config &, const std::string_view &account);

  Security(
      const std::string_view &account,
      const std::string_view &key,
      const std::string_view &password,
      const std::string_view &secret);

  Security(Security &&) = delete;
  Security(const Security &) = delete;

  std::string_view get_account() const { return account_; }
  std::string_view get_key() const { return key_; }

  std::string create_headers(const std::string_view &path, const std::string_view &body);

  std::string signed_challenge(const std::string_view &original_challenge);

 private:
  const std::string account_;
  const std::string key_;
  const std::string password_;
  core::crypto::SHA256 sha_;
  core::crypto::HMAC_SHA512 hmac_;
  // experimental
  std::chrono::milliseconds nonce_ = {};
};

}  // namespace kraken_futures
}  // namespace roq

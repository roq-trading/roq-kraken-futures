/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/crypto/hmac.hpp"
#include "roq/core/crypto/sha.hpp"

namespace roq {
namespace kraken_futures {
namespace tools {

class Hasher final {
 public:
  explicit Hasher(const std::string_view &secret);

  Hasher(Hasher &&) = delete;
  Hasher(const Hasher &) = delete;

  std::string create_headers(
      const std::string_view &path,
      const std::string_view &query,
      const std::string_view &key,
      std::chrono::milliseconds nonce);

  std::string signed_challenge(const std::string_view &original_challenge);

 private:
  core::crypto::SHA256 sha_;
  core::crypto::HMAC_SHA512 hmac_;
};

}  // namespace tools
}  // namespace kraken_futures
}  // namespace roq

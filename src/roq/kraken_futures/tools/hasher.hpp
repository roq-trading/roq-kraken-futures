/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>
#include <string_view>

#include "roq/core/crypto/hmac_sha512.hpp"
#include "roq/core/crypto/sha256.hpp"

namespace roq {
namespace kraken_futures {
namespace tools {

class Hasher final {
 public:
  explicit Hasher(std::string_view const &secret);

  Hasher(Hasher &&) = delete;
  Hasher(Hasher const &) = delete;

  std::string create_headers(
      std::string_view const &path,
      std::string_view const &query,
      std::string_view const &key,
      std::chrono::milliseconds nonce);

  std::string signed_challenge(std::string_view const &original_challenge);

 private:
  core::crypto::SHA256 sha_;
  core::crypto::HMAC_SHA512 hmac_;
};

}  // namespace tools
}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

#include "roq/core/hash/sha256.hpp"
#include "roq/core/hash/sha512.hpp"

#include "roq/core/mac/hmac.hpp"

namespace roq {
namespace kraken_futures {
namespace tools {

struct Crypto final {
  explicit Crypto(std::string_view const &secret);

  Crypto(Crypto &&) = delete;
  Crypto(Crypto const &) = delete;

  std::string create_headers(
      std::string_view const &path,
      std::string_view const &query,
      std::string_view const &key,
      std::chrono::milliseconds nonce);

  std::string signed_challenge(std::string_view const &original_challenge);

 private:
  using Hash = core::hash::SHA256;
  using MAC = core::mac::HMAC<core::hash::SHA512>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  Hash hash_;
  MAC mac_;
  Digest digest_;
};

}  // namespace tools
}  // namespace kraken_futures
}  // namespace roq

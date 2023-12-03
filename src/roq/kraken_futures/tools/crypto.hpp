/* Copyright (c) 2017-2024, Hans Erik Thrane */

#pragma once

#include <array>
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

#include "roq/utils/hash/sha256.hpp"
#include "roq/utils/hash/sha512.hpp"

#include "roq/utils/mac/hmac.hpp"

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
  using Hash = utils::hash::SHA256;
  using MAC = utils::mac::HMAC<utils::hash::SHA512>;
  using Digest = std::array<std::byte, MAC::DIGEST_LENGTH>;

  Hash hash_;
  MAC mac_;
  Digest digest_;
};

}  // namespace tools
}  // namespace kraken_futures
}  // namespace roq

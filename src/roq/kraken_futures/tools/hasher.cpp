/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/tools/hasher.hpp"

#include <fmt/format.h>

#include <array>

#include "roq/core/binascii/base64.hpp"

#include "roq/core/crypto/sha.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace tools {

namespace {
auto create_hmac(const std::string_view &secret) {
  auto raw_secret = core::binascii::Base64::decode(secret, true);
  return core::crypto::HMAC_SHA512(raw_secret);
}
}  // namespace

Hasher::Hasher(const std::string_view &secret) : hmac_(create_hmac(secret)) {
}

std::string Hasher::create_headers(
    const std::string_view &path,
    const std::string_view &query,
    const std::string_view &key,
    std::chrono::milliseconds nonce) {
  assert(!std::empty(path));
  if (nonce.count()) {
    auto nonce_ = fmt::format("{}"sv, nonce.count());
    sha_.clear();
    if (!std::empty(query)) {
      assert(query[0] == '?');
      auto raw = query.substr(1);  // note! not including '?'
      sha_.update(raw);
    }
    sha_.update(nonce_);
    sha_.update(path);
    std::array<char, 32> buffer_1;
    auto length_1 = sha_.digest(buffer_1);
    assert(length_1 == std::size(buffer_1));
    hmac_.clear();
    hmac_.update(std::data(buffer_1), std::size(buffer_1));
    std::array<char, 64> buffer_2;
    auto length_2 = hmac_.digest(buffer_2);
    assert(length_2 == std::size(buffer_2));
    auto authent = core::binascii::Base64::encode(buffer_2, false);
    return fmt::format(
        "APIKey: {}\r\n"
        "Nonce: {}\r\n"
        "Authent: {}\r\n"sv,
        key,
        nonce_,
        authent);
  } else {
    sha_.clear();
    if (!std::empty(query)) {
      assert(query[0] == '?');
      auto raw = query.substr(1);  // note! not including '?'
      sha_.update(raw);
    }
    sha_.update(path);
    std::array<char, 32> buffer_1;
    auto length_1 = sha_.digest(buffer_1);
    assert(length_1 == std::size(buffer_1));
    hmac_.clear();
    hmac_.update(std::data(buffer_1), std::size(buffer_1));
    std::array<char, 64> buffer_2;
    auto length_2 = hmac_.digest(buffer_2);
    assert(length_2 == std::size(buffer_2));
    auto authent = core::binascii::Base64::encode(buffer_2, false);
    return fmt::format(
        "APIKey: {}\r\n"
        "Authent: {}\r\n"sv,
        key,
        authent);
  }
}

std::string Hasher::signed_challenge(const std::string_view &original_challenge) {
  sha_.clear();
  sha_.update(original_challenge);
  std::array<char, 32> buffer_1;
  auto length_1 = sha_.digest(buffer_1);
  assert(length_1 == std::size(buffer_1));
  hmac_.clear();
  hmac_.update(std::data(buffer_1), std::size(buffer_1));
  std::array<char, 64> buffer_2;
  auto length_2 = hmac_.digest(buffer_2);
  assert(length_2 == std::size(buffer_2));
  auto result = core::binascii::Base64::encode(buffer_2, false);
  return result;
}

}  // namespace tools
}  // namespace kraken_futures
}  // namespace roq

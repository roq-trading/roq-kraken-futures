/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/tools/hasher.hpp"

#include <fmt/format.h>

#include <array>
#include <cassert>

#include "roq/core/binascii/base64.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace tools {

namespace {
auto create_hmac(std::string_view const &secret) {
  auto raw_secret = core::binascii::Base64::decode(secret, true);
  return core::crypto::HMAC_SHA512(raw_secret);
}
}  // namespace

Hasher::Hasher(std::string_view const &secret) : hmac_(create_hmac(secret)) {
}

std::string Hasher::create_headers(
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &key,
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

std::string Hasher::signed_challenge(std::string_view const &original_challenge) {
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

/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/tools/crypto.hpp"

#include <fmt/core.h>

#include <cassert>
#include <iterator>
#include <span>
#include <vector>

#include "roq/core/codec/base64.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {
namespace tools {

// === HELPERS ===

namespace {
template <typename R>
R create_hmac(auto const &secret) {
  std::vector<std::byte> buffer;
  buffer.resize(core::codec::Base64::get_max_binary_length(std::size(secret)));
  auto raw_secret = core::codec::Base64::decode(buffer, secret, false, false);
  return R{raw_secret};
}
}  // namespace

// === IMPLEMENTATION ===

Crypto::Crypto(std::string_view const &secret) : mac_{create_hmac<decltype(mac_)>(secret)} {
}

std::string Crypto::create_headers(
    std::string_view const &path,
    std::string_view const &query,
    std::string_view const &key,
    std::chrono::milliseconds nonce) {
  assert(!std::empty(path));
  if (nonce.count()) {
    auto nonce_ = fmt::format("{}"sv, nonce.count());
    hash_.clear();
    if (!std::empty(query)) {
      assert(query[0] == '?');
      auto raw = query.substr(1);  // note! not including '?'
      hash_.update(raw);
    }
    hash_.update(nonce_);
    hash_.update(path);
    std::array<std::byte, Hash::DIGEST_LENGTH> buffer_1;
    auto digest_1 = hash_.final(buffer_1);
    mac_.clear();
    mac_.update(digest_1);
    auto digest_2 = mac_.final(digest_);
    std::string authent;
    core::codec::Base64::encode(authent, digest_2, false, false);
    return fmt::format(
        "APIKey: {}\r\n"
        "Nonce: {}\r\n"
        "Authent: {}\r\n"sv,
        key,
        nonce_,
        authent);
  } else {
    hash_.clear();
    if (!std::empty(query)) {
      assert(query[0] == '?');
      auto raw = query.substr(1);  // note! not including '?'
      hash_.update(raw);
    }
    hash_.update(path);
    std::array<std::byte, Hash::DIGEST_LENGTH> buffer_1;
    auto digest_1 = hash_.final(buffer_1);
    mac_.clear();
    mac_.update(digest_1);
    auto digest_2 = mac_.final(digest_);
    std::string authent;
    core::codec::Base64::encode(authent, digest_2, false, false);
    return fmt::format(
        "APIKey: {}\r\n"
        "Authent: {}\r\n"sv,
        key,
        authent);
  }
}

std::string Crypto::signed_challenge(std::string_view const &original_challenge) {
  hash_.clear();
  hash_.update(original_challenge);
  std::array<std::byte, Hash::DIGEST_LENGTH> buffer_1;
  auto digest_1 = hash_.final(buffer_1);
  mac_.clear();
  mac_.update(digest_1);
  auto digest_2 = mac_.final(digest_);
  std::string result;
  core::codec::Base64::encode(result, digest_2, false, false);
  return result;
}

}  // namespace tools
}  // namespace kraken_futures
}  // namespace roq

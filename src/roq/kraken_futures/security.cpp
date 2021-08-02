/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/security.h"

#include <algorithm>
#include <array>
#include <random>

#include "roq/logging.h"

#include "roq/utils/safe_cast.h"

#include "roq/core/clock.h"

#include "roq/core/binascii/base64.h"

#include "roq/core/crypto/sha.h"

using namespace std::literals;
using namespace roq::literals;

using namespace std::chrono_literals;

namespace roq {
namespace kraken_futures {

namespace {
static const constexpr auto THRESHOLD = -1000ms;

static auto create_hmac(const std::string_view &secret) {
  auto raw_secret = core::binascii::Base64::decode(secret, true);
  return core::crypto::HMAC_SHA512(raw_secret);
}
}  // namespace

Security::Security(const Config &config, const std::string_view &account)
    : account_(account), key_(config.get_access_key(account)),
      password_(config.get_access_password(account)),
      hmac_(create_hmac(config.get_access_secret(account))),
      nonce_(std::chrono::duration_cast<decltype(nonce_)>(core::get_realtime_clock())) {
}

Security::Security(
    const std::string_view &account,
    const std::string_view &key,
    const std::string_view &password,
    const std::string_view &secret)
    : account_(account), key_(key), password_(password), hmac_(create_hmac(secret)),
      nonce_(std::chrono::duration_cast<decltype(nonce_)>(core::get_realtime_clock())) {
}

std::string Security::create_headers(const std::string_view &path, const std::string_view &query) {
  assert(!path.empty());
  auto nonce = roq::format("{}"_sv, get_nonce().count());
  sha_.clear();
  if (!query.empty()) {
    assert(query[0] == '?');
    auto raw = query.substr(1);  // note! not including '?'
    sha_.update(raw);
  }
  sha_.update(nonce);
  sha_.update(path);
  std::array<char, 32> buffer_1;
  auto length_1 = sha_.digest(buffer_1);
  assert(length_1 == buffer_1.size());
  hmac_.clear();
  hmac_.update(buffer_1.data(), buffer_1.size());
  std::array<char, 64> buffer_2;
  auto length_2 = hmac_.digest(buffer_2);
  assert(length_2 == buffer_2.size());
  auto authent = core::binascii::Base64::encode(buffer_2);
  return roq::format(
      "APIKey: {}\r\n"
      "Nonce: {}\r\n"
      "Authent: {}\r\n"_sv,
      key_,
      nonce,
      authent);
}

std::string Security::signed_challenge(const std::string_view &original_challenge) {
  sha_.clear();
  sha_.update(original_challenge);
  std::array<char, 32> buffer_1;
  auto length_1 = sha_.digest(buffer_1);
  assert(length_1 == buffer_1.size());
  hmac_.clear();
  hmac_.update(buffer_1.data(), buffer_1.size());
  std::array<char, 64> buffer_2;
  auto length_2 = hmac_.digest(buffer_2);
  assert(length_2 == buffer_2.size());
  auto result = core::binascii::Base64::encode(buffer_2);
  return result;
}

std::chrono::milliseconds Security::get_nonce() {
  auto now = std::chrono::duration_cast<decltype(nonce_)>(core::get_realtime_clock());
  nonce_ = std::max(now, nonce_ + 1ms);  // note! can't reuse
  return nonce_;
}

}  // namespace kraken_futures
}  // namespace roq

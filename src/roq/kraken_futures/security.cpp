/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/security.h"

#include <array>
#include <random>

#include "roq/logging.h"

#include "roq/utils/safe_cast.h"

#include "roq/core/clock.h"

#include "roq/core/binascii/base64.h"

#include "roq/core/crypto/sha.h"

using namespace std::literals;
using namespace roq::literals;

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

std::string Security::create_headers(const std::string_view &path, const std::string_view &query) {
  assert(!path.empty());
  assert(!query.empty());
  auto nonce = roq::format("{}"_sv, (++nonce_).count());
  sha_.clear();
  sha_.update(query);
  sha_.update(nonce);
  sha_.update(path);
  std::array<char, 32> buffer_1;
  auto length_1 = sha_.digest(buffer_1);
  assert(length_1 == buffer_1.size());
  hmac_.clear();
  hmac_.update(path.data(), path.length());
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

}  // namespace kraken_futures
}  // namespace roq

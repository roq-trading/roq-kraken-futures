/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>

#include "roq/kraken_futures/config.hpp"

#include "roq/kraken_futures/tools/hasher.hpp"

namespace roq {
namespace kraken_futures {

class Security final {
 public:
  Security(Config const &, std::string_view const &account);

  Security(Security &&) = delete;
  Security(Security const &) = delete;

  std::string_view get_account() const { return account_; }
  std::string_view get_key() const { return key_; }

  std::string create_headers(std::string_view const &path, std::string_view const &body);

  std::string signed_challenge(std::string_view const &original_challenge);

 protected:
  std::chrono::milliseconds get_nonce();

 private:
  const std::string account_;
  const std::string key_;
  tools::Hasher hasher_;
  std::chrono::milliseconds nonce_ = {};
};

}  // namespace kraken_futures
}  // namespace roq

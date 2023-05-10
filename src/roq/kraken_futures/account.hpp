/* Copyright (c) 2017-2023, Hans Erik Thrane */

#pragma once

#include <chrono>
#include <string>

#include "roq/kraken_futures/config.hpp"

#include "roq/kraken_futures/tools/crypto.hpp"

namespace roq {
namespace kraken_futures {

struct Account final {
  Account(Config const &, std::string_view const &name, bool use_nonce);

  Account(Account &&) = delete;
  Account(Account const &) = delete;

  std::string_view get_name() const { return name_; }
  std::string_view get_key() const { return key_; }

  std::string create_headers(std::string_view const &path, std::string_view const &body);

  std::string signed_challenge(std::string_view const &original_challenge);

 protected:
  std::chrono::milliseconds get_nonce();

 private:
  std::string const name_;
  std::string const key_;
  tools::Crypto crypto_;
  std::chrono::milliseconds nonce_ = {};
  bool const use_nonce_;
};

}  // namespace kraken_futures
}  // namespace roq

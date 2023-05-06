/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/settings.hpp"

#include "roq/kraken_futures/flags/flags.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

Settings Settings::create(server::Type type) {
  auto settings = server::create_settings(type, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER);
  return {settings};
}

}  // namespace kraken_futures
}  // namespace roq

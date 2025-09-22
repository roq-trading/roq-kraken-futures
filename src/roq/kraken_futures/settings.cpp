/* Copyright (c) 2017-2025, Hans Erik Thrane */

#include "roq/kraken_futures/settings.hpp"

#include "roq/logging.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

Settings::Settings(args::Parser const &args)
    : server::flags::Settings{args, ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER}, flags::Flags{flags::Flags::create()}, misc{flags::Misc::create()},
      rest{flags::REST::create()}, ws{flags::WS::create()}, request{flags::Request::create()} {
  log::info("settings={}"sv, *this);
}

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2023, Hans Erik Thrane */

#include "roq/kraken_futures/application.hpp"

#include "roq/kraken_futures/config.hpp"
#include "roq/kraken_futures/gateway.hpp"
#include "roq/kraken_futures/settings.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === CONSTANTS ===

namespace {
auto const TYPE = server::Type::ORDER_MANAGEMENT;
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  Settings settings{args, TYPE};
  Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading<Gateway>{settings, config, *context}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

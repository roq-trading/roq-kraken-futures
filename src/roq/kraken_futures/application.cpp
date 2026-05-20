/* Copyright (c) 2017-2026, Hans Erik Thrane */

#include "roq/kraken_futures/application.hpp"

#include "roq/kraken_futures/flags/settings.hpp"

#include "roq/kraken_futures/gateway/config.hpp"
#include "roq/kraken_futures/gateway/controller.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

// === CONSTANTS ===

namespace {
uint8_t const API_2 = {};
}

// === IMPLEMENTATION ===

int Application::main(args::Parser const &args) {
  flags::Settings settings{args};
  gateway::Config config{settings};
  auto context = server::create_io_context(settings);
  server::Trading2<gateway::Controller>{settings, config, *context, API_2}.dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/application.h"

#include "roq/kraken_futures/config.h"
#include "roq/kraken_futures/flags.h"
#include "roq/kraken_futures/gateway.h"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Starting the gateway"sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, ROQ_BUILD_NUMBER, {}, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

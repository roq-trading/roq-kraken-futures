/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/application.h"

#include "roq/kraken_futures/config.h"
#include "roq/kraken_futures/flags.h"
#include "roq/kraken_futures/gateway.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"_fmt, Flags::config_file());
  Config config(Flags::config_file());
  log::trace_1("config={}"_fmt, config);
  log::info("Starting the gateway"_sv);
  roq::server::Trading<Gateway>(ROQ_PACKAGE_NAME, config, server::RequestIdType::SEQUENTIAL, config)
      .dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

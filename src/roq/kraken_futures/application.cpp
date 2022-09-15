/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/application.hpp"

#include "roq/kraken_futures/config.hpp"
#include "roq/kraken_futures/flags.hpp"
#include "roq/kraken_futures/gateway.hpp"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

int Application::main(int, char **) {
  log::info(R"(Parse config_file="{}")"sv, Flags::config_file());
  Config config(Flags::config_file(), Flags::secrets_file());
  log::info<1>("config={}"sv, config);
  log::info("Starting the gateway"sv);
  server::Settings settings{
      .package_name = ROQ_PACKAGE_NAME,
      .build_number = ROQ_BUILD_NUMBER,
      .api = {},
      .type = server::Type::ORDER_MANAGEMENT,
  };
  server::Trading<Gateway>(settings, config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

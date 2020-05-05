/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/kraken-futures/application.h"

#include "roq/kraken-futures/config.h"
#include "roq/kraken-futures/gateway.h"
#include "roq/kraken-futures/options.h"

namespace roq {
namespace kraken_futures {

int Application::main(int, char **) {
  LOG(INFO)("Parse configuration");
  Config config(FLAGS_config_file);
  VLOG(1)(FMT_STRING("config={}"), config);
  LOG(INFO)("Starting the gateway");
  roq::server::Trading<Gateway>(
      PACKAGE_NAME,
      config,
      FLAGS_listen,
      server::RequestIdType::SEQUENTIAL,
      config).dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

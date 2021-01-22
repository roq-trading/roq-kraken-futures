/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/kraken_futures/application.h"

#include <absl/flags/flag.h>

#include "roq/kraken_futures/config.h"
#include "roq/kraken_futures/gateway.h"
#include "roq/kraken_futures/options.h"

namespace roq {
namespace kraken_futures {

int Application::main(int, char **) {
  LOG(INFO)("Parse configuration");
  Config config(absl::GetFlag(FLAGS_config_file));
  VLOG(1)("config={}", config);
  LOG(INFO)("Starting the gateway");
  roq::server::Trading<Gateway>(
      ROQ_PACKAGE_NAME, config, server::RequestIdType::SEQUENTIAL, config)
      .dispatch();
  return EXIT_SUCCESS;
}

}  // namespace kraken_futures
}  // namespace roq

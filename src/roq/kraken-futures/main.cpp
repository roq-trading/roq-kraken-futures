/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/kraken-futures/application.h"

namespace {
constexpr std::string_view DESCRIPTION = "Roq HitBTC Gateway";
}  // namespace

int main(int argc, char **argv) {
  return roq::kraken_futures::Application(
      argc,
      argv,
      DESCRIPTION,
      VERSION).run();
}

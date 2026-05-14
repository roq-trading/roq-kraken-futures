/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace kraken_futures {

enum class RestState : uint8_t {
  UNDEFINED = 0,
  INSTRUMENTS,
  DONE,
};

}  // namespace kraken_futures
}  // namespace roq

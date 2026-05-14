/* Copyright (c) 2017-2026, Hans Erik Thrane */

#pragma once

#include <cstdint>

namespace roq {
namespace kraken_futures {

enum class DropCopyState : uint8_t {
  UNDEFINED = 0,
  GET_CHALLENGE,
  SUBSCRIBE,
  DONE,
};

}  // namespace kraken_futures
}  // namespace roq

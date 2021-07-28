/* Copyright (c) 2017-2021, Hans Erik Thrane */

#pragma once

#include <string_view>

#include "roq/format.h"
#include "roq/literals.h"

#include "roq/core/json/parser.h"

#ifdef VERSION
#undef VERSION
#endif

namespace roq {
namespace kraken_futures {
namespace json {

struct Result final {
  enum type_t { UNDEFINED, UNKNOWN, SUCCESS, ERROR };

  constexpr Result() = default;

  // cppcheck-suppress noExplicitConstructor
  constexpr Result(type_t type) : type_(type) {}  // NOLINT (allow implicit)

  explicit Result(const core::json::value_t &);

  explicit Result(const std::string_view &name);

  Result &operator=(const std::string_view &name);

  constexpr operator type_t() const { return type_; }

  std::string_view as_text() const;
  std::string_view as_raw_text() const;

 private:
  type_t type_ = {};
};

}  // namespace json
}  // namespace kraken_futures
}  // namespace roq

template <>
struct fmt::formatter<roq::kraken_futures::json::Result> : public roq::formatter {
  template <typename Context>
  auto format(const roq::kraken_futures::json::Result &value, Context &context) {
    using namespace roq::literals;
    return roq::format_to(context.out(), R"({})"_sv, value.as_text());
  }
};

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include <gtest/gtest.h>

#include "roq/literals.h"

#include "roq/kraken_futures/tools/hasher.h"

using namespace roq::literals;

using namespace std::chrono_literals;  // NOLINT

using namespace roq;
using namespace roq::kraken_futures;

// https://support.kraken.com/hc/en-us/articles/360022839451-Generate-API-keys
// https://support.kraken.com/hc/en-us/articles/360022635592-Generate-authentication-strings-REST-API-

TEST(hasher, headers_without_nonce) {
  const auto KEY = "rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf"_sv;
  const auto SECRET =
      "rttp4AzwRfYEdQ7R7X8Z/04Y4TZPa97pqCypi3xXxAqftygftnI6H9yGV+OcUOOJeFtZkr8mVwbAndU3Kz4Q+eG"_sv;
  tools::Hasher hasher(SECRET);
  auto result =
      hasher.create_headers("/api/v3/orderbook"_sv, "?symbol=fi_xbtusd_180615"_sv, KEY, {});
  EXPECT_EQ(
      result,
      "APIKey: rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf\r\n"
      "Authent: BGOdiF//YXbOtKUkyFFRqKAft7gai33YfScxFrXMdMHGUJ6wSaMA6y0p6UzfYzj5Flgvv+SFQe53h2KrEe37Ng==\r\n"_sv);
}

TEST(hasher, headers_with_nonce) {
  const auto KEY = "rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf"_sv;
  const auto SECRET =
      "rttp4AzwRfYEdQ7R7X8Z/04Y4TZPa97pqCypi3xXxAqftygftnI6H9yGV+OcUOOJeFtZkr8mVwbAndU3Kz4Q+eG"_sv;
  tools::Hasher hasher(SECRET);
  auto result = hasher.create_headers(
      "/api/v3/orderbook"_sv, "?symbol=fi_xbtusd_180615"_sv, KEY, 1415957147987ms);
  EXPECT_EQ(
      result,
      "APIKey: rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf\r\n"
      "Nonce: 1415957147987\r\n"
      "Authent: DqUyz8Wh/72af7dimSXHw91IFxrAriTgVodyg2s67PU2mVStwLDQak+uIoCtfb43XONq0xVAp+vm5dqnhFAB1Q==\r\n"_sv);
}

TEST(hasher, test_url_encoded) {
  const auto KEY = "zO4Zjh5GO+Zy2n4QwIyWEKb/EAgxSjOdtA+W1zZvOBYU3na2pE/p/nwx"_sv;
  const auto SECRET =
      "DuWQ3317vPm8tlOoylWmhfQJwGkKiR+ABuNNocaPxU2DA5H9aZAKkGO0sYBSGKirtdlkm8dL9DAp6husEABgWyOr"_sv;
  const auto PATH = "/api/v3/sendorder"_sv;
  const auto QUERY =
      R"(?orderType=lmt&symbol=PI_XBTUSD&side=buy&size=1&limitPrice=44587.5&cliOrdId=123%2B%2F%3D&reduceOnly=false)"_sv;
  tools::Hasher hasher(SECRET);
  auto result = hasher.create_headers(PATH, QUERY, KEY, 1629361663947ms);
  EXPECT_EQ(
      result,
      "APIKey: zO4Zjh5GO+Zy2n4QwIyWEKb/EAgxSjOdtA+W1zZvOBYU3na2pE/p/nwx\r\n"
      "Nonce: 1629361663947\r\n"
      "Authent: YU08zbwSTBmsVOSQQh8EPPW6eAZUmqgpRy6t5EmO2j/5aEyX3LTw1G2H+CP0dqUWOhLIFoQAXuZQcK5vU/B/qQ==\r\n"_sv);
}

/*
YU08zbwSTBmsVOSQQh8EPPW6eAZUmqgpRy6t5EmO2j/5aEyX3LTw1G2H+CP0dqUWOhLIFoQAXuZQcK5vU/B/qQ==
*/

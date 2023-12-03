/* Copyright (c) 2017-2024, Hans Erik Thrane */

#include <chrono>
#include <string_view>

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "roq/utils/hash/sha256.hpp"

#include "roq/kraken_futures/tools/crypto.hpp"

using namespace std::literals;

using namespace std::chrono_literals;  // NOLINT

using namespace roq;
using namespace roq::kraken_futures;

using namespace Catch::literals;

// https://support.kraken.com/hc/en-us/articles/360022839451-Generate-API-keys
// https://support.kraken.com/hc/en-us/articles/360022635592-Generate-authentication-strings-REST-API-

/* note! this example includes an invalid secret -- length is 87 but should be 88

TEST_CASE("crypto_headers_without_nonce", "[crypto]") {
  auto const KEY = "rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf"sv;
  auto const SECRET = "rttp4AzwRfYEdQ7R7X8Z/04Y4TZPa97pqCypi3xXxAqftygftnI6H9yGV+OcUOOJeFtZkr8mVwbAndU3Kz4Q+eG"sv;
  tools::Crypto crypto{SECRET};
  auto result = crypto.create_headers("/api/v3/orderbook"sv, "?symbol=fi_xbtusd_180615"sv, KEY, {});
  CHECK(
      result ==
      "APIKey: rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf\r\n"
      "Authent: BGOdiF//YXbOtKUkyFFRqKAft7gai33YfScxFrXMdMHGUJ6wSaMA6y0p6UzfYzj5Flgvv+SFQe53h2KrEe37Ng==\r\n"sv);
}

TEST_CASE("crypto_headers_with_nonce", "[crypto]") {
  auto const KEY = "rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf"sv;
  auto const SECRET = "rttp4AzwRfYEdQ7R7X8Z/04Y4TZPa97pqCypi3xXxAqftygftnI6H9yGV+OcUOOJeFtZkr8mVwbAndU3Kz4Q+eG"sv;
  tools::Crypto crypto{SECRET};
  auto result = crypto.create_headers("/api/v3/orderbook"sv, "?symbol=fi_xbtusd_180615"sv, KEY, 1415957147987ms);
  CHECK(
      result ==
      "APIKey: rRra59qKQs3y1egAgSaG0RJlBcbq97wTUXSxXxPdhRZdv7z9ijZRWgrf\r\n"
      "Nonce: 1415957147987\r\n"
      "Authent: DqUyz8Wh/72af7dimSXHw91IFxrAriTgVodyg2s67PU2mVStwLDQak+uIoCtfb43XONq0xVAp+vm5dqnhFAB1Q==\r\n"sv);
}
*/

TEST_CASE("crypto_test_url_encoded", "[crypto]") {
  auto const KEY = "zO4Zjh5GO+Zy2n4QwIyWEKb/EAgxSjOdtA+W1zZvOBYU3na2pE/p/nwx"sv;
  auto const SECRET = "DuWQ3317vPm8tlOoylWmhfQJwGkKiR+ABuNNocaPxU2DA5H9aZAKkGO0sYBSGKirtdlkm8dL9DAp6husEABgWyOr"sv;
  auto const PATH = "/api/v3/sendorder"sv;
  auto const QUERY =
      R"(?orderType=lmt&symbol=PI_XBTUSD&side=buy&size=1&limitPrice=44587.5&cliOrdId=123%2B%2F%3D&reduceOnly=false)"sv;
  tools::Crypto crypto{SECRET};
  auto result = crypto.create_headers(PATH, QUERY, KEY, 1629361663947ms);
  CHECK(
      result ==
      "APIKey: zO4Zjh5GO+Zy2n4QwIyWEKb/EAgxSjOdtA+W1zZvOBYU3na2pE/p/nwx\r\n"
      "Nonce: 1629361663947\r\n"
      "Authent: YU08zbwSTBmsVOSQQh8EPPW6eAZUmqgpRy6t5EmO2j/5aEyX3LTw1G2H+CP0dqUWOhLIFoQAXuZQcK5vU/B/qQ==\r\n"sv);
}

/*
YU08zbwSTBmsVOSQQh8EPPW6eAZUmqgpRy6t5EmO2j/5aEyX3LTw1G2H+CP0dqUWOhLIFoQAXuZQcK5vU/B/qQ==
*/

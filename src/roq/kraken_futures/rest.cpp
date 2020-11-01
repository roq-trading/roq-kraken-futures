/* Copyright (c) 2017-2020, Hans Erik Thrane */

#include "roq/kraken_futures/rest.h"

#include <fmt/chrono.h>
#include <fmt/format.h>

#include "roq/core/json/parser.h"

#include "roq/kraken_futures/gateway.h"
#include "roq/kraken_futures/options.h"

#include "roq/kraken_futures/json/utils.h"

#include "roq/kraken_futures/json/asset_pairs.h"

namespace roq {
namespace kraken_futures {

namespace {
constexpr std::string_view CONNECTION = "rest";

static const std::string_view ACCEPT_JSON{"application/json"};

static auto create_counter(const std::string_view &function) {
  return core::metrics::Counter(FLAGS_name, CONNECTION, function);
}

static auto create_profile(const std::string_view &function) {
  return core::metrics::Profile(FLAGS_name, CONNECTION, function);
}

static auto create_latency(const std::string_view &function) {
  return core::metrics::Latency(FLAGS_name, CONNECTION, function);
}
}  // namespace

Rest::Rest(
    Gateway &gateway,
    [[maybe_unused]] const Config &config,
    Random &random,
    core::event::Base &base,
    core::event::DNSBase &dns_base,
    core::ssl::Context &ssl_context)
    : _gateway(gateway), _random(random),
      _connection(
          *this,
          base,
          dns_base,
          ssl_context,
          core::URI(FLAGS_rest_uri),
          ROQ_PACKAGE_NAME,
          true,  // keep alive
          FLAGS_request_queue_depth,
          std::chrono::seconds{FLAGS_request_timeout_secs},
          std::chrono::seconds{FLAGS_rate_limit_interval_secs},
          FLAGS_rate_limit_max_requests,
          std::chrono::seconds{FLAGS_ping_freq_secs},
          FLAGS_decode_buffer_size,
          FLAGS_encode_buffer_size,
          FLAGS_rest_ping_path),
      _decode_buffer(FLAGS_decode_buffer_size),
      _counter{
          .disconnect = create_counter("disconnect"),
      },
      _profile{
          .asset_pairs = create_profile("asset_pairs"),
      },
      _latency{
          .ping = create_latency("ping"),
      } {
}

bool Rest::ready() const {
  return _connection.ready();
}

void Rest::close() {
  _connection.close();
}

void Rest::operator()(const Event<Start> &) {
  _connection.start();
}

void Rest::operator()(const Event<Stop> &) {
  _connection.stop();
}

void Rest::operator()(const Event<Timer> &event) {
  _connection.refresh(event.value.now);
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(_counter.disconnect, metrics::COUNTER)
      // profile
      .write(_profile.asset_pairs, metrics::PROFILE)
      // latency
      .write(_latency.ping, metrics::LATENCY);
}

void Rest::get_asset_pairs(
    std::function<void(const core::web::Response &)> &&callback) {
  _connection.request(
      core::http::Method::GET,
      "/public/AssetPairs",
      std::string_view(),  // query
      ACCEPT_JSON,
      std::string_view(),  // content_type
      std::string_view(),  // headers
      std::string_view(),  // body
      [this, callback](auto &response) {
        if (response.success()) {
          auto [status, body] = response.get();
          if (status == core::http::Status::OK) {
            _profile.asset_pairs([&]() {
              core::json::Buffer buffer(_decode_buffer);
              auto asset_pairs =
                  core::json::Parser::create<json::AssetPairs>(body, buffer);
              VLOG(1)(R"(asset_pairs={})", asset_pairs);
              _gateway(asset_pairs);
            });
          }
        }
        callback(response);
      });
}

void Rest::operator()(const core::web::Client::Connected &) {
  _gateway(*this);
}

void Rest::operator()(const core::web::Client::Disconnected &) {
  ++_counter.disconnect;
  _gateway(*this);
}

void Rest::operator()(const core::web::Client::Latency &latency) {
  _latency.ping.update(
      std::chrono::duration_cast<std::chrono::nanoseconds>(latency.sample)
          .count());
}

}  // namespace kraken_futures
}  // namespace roq

/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/web_socket.h"

#include <fmt/format.h>

#include "roq/core/clock.h"

#include "roq/kraken_futures/flags.h"
#include "roq/kraken_futures/gateway.h"

#include "roq/kraken_futures/json/parser.h"

namespace roq {
namespace kraken_futures {

namespace {
constexpr std::string_view CONNECTION = "ws";

static auto create_counter(const std::string_view &function) {
  return core::metrics::Counter(Flags::name(), CONNECTION, function);
}

static auto create_profile(const std::string_view &function) {
  return core::metrics::Profile(Flags::name(), CONNECTION, function);
}

static auto create_latency(const std::string_view &function) {
  return core::metrics::Latency(Flags::name(), CONNECTION, function);
}
}  // namespace

WebSocket::WebSocket(
    Gateway &gateway,
    const Config &config,
    Random &random,
    core::event::Base &base,
    core::event::DNSBase &dns_base,
    core::ssl::Context &ssl_context)
    : _gateway(gateway), _access_key(config.get_access_key()), _random(random),
      _connection(
          *this,
          base,
          dns_base,
          ssl_context,
          core::URI(Flags::ws_uri()),
          std::string_view(),  // query
          std::chrono::seconds{Flags::ping_freq_secs()},
          Flags::decode_buffer_size(),  // XXX need read buffer size
          Flags::encode_buffer_size(),
          []() { return std::string(); }),
      _decode_buffer(Flags::decode_buffer_size()),
      _counter{
          .disconnect = create_counter("disconnect"),
      },
      _profile{
          .parse = create_profile("parse"),
      },
      _latency{
          .ping = create_latency("ping"),
          .heartbeat = create_latency("heartbeat"),
      } {
}

bool WebSocket::ready() const {
  return _connection.ready();
}

void WebSocket::close() {
  _connection.close();
}

void WebSocket::operator()(const Event<Start> &) {
  _connection.start();
}

void WebSocket::operator()(const Event<Stop> &) {
  _connection.stop();
}

void WebSocket::operator()(const Event<Timer> &event) {
  _connection.refresh(event.value.now);
}

void WebSocket::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(_counter.disconnect, metrics::COUNTER)
      // profile
      .write(_profile.parse, metrics::PROFILE)
      // latency
      .write(_latency.ping, metrics::LATENCY)
      .write(_latency.heartbeat, metrics::LATENCY);
}

template <>
void WebSocket::subscribe(const std::string_view &name, const roq::span<std::string> &pairs) {
  LOG(INFO)(R"(subscribe name="{}", len(pairs)={})", name, std::size(pairs));
  if (Flags::book_depth() && name.compare("book") == 0) {
    auto message = fmt::format(
        R"({{)"
        R"("event":"subscribe",)"
        R"("pair":["{}"],)"
        R"("subscription":{{)"
        R"("name":"{}",)"
        R"("depth":{})"
        R"(}})"
        R"(}})",
        fmt::join(pairs, R"(",")"),
        name,
        Flags::book_depth());
    DLOG(INFO)(R"(request="{}")", message);
    _connection.send_text(message);
  } else {
    auto message = fmt::format(
        R"({{)"
        R"("event":"subscribe",)"
        R"("pair":["{}"],)"
        R"("subscription":{{)"
        R"("name":"{}")"
        R"(}})"
        R"(}})",
        fmt::join(pairs, R"(",")"),
        name);
    DLOG(INFO)(R"(request="{}")", message);
    _connection.send_text(message);
  }
}

void WebSocket::operator()(const core::web::Socket::Connected &) {
  // note! wait for upgrade
}

void WebSocket::operator()(const core::web::Socket::Disconnected &) {
  ++_counter.disconnect;
  _gateway(*this);
}

void WebSocket::operator()(const core::web::Socket::Ready &) {
  LOG(INFO)("Ready");
  _gateway(*this);
}

void WebSocket::operator()(const core::web::Socket::Close &) {
}

void WebSocket::operator()(const core::web::Socket::Latency &latency) {
  _latency.ping.update(latency.sample);
}

void WebSocket::operator()(const core::web::Socket::Text &text) {
  parse(text.payload);
}

void WebSocket::parse(const std::string_view &message) {
  _profile.parse([&]() {
    core::json::Buffer buffer(_decode_buffer);
    auto result = json::Parser::dispatch(*this, message, buffer);
  });
}

void WebSocket::operator()(const json::Error &error) {
  LOG(FATAL)("error={}", error);
}

void WebSocket::operator()(const json::SystemStatus &system_status) {
  LOG(INFO)("system_status={}", system_status);
}

void WebSocket::operator()(const json::Pong &pong) {
  VLOG(1)("pong={}", pong);
}

void WebSocket::operator()(const json::Heartbeat &heartbeat) {
  VLOG(1)("heartbeat={}", heartbeat);
}

void WebSocket::operator()(const json::SubscriptionStatus &subscription_status) {
  VLOG(1)("subscription_status={}", subscription_status);
}

void WebSocket::operator()(const json::Trade &trade, const std::string_view &pair) {
  VLOG(3)(R"(trade={}, pair="{}")", trade, pair);
  _gateway(trade, pair);
}

void WebSocket::operator()(const json::Spread &spread, const std::string_view &pair) {
  VLOG(3)(R"(spread={}, pair="{}")", spread, pair);
  _gateway(spread, pair);
}

void WebSocket::operator()(const json::Book &book, const std::string_view &pair) {
  VLOG(3)(R"(book={}, pair="{}")", book, pair);
  _gateway(book, pair);
}

}  // namespace kraken_futures
}  // namespace roq

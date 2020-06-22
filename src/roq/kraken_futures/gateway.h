/* Copyright (c) 2017-2020, Hans Erik Thrane */

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "roq/server.h"
#include "roq/download.h"

#include "roq/core/hash/map.h"
#include "roq/core/hash/set.h"

#include "roq/core/ssl/ssl.h"

#include "roq/core/event/base.h"
#include "roq/core/event/dns_base.h"

#include "roq/kraken_futures/config.h"
#include "roq/kraken_futures/random.h"
#include "roq/kraken_futures/rest.h"
#include "roq/kraken_futures/web_socket.h"

#include "roq/kraken_futures/web_socket_state.h"

#include "roq/kraken_futures/json/asset_pairs.h"

namespace roq {
namespace kraken_futures {

class Gateway final : public server::Handler {
 public:
  Gateway(
      server::Dispatcher& dispatcher,
      const Config& config);

  void operator()(const server::StartEvent&) override;
  void operator()(const server::StopEvent&) override;
  void operator()(const server::TimerEvent&) override;
  void operator()(const server::ConnectionStatusEvent&) override;

  void operator()(
      const CreateOrderEvent& event,
      const std::string_view& request_id,
      uint32_t gateway_order_id) override;
  void operator()(
      const ModifyOrderEvent& event,
      const std::string_view& request_id,
      const server::OMS_Order& order) override;
  void operator()(
      const CancelOrderEvent& event,
      const std::string_view& request_id,
      const server::OMS_Order& order) override;

  void operator()(metrics::Writer& writer) override;

  // rest
  void operator()(const Rest&);

  void operator()(const json::AssetPairs&);

  // web socket
  void operator()(const WebSocket&);

  void operator()(
      const json::Trade& trade,
      const std::string_view& pair);
  void operator()(
      const json::Spread& spread,
      const std::string_view& pair);
  void operator()(
      const json::Book& book,
      const std::string_view& pair);

 private:
  using WebSocketDownload = server::Download<WebSocketState>;

  int32_t download(WebSocketDownload::State state);

 private:
  void update(GatewayStatus gateway_status);

  void download_asset_pairs();

  void subscribe();

  template <typename T>
  void enqueue(
      const T& value,
      const server::Trace& trace,
      bool is_last);

  template <typename T>
  void enqueue(
      uint8_t user_id,
      const T& value,
      const server::Trace& trace,
      bool is_last);

 private:
  server::Dispatcher& _dispatcher;
  // config
  const std::string _account;
  const std::string _access_key;
  // authentication
  Random _random;
  // async
  core::event::Base _base;
  core::event::DNSBase _dns_base;
  // crypto
  core::ssl::Context _ssl_context;
  // connections
  struct {
    WebSocket connection;
    WebSocketDownload download;
  } _web_socket;
  struct {
    Rest connection;
  } _rest;
  // download (web socket)
  std::vector<std::string> _symbols;
  // market data + order manager
  GatewayStatus _gateway_status = GatewayStatus::DISCONNECTED;
  // market data
  core::page_aligned_vector<MBPUpdate> _bid, _ask;
  core::page_aligned_vector<Trade> _trade;
};

}  // namespace kraken_futures
}  // namespace roq

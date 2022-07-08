/* Copyright (c) 2017-2022, Hans Erik Thrane */

#pragma once

#include <absl/container/flat_hash_map.h>

#include <memory>
#include <string>
#include <vector>

#include "roq/server.hpp"

#include "roq/core/io/event_context.hpp"

#include "roq/kraken_futures/config.hpp"
#include "roq/kraken_futures/drop_copy.hpp"
#include "roq/kraken_futures/market_data.hpp"
#include "roq/kraken_futures/order_entry.hpp"
#include "roq/kraken_futures/rest.hpp"
#include "roq/kraken_futures/security.hpp"
#include "roq/kraken_futures/shared.hpp"

namespace roq {
namespace kraken_futures {

class Gateway final : public server::Handler,
                      public Rest::Handler,
                      public OrderEntry::Handler,
                      public MarketData::Handler,
                      public DropCopy::Handler {
 public:
  Gateway(server::Dispatcher &, Config const &);

 protected:
  // server::Handler

  void operator()(Event<Start> const &) override;
  void operator()(Event<Stop> const &) override;
  void operator()(Event<Timer> const &) override;
  void operator()(Event<Connected> const &) override;
  void operator()(Event<Disconnected> const &) override;

  uint16_t operator()(Event<CreateOrder> const &, oms::Order const &, std::string_view const &request_id) override;
  uint16_t operator()(
      Event<ModifyOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;
  uint16_t operator()(
      Event<CancelOrder> const &,
      oms::Order const &,
      std::string_view const &request_id,
      std::string_view const &previous_request_id) override;

  uint16_t operator()(Event<CancelAllOrders> const &, std::string_view const &request_id) override;

  void operator()(metrics::Writer &) override;

  void operator()(Trace<StreamStatus const> const &) override;
  void operator()(Trace<ExternalLatency const> const &) override;
  void operator()(Trace<ReferenceData const> const &, bool is_last) override;
  void operator()(Trace<MarketStatus const> const &, bool is_last) override;
  void operator()(Trace<TopOfBook const> const &, bool is_last) override;
  void operator()(Trace<MarketByPriceUpdate const> const &, bool is_last, bool refresh) override;
  void operator()(Trace<TradeSummary const> const &, bool is_last) override;
  void operator()(Trace<StatisticsUpdate const> const &, bool is_last) override;
  void operator()(Trace<TradeUpdate const> const &, bool is_last, uint8_t user_id) override;
  void operator()(Trace<FundsUpdate const> const &, bool is_last) override;
  void operator()(Trace<PositionUpdate const> const &, bool is_last) override;

  void operator()(Rest::SymbolsUpdate &) override;

  void ensure_symbol_slices(size_t size);

  // utilities

  OrderEntry &get_order_entry(std::string_view const &account);

 private:
  server::Dispatcher &dispatcher_;
  // config
  const std::string master_account_;
  // security
  absl::flat_hash_map<Account, std::unique_ptr<Security>> security_;
  // io
  core::io::EventContext context_;
  // shared
  Shared shared_;
  // seed
  uint16_t stream_id_ = {};
  // streams
  Rest rest_;
  absl::flat_hash_map<Account, std::unique_ptr<OrderEntry>> order_entry_;
  absl::flat_hash_map<Account, std::unique_ptr<DropCopy>> drop_copy_;
  std::vector<std::unique_ptr<MarketData>> market_data_;
};

}  // namespace kraken_futures
}  // namespace roq

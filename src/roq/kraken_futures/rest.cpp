/* Copyright (c) 2017-2021, Hans Erik Thrane */

#include "roq/kraken_futures/rest.h"

#include <algorithm>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"

#include "roq/kraken_futures/json/result.h"

using namespace roq::literals;

namespace roq {
namespace kraken_futures {

namespace {
static const auto NAME = "rest"_sv;

static const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

static const auto ALLOW_PIPELINING = true;

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};
}  // namespace

Rest::Rest(Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"_sv, stream_id_, NAME)),
      connection_(
          *this,
          context,
          Flags::decode_buffer_size(),
          Flags::encode_buffer_size(),
          core::URI(Flags::rest_uri()),
          ROQ_PACKAGE_NAME,
          core::http::Connection::KEEP_ALIVE,
          ALLOW_PIPELINING,
          Flags::rest_request_timeout(),
          Flags::rest_rate_limit_interval(),
          Flags::rest_rate_limit_max_requests(),
          Flags::rest_ping_freq(),
          Flags::rest_ping_path()),
      decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"_sv),
      },
      profile_{
          .instruments = create_metrics(name_, "instruments"_sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"_sv),
      },
      shared_(shared),
      download_(Flags::rest_request_timeout(), [this](auto state) { return download(state); }) {
}

void Rest::operator()(const Event<Start> &) {
  connection_.start();
}

void Rest::operator()(const Event<Stop> &) {
  connection_.stop();
}

void Rest::operator()(const Event<Timer> &event) {
  connection_.refresh(event.value.now);
}

void Rest::operator()(metrics::Writer &writer) {
  writer
      // counter
      .write(counter_.disconnect, metrics::COUNTER)
      // profile
      .write(profile_.instruments, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    server::TraceInfo trace_info;
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"_sv, stream_status);
    server::create_trace_and_dispatch(trace_info, stream_status, handler_);
  }
}

template <>
void Rest::get(std::function<void(const core::Promise<json::Instruments> &)> &&callback) {
  core::web::Request request{
      .method = core::http::Method::GET,
      .path = "/api/v3/instruments"_sv,
      .query = {},
      .accept = core::http::Accept::JSON,
      .content_type = {},
      .headers = {},
      .body = {},
      .quality_of_service = {},
      .rate_limit_weight = 1,
  };
  connection_(
      "instruments"_sv,
      request,
      [this, callback{std::move(callback)}]([[maybe_unused]] auto &request_id, auto &response) {
        profile_.instruments([&]() {
          try {
            response.expect(core::http::Status::OK);
            core::json::Buffer buffer(decode_buffer_);
            auto instruments =
                core::json::Parser::create<json::Instruments>(response.body(), buffer);
            if (instruments.error.empty()) {
              log::info<2>("instruments={}"_sv, instruments);
              core::Promise<json::Instruments> promise(instruments);
              callback(promise);
            } else {
              log::warn("instruments={}"_sv, instruments);
              log::fatal("Unexpected"_sv);
            }
          } catch (core::NetworkError &e) {
            log::warn(R"(Exception type={}, what="{}")"_sv, typeid(e).name(), e.what());
            core::Promise<json::Instruments> promise(std::current_exception());
            callback(promise);
          }
        });
      });
}

void Rest::operator()(const core::web::Client::Connected &) {
  if (download_.downloading()) {
    download_.bump();
  } else {
    (*this)(ConnectionStatus::DOWNLOADING);
    download_.begin();
  }
}

void Rest::operator()(const core::web::Client::Disconnected &) {
  ++counter_.disconnect;
  (*this)(ConnectionStatus::DISCONNECTED);
  if (!download_.downloading())
    download_.reset();
}

void Rest::operator()(const core::web::Client::Latency &latency) {
  server::TraceInfo trace_info;
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(trace_info, external_latency, handler_);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    case RestState::UNDEFINED:
      assert(false);
      break;
    case RestState::INSTRUMENTS:
      download_instruments();
      return 1;
    case RestState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

void Rest::download_instruments() {
  constexpr auto state = RestState::INSTRUMENTS;
  auto sequence = download_.sequence();
  get<json::Instruments>([this, sequence](auto &promise) {
    try {
      if (download_.skip(sequence, state))
        return;
      (*this)(promise.get());
      download_.check(state);
    } catch (core::NetworkError &) {
      download_.retry(state);
    }
  });
}

void Rest::operator()(const json::Instruments &instruments) {
  assert(instruments.error.empty());
  server::TraceInfo trace_info;  // XXX not correct (*parsing* already done)
  std::vector<std::string> symbols;
  symbols.reserve(instruments.instruments.size());
  size_t counter = {};
  for (auto &item : instruments.instruments) {
    log::info<2>("item={}"_sv, item);
    std::string symbol(item.symbol);  // note! we need the upper-case version
    std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::toupper);
    if (shared_.discard_symbol(symbol))
      continue;
    if (all_symbols_.emplace(symbol).second)  // only include new
      symbols.emplace_back(symbol);
    ++counter;
    ReferenceData reference_data{
        .stream_id = stream_id_,
        .exchange = Flags::exchange(),
        .symbol = symbol,
        .description = {},
        .security_type = {},
        .base_currency = {},
        .quote_currency = {},
        .commission_currency = {},
        .tick_size = item.tick_size,
        .multiplier = item.contract_size,
        .min_trade_vol = 1.0,  // XXX check
        .max_trade_vol = NaN,
        .trade_vol_step_size = 1.0,  // XXX check
        .option_type = {},
        .strike_currency = {},
        .strike_price = NaN,
        .underlying = {},
        .time_zone = {},
        .issue_date = {},
        .settlement_date = {},
        .expiry_datetime = {},
        .expiry_datetime_utc = {},
    };
    server::create_trace_and_dispatch(trace_info, reference_data, handler_, true);
  }
  log::info("Instruments {} / {}"_sv, counter, instruments.instruments.size());
  if (!symbols.empty()) {
    SymbolsUpdate symbols_update{
        .symbols = symbols,
    };
    handler_(symbols_update);
  }
}

}  // namespace kraken_futures
}  // namespace roq

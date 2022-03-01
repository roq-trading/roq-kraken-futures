/* Copyright (c) 2017-2022, Hans Erik Thrane */

#include "roq/kraken_futures/rest.h"

#include <algorithm>
#include <utility>

#include "roq/utils/mask.h"
#include "roq/utils/update.h"

#include "roq/core/json/parser.h"

#include "roq/core/metrics/factory.h"

#include "roq/kraken_futures/flags.h"

#include "roq/kraken_futures/json/result.h"

using namespace std::literals;

namespace roq {
namespace kraken_futures {

namespace {
const auto NAME = "rest"sv;

const auto SUPPORTS = utils::Mask{
    SupportType::REFERENCE_DATA,
    SupportType::MARKET_STATUS,
};

struct create_metrics final : public core::metrics::Factory {
  explicit create_metrics(const std::string_view &group, const std::string_view &function)
      : core::metrics::Factory(server::Flags::name(), group, function) {}
};

auto create_connection(auto &handler, auto &context) {
  auto uri = Flags::rest_uri();
  core::web::Client::Config config{
      .decode_buffer_size = Flags::decode_buffer_size(),
      .encode_buffer_size = Flags::encode_buffer_size(),
      .validate_certificate = server::Flags::tls_validate_certificate(),
      .uris = {&uri, 1},
      .proxy = Flags::rest_proxy(),
      .user_agent = ROQ_PACKAGE_NAME,
      .connection = core::http::Connection::KEEP_ALIVE,
      .allow_pipelining = true,
      .request_timeout = Flags::rest_request_timeout(),
      .ping_frequency = Flags::rest_ping_freq(),
      .ping_path = Flags::rest_ping_path(),
  };
  return core::web::Client{handler, context, config};
}
}  // namespace

Rest::Rest(Handler &handler, core::io::Context &context, uint16_t stream_id, Shared &shared)
    : handler_(handler), stream_id_(stream_id), name_(fmt::format("{}:{}"sv, stream_id_, NAME)),
      connection_(create_connection(*this, context)), decode_buffer_(Flags::decode_buffer_size()),
      counter_{
          .disconnect = create_metrics(name_, "disconnect"sv),
      },
      profile_{
          .instruments = create_metrics(name_, "instruments"sv),
          .instruments_ack = create_metrics(name_, "instruments_ack"sv),
      },
      latency_{
          .ping = create_metrics(name_, "ping"sv),
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
      .write(profile_.instruments_ack, metrics::PROFILE)
      // latency
      .write(latency_.ping, metrics::LATENCY);
}

void Rest::operator()(ConnectionStatus status) {
  if (utils::update(status_, status)) {
    auto trace_info = server::create_trace_info();
    StreamStatus stream_status{
        .stream_id = stream_id_,
        .account = {},
        .supports = SUPPORTS.get(),
        .status = status_,
        .type = StreamType::REST,
        .priority = Priority::PRIMARY,
    };
    log::info("stream_status={}"sv, stream_status);
    server::create_trace_and_dispatch(handler_, trace_info, stream_status);
  }
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
  auto trace_info = server::create_trace_info();
  ExternalLatency external_latency{
      .stream_id = stream_id_,
      .account = {},
      .latency = latency.sample,
  };
  server::create_trace_and_dispatch(handler_, trace_info, external_latency);
  latency_.ping.update(latency.sample);
}

uint32_t Rest::download(RestState state) {
  switch (state) {
    // using enum RestState::type_t; // XXX clang13
    case RestState::UNDEFINED:
      assert(false);
      break;
    case RestState::INSTRUMENTS:
      get_instruments();
      return 1;
    case RestState::DONE:
      (*this)(ConnectionStatus::READY);
      return {};
  }
  assert(false);
  return {};
}

// instruments

void Rest::get_instruments() {
  profile_.instruments([&]() {
    auto method = core::http::Method::GET;
    auto path = "/api/v3/instruments"sv;
    core::web::Request request{
        .method = method,
        .path = path,
        .query = {},
        .accept = core::http::Accept::JSON,
        .content_type = {},
        .headers = {},
        .body = {},
        .quality_of_service = {},
    };
    auto sequence = download_.sequence();
    connection_(
        "instruments"sv,
        request,
        [this, sequence]([[maybe_unused]] auto &request_id, auto &response) {
          auto trace_info = server::create_trace_info();
          server::Trace event(trace_info, response);
          get_instruments_ack(event, sequence);
        });
  });
}

void Rest::get_instruments_ack(const server::Trace<core::web::Response> &event, uint32_t sequence) {
  profile_.instruments_ack([&]() {
    auto &[trace_info, response] = event;
    auto state = RestState::INSTRUMENTS;
    try {
      auto [status, category, body] = response.result();
      log::debug(R"(status={}, category={}, body="{}")"sv, status, category, body);
      if (download_.skip(sequence, state)) {
        log::info("Download state={} has already been processed"sv, state);
        return;
      }
      response.expect(core::http::Status::OK);
      core::json::Buffer buffer(decode_buffer_);
      auto instruments = core::json::Parser::create<json::Instruments>(body, buffer);
      if (std::empty(instruments.error)) {
        server::Trace event(trace_info, instruments);
        (*this)(event);
        download_.check(state);
      } else {
        log::warn("instruments={}"sv, instruments);
        if (utils::compare(instruments.error, "Unavailable"sv) == 0) {
          download_.retry(state);
        } else {
          log::fatal("Unexpected"sv);
        }
      }
    } catch (core::NetworkError &e) {
      log::warn(R"(Exception type={}, what="{}")"sv, typeid(e).name(), e.what());
      download_.retry(state);
    }
  });
}

void Rest::operator()(const server::Trace<json::Instruments> &events) {
  auto &[trace_info, instruments] = events;
  log::info<4>("instruments={}"sv, instruments);
  assert(std::empty(instruments.error));
  std::vector<std::string> symbols;
  symbols.reserve(std::size(instruments.instruments));
  size_t counter = {};
  for (auto &item : instruments.instruments) {
    log::info<2>("item={}"sv, item);
    std::string symbol(item.symbol);  // note! we need the upper-case version
    std::transform(std::begin(symbol), std::end(symbol), std::begin(symbol), ::toupper);
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
        .margin_currency = {},
        .commission_currency = {},
        .tick_size = item.tick_size,
        .multiplier = item.contract_size,
        .min_trade_vol = 1.0,
        .max_trade_vol = NaN,
        .trade_vol_step_size = 1.0,
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
    server::create_trace_and_dispatch(handler_, trace_info, reference_data, true);
  }
  log::info("Instruments {} / {}"sv, counter, std::size(instruments.instruments));
  if (!std::empty(symbols)) {
    SymbolsUpdate symbols_update{
        .symbols = symbols,
    };
    handler_(symbols_update);
  }
}

}  // namespace kraken_futures
}  // namespace roq

#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="libtool --mode=execute gdb --args"
else
  PREFIX=
fi

NAME="kraken-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-kraken-futures/$CONFIG.toml"

ENV=""

URI="futures.kraken.com"

REST_URI="https://${ENV}$URI"
WS_URI="wss://${ENV}$URI/ws/v3"

$PREFIX ./roq-kraken-futures \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --event_log_symlink true \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  --rest_uri "$REST_URI" \
  --ws_uri "$WS_URI" \
  --time_series_interval "60s" \
  --time_series_lookback "2h" \
  --time_series_realtime true \
  --download_time_series true \
  $@

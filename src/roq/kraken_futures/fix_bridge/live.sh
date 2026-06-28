#!/usr/bin/env bash

if [ "$1" == "debug" ]; then
  PREFIX="libtool --mode=execute gdb --args"
else
  PREFIX=
fi

NAME="kraken-futures"

CONFIG="${CONFIG:-$NAME}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-kraken-futures/$CONFIG.toml"

FLAGFILE="../../../../share/flags/prod/flags.cfg"

$PREFIX ./roq-kraken-futures-fix-bridge \
  --name "$NAME" \
  --config_file "$CONFIG_FILE" \
  --flagfile "$FLAGFILE" \
  --cache_dir "$HOME/var/lib/roq/cache" \
  --event_log_dir "$HOME/var/lib/roq/data" \
  --client_listen_address "$HOME/run/$NAME.sock" \
  --service_listen_address "$HOME/run/metrics/${NAME}.sock" \
  $@

#  --time_series_interval "60s" \
#  --time_series_lookback "2h" \
#  --time_series_realtime true \
#  --download_time_series true \

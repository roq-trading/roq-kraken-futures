#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
  PREFIX="libtool --mode=execute gdb --args"
else
  PREFIX=
fi

NAME="kraken-futures"

CONFIG_FILE="$CWD/config/$NAME-demo.toml"

ENV="demo-"

URI="futures.kraken.com"

REST_URI="https://${ENV}$URI/derivatives"
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
  $@

#!/usr/bin/env bash

CWD="$(realpath "$(dirname "${BASH_SOURCE[0]}")")"

if [ "$1" == "debug" ]; then
	PREFIX="libtool --mode=execute gdb --args"
else
	PREFIX=
fi

NAME="kraken-futures"

CONFIG_FILE="$CWD/config/$NAME.toml"

ENV=""

URI="futures.kraken.com"

REST_URI="https://${ENV}$URI/derivatives"
WS_URI="wss://${ENV}$URI/ws/v3"

$PREFIX ./roq-kraken-futures \
	--name "$NAME" \
	--config_file "$CONFIG_FILE" \
	--client_listen_address "$CWD/$NAME.sock" \
	--metrics_listen_address "$CWD/${NAME}_metrics.sock" \
	--rest_uri "$REST_URI" \
	--ws_uri "$WS_URI" \
	$@

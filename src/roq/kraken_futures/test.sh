#!/usr/bin/env bash

NAME="kraken-futures"

KERNEL="$(uname -a)"
                                                                                                                                                                   
case "$KERNEL" in
  Linux*)
    LOCAL_INTERFACE=$(ip route get 8.8.8.8 | sed -n 's/.*src \([^\ ]*\).*/\1/p')
    ;;
  Darwin*)
    LOCAL_INTERFACE=$(osascript -e "IPv4 address of (system info)")
    ;;
  *)
    (>&2 echo -e "\033[1;31mERROR: Unknown architecture.\033[0m") && exit 1
esac

# debug?                                                                                                                                                                      

if [ "$1" == "debug" ]; then
  case "$KERNEL" in
    Linux*)
      PREFIX="gdb --command=gdb_commands --args"
      ;;
    Darwin*)
      PREFIX="lldb --"
      ;;
  esac
  shift 1
else
  PREFIX=
fi

CONFIG="${CONFIG:-$NAME-demo}"

CONFIG_FILE="$ROQ_CONFIG_PATH/roq-kraken-futures/$CONFIG.toml"

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

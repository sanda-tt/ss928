#!/bin/sh
set -eu

APP_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
CONFIG=${CONFIG:-"$APP_DIR/config.ss928_uart4.json"}
SERIAL_DEVICE=${SERIAL_DEVICE:-"/dev/ttyAMA4"}
BAUD=${BAUD:-115200}

echo "Configuring SS928 40Pin UART4 for DX-GP21-A"
bspmm 0x102F0134 0x1201  # 40Pin Pin10 -> UART4_RXD
bspmm 0x102F0138 0x1201  # 40Pin Pin8  -> UART4_TXD

if [ ! -e "$SERIAL_DEVICE" ]; then
  echo "WARN: $SERIAL_DEVICE does not exist yet; check kernel ttyAMA4 support" >&2
fi

cd "$APP_DIR"
exec python3 ./dx_gp21_tracker.py \
  --config "$CONFIG" \
  --serial-device "$SERIAL_DEVICE" \
  --baud "$BAUD"

#!/bin/sh
set -eu

BASE_DIR="${SMARTBAG_HOME:-/root/smartbag_alert}"
CONFIG_FILE="${SMARTBAG_CONFIG:-$BASE_DIR/config/smartbag.env}"

if [ -f "$CONFIG_FILE" ]; then
  set -a
  . "$CONFIG_FILE"
  set +a
fi

BASE_DIR="${SMARTBAG_HOME:-$BASE_DIR}"
MODEL_PATH="${SMARTBAG_OM_MODEL:-$BASE_DIR/intelligent_bag/models/yolo11s.om}"
AUDIO_ROOT="${SMARTBAG_AUDIO_ROOT:-$BASE_DIR/audio}"
BLE_NAME="${SMARTBAG_BLE_NAME:-SS928-SmartBag}"
EVENT_TIMEOUT="${SMARTBAG_EVENT_TIMEOUT:-1.0}"
I2C_DEV="${SMARTBAG_I2C_DEV:-/dev/i2c-0}"
TM6605_MUX_ADDR="${SMARTBAG_TM6605_MUX_ADDR:-0x70}"
LEFT_TM6605_CHANNEL="${SMARTBAG_LEFT_TM6605_CHANNEL:-0}"
RIGHT_TM6605_CHANNEL="${SMARTBAG_RIGHT_TM6605_CHANNEL:-1}"
TM6605_USE_MUX="${SMARTBAG_TM6605_USE_MUX:-0}"
ENABLE_RIGHT_TM6605="${SMARTBAG_ENABLE_RIGHT_TM6605:-0}"
LEFT_DETECTOR="${SMARTBAG_LEFT_DETECTOR:-}"
RIGHT_DETECTOR="${SMARTBAG_RIGHT_DETECTOR:-}"
MR20_CONFIG="${SMARTBAG_MR20_CONFIG:-$BASE_DIR/config/mr20_radar.json}"

if [ "${SMARTBAG_ENABLE_OM_BRIDGE:-0}" = "1" ]; then
  LEFT_RUNNER="${SMARTBAG_LEFT_RUNNER:-}"
  RIGHT_RUNNER="${SMARTBAG_RIGHT_RUNNER:-}"
  if [ -n "$LEFT_RUNNER" ] && [ -z "$LEFT_DETECTOR" ]; then
    LEFT_DETECTOR="python3 om_alert_bridge.py --side left --model $MODEL_PATH --runner $LEFT_RUNNER --runner-cwd $BASE_DIR/runtime/om_left"
  fi
  if [ -n "$RIGHT_RUNNER" ] && [ -z "$RIGHT_DETECTOR" ]; then
    RIGHT_DETECTOR="python3 om_alert_bridge.py --side right --model $MODEL_PATH --runner $RIGHT_RUNNER --runner-cwd $BASE_DIR/runtime/om_right"
  fi
fi

if [ ! -f "$MODEL_PATH" ]; then
  echo "WARN: OM model not found: $MODEL_PATH" >&2
fi

mkdir -p "$BASE_DIR/runtime" "$BASE_DIR/logs"
cd "$BASE_DIR/controller"
set -- python3 main.py \
  --audio-root "$AUDIO_ROOT" \
  --ble-name "$BLE_NAME" \
  --event-timeout "$EVENT_TIMEOUT" \
  --i2c-dev "$I2C_DEV" \
  --tm6605-mux-addr "$TM6605_MUX_ADDR" \
  --left-tm6605-channel "$LEFT_TM6605_CHANNEL" \
  --right-tm6605-channel "$RIGHT_TM6605_CHANNEL" \
  --left-detector "$LEFT_DETECTOR" \
  --right-detector "$RIGHT_DETECTOR"

if [ "$TM6605_USE_MUX" = "1" ]; then
  set -- "$@" --tm6605-use-mux
fi

if [ "$ENABLE_RIGHT_TM6605" = "1" ]; then
  set -- "$@" --enable-right-tm6605
fi

if [ -f "$MR20_CONFIG" ]; then
  set -- "$@" --mr20-config "$MR20_CONFIG"
else
  echo "INFO: MR20 disabled; config not found: $MR20_CONFIG" >&2
fi

exec "$@"

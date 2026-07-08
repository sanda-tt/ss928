#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd "$(dirname "$0")" && pwd)
CONFIG=${CONFIG:-"$SCRIPT_DIR/config.ss928_ble.json"}
BLE_SCRIPT=${BLE_SCRIPT:-"/opt/sample/ws73/ble.sh"}
PYTHON=${PYTHON:-python3}
I2C_BUS=${I2C_BUS:-0}
I2C_ADDR=${I2C_ADDR:-0x68}
PINMUX_I2C0=${PINMUX_I2C0:-1}
START_BLE_STACK=${START_BLE_STACK:-auto}
RESET_BLE_STACK=${RESET_BLE_STACK:-0}
USE_BLE_SH_DBUS=${USE_BLE_SH_DBUS:-0}

echo "BMI270 wiring for this startup: SDA/SCL on SS928 I2C0, CSB high, SDO low, addr=$I2C_ADDR"

have_hci()
{
  command -v btmgmt >/dev/null 2>&1 && btmgmt info 2>/dev/null | grep -q '^hci'
}

wait_for_hci()
{
  i=0
  while [ "$i" -lt 150 ]; do
    if have_hci; then
      return 0
    fi
    sleep 0.2
    i=$((i + 1))
  done
  return 1
}

if [ "$PINMUX_I2C0" = "1" ]; then
  if command -v bspmm >/dev/null 2>&1; then
    echo "Configuring 40Pin Pin3/Pin5 as I2C0"
    bspmm 0x102F013c 0x2031
    bspmm 0x102F0140 0x2031
  else
    echo "WARN: bspmm not found; skip I2C0 pinmux"
  fi
fi

if [ "$START_BLE_STACK" = "auto" ]; then
  if have_hci; then
    echo "Detected system BlueZ controller; skip ble.sh"
    START_BLE_STACK=0
  else
    echo "No system BlueZ controller detected; try loading ws73 BLE modules"
    ble_dir=$(dirname "$BLE_SCRIPT")
    if [ -f "$ble_dir/plat_soc.ko" ]; then
      insmod "$ble_dir/plat_soc.ko" 2>/dev/null || true
    fi
    if [ -f "$ble_dir/ble_soc.ko" ]; then
      insmod "$ble_dir/ble_soc.ko" 2>/dev/null || true
    fi
    sleep 1
    if wait_for_hci; then
      echo "System BlueZ controller appeared after module load; skip ble.sh"
      START_BLE_STACK=0
    else
      echo "No controller after module load; use full ble.sh fallback"
      START_BLE_STACK=1
    fi
  fi
fi

if [ "$START_BLE_STACK" = "1" ]; then
  if [ -f "$BLE_SCRIPT" ]; then
    if [ "$RESET_BLE_STACK" = "1" ]; then
      echo "Stopping existing BLE stack first"
      ble_dir=$(dirname "$BLE_SCRIPT")
      ble_name=$(basename "$BLE_SCRIPT")
      (cd "$ble_dir" && sh "./$ble_name" 1) >/dev/null 2>&1 || true
      sleep 1
    fi

    echo "Starting BLE stack with $BLE_SCRIPT 0"
    tmp_log="${TMPDIR:-/tmp}/ss928_ble_start_$$.log"
    ble_dir=$(dirname "$BLE_SCRIPT")
    ble_name=$(basename "$BLE_SCRIPT")
    : > "$tmp_log"
    (cd "$ble_dir" && sh "./$ble_name" 0) >"$tmp_log" 2>&1 &
    ble_pid=$!

    i=0
    while [ "$i" -lt 50 ]; do
      if grep -q 'DBUS_SYSTEM_BUS_ADDRESS=' "$tmp_log"; then
        break
      fi
      if ! kill -0 "$ble_pid" 2>/dev/null; then
        break
      fi
      sleep 0.2
      i=$((i + 1))
    done

    wait "$ble_pid" 2>/dev/null || true
    cat "$tmp_log"

    esc=$(printf '\033')
    session_value=$(grep -m 1 'DBUS_SESSION_BUS_ADDRESS=' "$tmp_log" | sed "s/^.*DBUS_SESSION_BUS_ADDRESS=//; s/${esc}.*$//" || true)
    system_value=$(grep -m 1 'DBUS_SYSTEM_BUS_ADDRESS=' "$tmp_log" | sed "s/^.*DBUS_SYSTEM_BUS_ADDRESS=//; s/${esc}.*$//" || true)
    rm -f "$tmp_log"

    if [ "$USE_BLE_SH_DBUS" = "1" ]; then
      if [ -n "$session_value" ]; then
        DBUS_SESSION_BUS_ADDRESS=$session_value
        export DBUS_SESSION_BUS_ADDRESS
        echo "Using DBUS_SESSION_BUS_ADDRESS from ble.sh"
      else
        echo "WARN: ble.sh did not print DBUS_SESSION_BUS_ADDRESS"
      fi

      if [ -n "$system_value" ]; then
        DBUS_SYSTEM_BUS_ADDRESS=$system_value
        export DBUS_SYSTEM_BUS_ADDRESS
        echo "Using DBUS_SYSTEM_BUS_ADDRESS from ble.sh"
      else
        echo "WARN: ble.sh did not print DBUS_SYSTEM_BUS_ADDRESS"
      fi
    else
      echo "Ignoring ble.sh DBus exports; use system BlueZ DBus"
      unset DBUS_SESSION_BUS_ADDRESS
      unset DBUS_SYSTEM_BUS_ADDRESS
      for p in $(ps -eo pid,args | awk '/[d]bus-daemon --config-file=\/usr\/share\/dbus-1\/session.conf/ {print $1}'); do
        kill "$p" 2>/dev/null || true
      done
      for p in $(ps -eo pid,args | awk '/[b]luetoothd -n/ {print $1}'); do
        kill "$p" 2>/dev/null || true
      done
    fi
  else
    echo "WARN: $BLE_SCRIPT not found; assuming bluetoothd/dbus are already running"
  fi
fi

if wait_for_hci; then
  echo "BLE controller is visible"
else
  echo "WARN: BLE controller still not visible before bluetoothctl"
fi

if command -v bluetoothctl >/dev/null 2>&1; then
  printf 'power on
quit
' | bluetoothctl || true
else
  echo "WARN: bluetoothctl not found; Python BLE registration will try current BlueZ state"
fi
exec "$PYTHON" "$SCRIPT_DIR/bmi270_backpack.py" \
  --config "$CONFIG" \
  --backend i2c \
  --i2c-bus "$I2C_BUS" \
  --i2c-addr "$I2C_ADDR" \
  --ble

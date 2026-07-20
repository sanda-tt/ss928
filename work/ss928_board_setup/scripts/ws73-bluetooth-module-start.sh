#!/bin/sh
set -eu

MODULE_DIR=${WS73_MODULE_DIR:-/opt/sample/ws73}

module_loaded()
{
  grep -q "^$1 " /proc/modules
}

if ! module_loaded plat_soc; then
  insmod "$MODULE_DIR/plat_soc.ko"
fi

if ! module_loaded ble_soc; then
  insmod "$MODULE_DIR/ble_soc.ko"
fi

i=0
while [ "$i" -lt 50 ]; do
  if [ -d /sys/class/bluetooth/hci0 ]; then
    exit 0
  fi
  sleep 0.2
  i=$((i + 1))
done

echo "WS73 BLE module loaded but hci0 did not appear" >&2
exit 1

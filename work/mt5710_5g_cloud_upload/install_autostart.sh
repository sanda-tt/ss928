#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
  echo "ERROR: run as root on BOARD-LINUX" >&2
  exit 2
fi

APP_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
UNIT_NAME=smartbag-5g-upload.service
UNIT_SOURCE="$APP_DIR/$UNIT_NAME"

if [ ! -f "$UNIT_SOURCE" ] || [ ! -x "$APP_DIR/start_ss928_5g_upload.sh" ]; then
  echo "ERROR: incomplete uploader installation in $APP_DIR" >&2
  exit 1
fi

install -m 0644 "$UNIT_SOURCE" "/etc/systemd/system/$UNIT_NAME"
systemctl daemon-reload
systemctl disable --now bmi270-backpack.service || true
systemctl enable --now smartbag-5g-upload.service

echo "Enabled $UNIT_NAME; inspect with: systemctl status $UNIT_NAME"

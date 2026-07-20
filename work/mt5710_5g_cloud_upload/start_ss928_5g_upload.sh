#!/bin/sh
set -eu

APP_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
WORK_ROOT=${WORK_ROOT:-$(dirname "$APP_DIR")}
ENV_FILE=${ENV_FILE:-/root/config/smartbag-upload.env}

if [ -f "$ENV_FILE" ]; then
  set -a
  . "$ENV_FILE"
  set +a
fi

PYTHONPATH="$WORK_ROOT${PYTHONPATH:+:$PYTHONPATH}"
export PYTHONPATH

exec python3 "$APP_DIR/mt5710_5g_cloud_upload.py" --work-root "$WORK_ROOT" "$@"

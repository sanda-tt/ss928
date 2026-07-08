#!/bin/sh
set -eu

cd /root/smartbag_alert/controller
exec python3 smartbag_alert_controller.py \
  --detector-cwd /root/vision_obstacle_tracker \
  --left-detector "python3 vision_obstacle_tracker.py --camera-index 0 --no-display" \
  --right-detector "python3 vision_obstacle_tracker.py --camera-index 1 --no-display"

#!/bin/sh
set -eu

PROJECT_DIR=${PROJECT_DIR:-/root/smartbag_alert/runtime/yolov8_vot}
. "$PROJECT_DIR/config/detector.env"

export LD_LIBRARY_PATH="/opt/lib:/opt/lib/npu:${LD_LIBRARY_PATH:-}"
exec "$PROJECT_DIR/bin/ss928_yolov8_vot" \
  --model "$MODEL_PATH" \
  --source raw \
  --input "$PROJECT_DIR/data/test_input.nv12" \
  --source-width 546 \
  --source-height 413 \
  --source-fps 30 \
  --side left \
  --emit-alert-jsonl \
  --no-display \
  --repeat "${1:-1}" \
  --conf "$CONFIDENCE" \
  --nms "$NMS" \
  --max-det "$MAX_DETECTIONS" \
  --target-classes "$TARGET_CLASSES" \
  --camera-height "$CAMERA_HEIGHT_M" \
  --camera-pitch "$CAMERA_PITCH_DEG" \
  --fov "$CAMERA_FOV_DEG" \
  --fov-type "$CAMERA_FOV_TYPE" \
  --distance-mode "$DISTANCE_MODE"

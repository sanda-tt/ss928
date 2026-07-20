#!/bin/sh
set -eu

PROJECT_DIR=${PROJECT_DIR:-/root/smartbag_alert/runtime/yolov8_vot}
. "$PROJECT_DIR/config/detector.env"

if [ "$ENABLE_STAGE_J" != 1 ]; then
  echo "camera stage is locked: offline real-detection acceptance (stage G) has not passed" >&2
  exit 3
fi
if [ "$CALIBRATION_ACCEPTED" != 1 ]; then
  echo "camera stage is locked: the actual 120-degree 2K camera calibration is not accepted" >&2
  exit 3
fi
if [ "$CAMERA_WIDTH" -le 0 ] || [ "$CAMERA_HEIGHT" -le 0 ] || [ "$CAMERA_FPS" -le 0 ]; then
  echo "camera stage is locked: record the real UVC width, height and fps first" >&2
  exit 3
fi

export LD_LIBRARY_PATH="/opt/lib:/opt/lib/npu:${LD_LIBRARY_PATH:-}"
exec "$PROJECT_DIR/bin/ss928_yolov8_vot" \
  --model "$MODEL_PATH" \
  --source camera \
  --camera "$CAMERA_DEVICE" \
  --source-width "$CAMERA_WIDTH" \
  --source-height "$CAMERA_HEIGHT" \
  --source-fps "$CAMERA_FPS" \
  --side "${SIDE:-left}" \
  --emit-alert-jsonl \
  --no-display \
  --conf "$CONFIDENCE" \
  --nms "$NMS" \
  --max-det "$MAX_DETECTIONS" \
  --target-classes "$TARGET_CLASSES" \
  --camera-height "$CAMERA_HEIGHT_M" \
  --camera-pitch "$CAMERA_PITCH_DEG" \
  --fov "$CAMERA_FOV_DEG" \
  --fov-type "$CAMERA_FOV_TYPE" \
  --distance-mode "$DISTANCE_MODE"

from __future__ import annotations

import math
import sys
import time
from pathlib import Path
from typing import Any, Callable, Mapping


IMU_FALL_DIR = Path(__file__).resolve().parents[1] / "imu_fall_detector"
if str(IMU_FALL_DIR) not in sys.path:
    sys.path.insert(0, str(IMU_FALL_DIR))

from fall_fusion import (  # noqa: E402
    FallFusionConfig,
    FallFusionGate,
    event_to_json,
    read_recent_warning,
)
from imu_fall_detector import (  # noqa: E402
    DetectorConfig,
    FallImpactDetector,
    ImuSample,
    load_config,
)


STANDARD_GRAVITY = 9.80665


def to_detector_sample(sample: Any) -> ImuSample:
    """Convert board SI units to g and degrees per second."""
    return ImuSample(
        t=float(sample.t),
        ax=float(sample.ax) / STANDARD_GRAVITY,
        ay=float(sample.ay) / STANDARD_GRAVITY,
        az=float(sample.az) / STANDARD_GRAVITY,
        gx=math.degrees(float(sample.gx)),
        gy=math.degrees(float(sample.gy)),
        gz=math.degrees(float(sample.gz)),
    )


class FallFusionRuntime:
    def __init__(
        self,
        warning_marker: str | Path,
        fusion_config: FallFusionConfig | None = None,
        detector: FallImpactDetector | None = None,
        detector_config_path: str | None = None,
        alarm_log: str | Path | None = None,
        alarm_sink: Callable[[Mapping[str, Any]], None] | None = None,
        cloud_alarm_sink: Callable[[Mapping[str, Any]], Any] | None = None,
        wall_clock: Callable[[], float] = time.time,
    ) -> None:
        self.fusion_config = fusion_config or FallFusionConfig()
        if detector is None:
            detector_config = load_config(detector_config_path) if detector_config_path else DetectorConfig()
            detector_config.terminal_hold_s = max(
                detector_config.terminal_hold_s,
                self.fusion_config.recovery_window_s + 1.0,
            )
            detector = FallImpactDetector(detector_config)
        self.detector = detector
        self.gate = FallFusionGate(self.fusion_config)
        self.warning_marker = Path(warning_marker)
        self.alarm_log = Path(alarm_log) if alarm_log else None
        self.alarm_sink = alarm_sink or self._emit_alarm
        self.cloud_alarm_sink = cloud_alarm_sink
        self.wall_clock = wall_clock

    def update(self, sample: Any) -> dict[str, Any] | None:
        imu_sample = to_detector_sample(sample)
        imu_events = self.detector.update(imu_sample)
        now_wall = float(self.wall_clock())

        for event in imu_events:
            if event.get("event") != "fall_confirmed":
                continue
            warning = read_recent_warning(
                self.warning_marker,
                now_wall=now_wall,
                window_s=self.fusion_config.warning_window_s,
            )
            self.gate.start(event, warning, now_mono=imu_sample.t, now_wall=now_wall)

        posture_delta = float(self.detector.last_features.get("posture_delta_deg", 0.0))
        alarm = self.gate.update_motion(
            posture_delta,
            now_mono=imu_sample.t,
            now_wall=now_wall,
        )
        if alarm is not None:
            self.alarm_sink(alarm)
            if self.cloud_alarm_sink is not None:
                try:
                    self.cloud_alarm_sink(alarm)
                except Exception as exc:
                    print(
                        f"WARN fall cloud upload failed: {type(exc).__name__}",
                        file=sys.stderr,
                        flush=True,
                    )
        return alarm

    def _emit_alarm(self, event: Mapping[str, Any]) -> None:
        payload = event_to_json(event)
        print(payload, flush=True)
        if self.alarm_log is None:
            return
        self.alarm_log.parent.mkdir(parents=True, exist_ok=True)
        with self.alarm_log.open("a", encoding="utf-8") as file:
            file.write(payload + "\n")

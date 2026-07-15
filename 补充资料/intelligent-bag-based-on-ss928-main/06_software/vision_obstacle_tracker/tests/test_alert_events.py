from __future__ import annotations

import sys
import types
import unittest
from pathlib import Path


PROJECT_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT_DIR))
sys.modules.setdefault("cv2", types.SimpleNamespace())

from risk_model import RiskAssessment, RiskLevel  # noqa: E402
from vision_obstacle_tracker import (  # noqa: E402
    build_vision_alert_event,
    select_highest_risk_assessment,
)


def assessment(track_id: int, score: float, level: RiskLevel) -> RiskAssessment:
    return RiskAssessment(
        track_id=track_id,
        score=score,
        level=level,
        ttc_s=None,
        trajectory_distance_m=None,
        drac_mps2=0.0,
        closing_speed_mps=0.0,
    )


class AlertEventTest(unittest.TestCase):
    def test_selects_highest_warning_level_then_score(self) -> None:
        low = assessment(1, 0.58, RiskLevel.ATTENTION)
        high = assessment(7, 0.76, RiskLevel.DANGER)
        same_level_higher_score = assessment(9, 0.79, RiskLevel.DANGER)

        selected = select_highest_risk_assessment({1: low, 7: high, 9: same_level_higher_score})

        self.assertEqual(selected.track_id, 9)

    def test_vision_alert_json_shape(self) -> None:
        event = build_vision_alert_event(
            "left",
            assessment(4, 0.82, RiskLevel.EMERGENCY),
            timestamp_s=123.456,
        )

        self.assertEqual(
            event,
            {
                "type": "vision_alert",
                "side": "left",
                "level": 4,
                "score": 0.82,
                "track_id": 4,
                "ts": 123.456,
            },
        )

    def test_safe_assessment_does_not_emit_warning_event(self) -> None:
        event = build_vision_alert_event(
            "right",
            assessment(2, 0.0, RiskLevel.SAFE),
            timestamp_s=5.0,
        )

        self.assertIsNone(event)


if __name__ == "__main__":
    unittest.main()

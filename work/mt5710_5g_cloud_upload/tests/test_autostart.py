from __future__ import annotations

import unittest
from pathlib import Path


APP_DIR = Path(__file__).resolve().parents[1]


class AutostartUnitTests(unittest.TestCase):
    def test_unit_is_non_blocking_and_restarts_failed_hardware_attempts(self) -> None:
        unit = (APP_DIR / "smartbag-5g-upload.service").read_text(encoding="utf-8")

        for required in (
            "Type=simple",
            "Restart=always",
            "RestartSec=5",
            "TimeoutStopSec=10",
            "KillMode=control-group",
            "ExecStart=/root/work/mt5710_5g_cloud_upload/start_ss928_5g_upload.sh",
            "WantedBy=multi-user.target",
        ):
            self.assertIn(required, unit)
        for forbidden in ("network-online.target", "Type=oneshot", "ExecStartPre"):
            self.assertNotIn(forbidden, unit)


if __name__ == "__main__":
    unittest.main()

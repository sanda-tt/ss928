#!/usr/bin/env python3
"""Exercise the deployed controller parser while avoiding its dry-run sysfs wait.

The deployed LinuxSysfsPwm.setup_channel() waits for a real sysfs node even
when dry_run is true.  This test-only shim bypasses only that wait; event
parsing, state reduction, and dry-run output remain the deployed controller's
own code.
"""

from __future__ import annotations

import sys
from pathlib import Path


CONTROLLER_DIR = Path("/root/smartbag_alert/controller")
sys.path.insert(0, str(CONTROLLER_DIR))

import pwm_lights  # noqa: E402


_real_setup_channel = pwm_lights.LinuxSysfsPwm.setup_channel


def _setup_channel(self, chip: int, channel: int, period_ns: int) -> None:
    if not self.dry_run:
        _real_setup_channel(self, chip, channel, period_ns)
        return
    print(
        f"DRY pwm setup pwmchip{chip}/pwm{channel} period={period_ns} "
        "(test shim: no sysfs wait)",
        flush=True,
    )


pwm_lights.LinuxSysfsPwm.setup_channel = _setup_channel

import smartbag_alert_controller  # noqa: E402


if __name__ == "__main__":
    smartbag_alert_controller.main()

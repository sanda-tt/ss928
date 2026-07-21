from __future__ import annotations

import sys
import tempfile
import unittest
from contextlib import nullcontext
from pathlib import Path


PROJECT_DIR = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(PROJECT_DIR))

from alert_core import (  # noqa: E402
    AlertEvent,
    AlertState,
    audio_clip_for,
    parse_alert_command,
    record_high_warning_marker,
)
from tm6605_haptics import (  # noqa: E402
    TM6605_EFFECT_REGISTER,
    TM6605_LIGHT_IMPACT,
    TM6605_PLAY_REGISTER,
    TM6605_STRONG_ALERT,
    Tm6605Haptics,
)
from pwm_lights import (  # noqa: E402
    LEVEL3_DUTY_PERCENT,
    LEVEL4_DUTY_PERCENT,
    LinuxSysfsPwm,
    PwmLights,
)


class FakeI2cBus:
    dry_run = False

    def __init__(self) -> None:
        self.writes: list[tuple[int, bytes, str]] = []

    def write_to(self, address: int, data: bytes, label: str) -> None:
        self.writes.append((address, data, label))

    def transaction(self):
        return nullcontext()


class FakePwm(LinuxSysfsPwm):
    def __init__(self) -> None:
        super().__init__(dry_run=False)
        self.outputs: list[tuple[int, int, int, int, bool]] = []

    def setup_channel(self, chip: int, channel: int, period_ns: int) -> None:
        return

    def set_output(self, chip: int, channel: int, period_ns: int, duty_percent: int, enabled: bool) -> None:
        self.outputs.append((chip, channel, period_ns, duty_percent, enabled))


class ZeroPeriodRejectingPwm(LinuxSysfsPwm):
    """Match the board driver: redundant disable is invalid before period setup."""

    def _write(self, path: Path, value: str, label: str) -> None:
        if path.name == "enable" and value == "0":
            period = path.parent / "period"
            if path.read_text(encoding="ascii") == "0" and period.read_text(encoding="ascii") == "0":
                raise OSError(22, "Invalid argument")
        super()._write(path, value, label)


class AlertCoreTest(unittest.TestCase):
    def test_pwm_setup_skips_redundant_disable_before_initial_period(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            channel_dir = root / "pwmchip0" / "pwm10"
            channel_dir.mkdir(parents=True)
            (channel_dir / "enable").write_text("0", encoding="ascii")
            (channel_dir / "period").write_text("0", encoding="ascii")
            (channel_dir / "duty_cycle").write_text("0", encoding="ascii")

            pwm = ZeroPeriodRejectingPwm(root=root)
            pwm.setup_channel(0, 10, 1_000_000)

            self.assertEqual((channel_dir / "enable").read_text(encoding="ascii"), "0")
            self.assertEqual((channel_dir / "period").read_text(encoding="ascii"), "1000000")
            self.assertEqual((channel_dir / "duty_cycle").read_text(encoding="ascii"), "0")

    def test_only_camera_or_radar_level_three_and_four_refresh_fall_marker(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            marker = Path(temp) / "warning.json"

            self.assertFalse(
                record_high_warning_marker(
                    AlertEvent(side="left", level=2, source="vision"), marker, now_wall=1000.0
                )
            )
            self.assertFalse(marker.exists())
            self.assertTrue(
                record_high_warning_marker(
                    AlertEvent(side="right", level=4, source="radar:right_rear"),
                    marker,
                    now_wall=1001.0,
                )
            )
            self.assertIn('"level":4', marker.read_text(encoding="utf-8"))

    def test_level_one_is_one_light_haptic(self) -> None:
        bus = FakeI2cBus()
        haptics = Tm6605Haptics(bus)
        haptics.set_levels({"left": 1}, now=10.0)

        effect_writes = [data for address, data, _label in bus.writes if address == 0x2D and data[0] == TM6605_EFFECT_REGISTER]
        self.assertEqual(effect_writes, [bytes((TM6605_EFFECT_REGISTER, TM6605_LIGHT_IMPACT))])

    def test_level_two_is_two_light_haptics(self) -> None:
        bus = FakeI2cBus()
        haptics = Tm6605Haptics(bus)
        haptics.set_levels({"left": 2}, now=10.0)
        haptics.tick(now=10.25)

        effect_writes = [data for address, data, _label in bus.writes if address == 0x2D and data[0] == TM6605_EFFECT_REGISTER]
        self.assertEqual(effect_writes, [bytes((TM6605_EFFECT_REGISTER, TM6605_LIGHT_IMPACT))] * 2)

    def test_level_three_is_two_730_ms_alerts_for_about_1_5_seconds(self) -> None:
        bus = FakeI2cBus()
        haptics = Tm6605Haptics(bus)
        haptics.set_levels({"left": 3}, now=10.0)
        haptics.tick(now=10.75)
        haptics.tick(now=11.50)

        effect_writes = [data for address, data, _label in bus.writes if address == 0x2D and data[0] == TM6605_EFFECT_REGISTER]
        self.assertEqual(effect_writes, [bytes((TM6605_EFFECT_REGISTER, 15))] * 2)
        play_writes = [data for address, data, _label in bus.writes if address == 0x2D and data[0] == TM6605_PLAY_REGISTER]
        self.assertEqual(play_writes, [bytes((TM6605_PLAY_REGISTER, 1))] * 2)

    def test_left_level_four_is_three_strong_pulses(self) -> None:
        bus = FakeI2cBus()
        haptics = Tm6605Haptics(bus)
        haptics.set_levels({"left": 4}, now=10.0)
        haptics.tick(now=10.25)
        haptics.tick(now=10.50)

        effect_writes = [data for address, data, _label in bus.writes if address == 0x2D and data[0] == TM6605_EFFECT_REGISTER]
        self.assertEqual(effect_writes, [bytes((TM6605_EFFECT_REGISTER, TM6605_STRONG_ALERT))] * 3)
        self.assertTrue(any(data == bytes((TM6605_PLAY_REGISTER, 1)) for _address, data, _label in bus.writes))

    def test_right_haptics_are_idle_until_explicitly_enabled(self) -> None:
        bus = FakeI2cBus()
        haptics = Tm6605Haptics(bus)
        haptics.set_levels({"right": 4}, now=10.0)
        haptics.tick(now=11.0)
        self.assertEqual(bus.writes, [])

    def test_tca9548a_routes_left_and_right_to_separate_channels(self) -> None:
        bus = FakeI2cBus()
        haptics = Tm6605Haptics(
            bus,
            mux_address=0x70,
            channels={"left": 1, "right": 2},
            connected_sides=("left", "right"),
        )
        haptics.set_levels({"left": 3, "right": 4}, now=10.0)
        mux_writes = [data for address, data, _label in bus.writes if address == 0x70]
        self.assertIn(bytes((0x02,)), mux_writes)
        self.assertIn(bytes((0x04,)), mux_writes)

        started_sides = {
            label.split()[0]
            for address, data, label in bus.writes
            if address == 0x2D and data == bytes((TM6605_PLAY_REGISTER, 1))
        }
        self.assertEqual(started_sides, {"left", "right"})

    def test_level_three_light_is_50_percent_for_one_second(self) -> None:
        pwm = FakePwm()
        lights = PwmLights(pwm)
        lights.set_levels({"left": 3}, now=10.0)
        lights.tick(now=11.0)
        enabled = [entry for entry in pwm.outputs if entry[-1]]
        self.assertEqual(enabled[-1][0:2], (0, 10))
        self.assertEqual(enabled[-1][3], LEVEL3_DUTY_PERCENT)
        self.assertFalse(pwm.outputs[-1][-1])

    def test_level_four_light_flashes_three_80_percent_pulses(self) -> None:
        pwm = FakePwm()
        lights = PwmLights(pwm)
        lights.set_levels({"right": 4}, now=10.0)
        lights.tick(now=10.2)
        lights.tick(now=10.4)
        lights.tick(now=10.6)
        lights.tick(now=10.8)
        lights.tick(now=11.0)
        enabled = [entry for entry in pwm.outputs if entry[-1]]
        self.assertEqual([(entry[0], entry[1], entry[3]) for entry in enabled], [(0, 1, LEVEL4_DUTY_PERCENT)] * 3)

    def test_levels_one_and_two_do_not_enable_lights_or_audio(self) -> None:
        pwm = FakePwm()
        lights = PwmLights(pwm)
        lights.set_levels({"left": 1, "right": 2}, now=10.0)

        self.assertFalse(any(entry[-1] for entry in pwm.outputs))
        self.assertIsNone(audio_clip_for("left", 1))
        self.assertIsNone(audio_clip_for("right", 2))
        self.assertEqual(audio_clip_for("left", 3), "L3")
        self.assertEqual(audio_clip_for("right", 4), "R4")

    def test_parse_ble_alert_commands(self) -> None:
        left = parse_alert_command("AL L1")
        right = parse_alert_command(" al r4 ")
        clear = parse_alert_command("AL CLEAR")
        fall = parse_alert_command("AL FALL")

        self.assertEqual(left.kind, "alert")
        self.assertEqual(left.side, "left")
        self.assertEqual(left.level, 1)
        self.assertEqual(right.side, "right")
        self.assertEqual(right.level, 4)
        self.assertEqual(clear.kind, "clear")
        self.assertEqual(fall.kind, "fall")

    def test_invalid_alert_command_is_rejected(self) -> None:
        with self.assertRaises(ValueError):
            parse_alert_command("AL X5")

    def test_clear_stops_all_vibration(self) -> None:
        state = AlertState(event_timeout_s=1.0)

        active = state.apply_command(parse_alert_command("AL R4"), now=10.0)
        self.assertEqual(active.audio_clip, "R4")
        self.assertEqual(active.levels, {"left": 0, "right": 4})

        cleared = state.apply_command(parse_alert_command("AL CLEAR"), now=10.1)
        self.assertIsNone(cleared.audio_clip)
        self.assertEqual(cleared.levels, {"left": 0, "right": 0})

    def test_event_timeout_closes_stale_side_only(self) -> None:
        state = AlertState(event_timeout_s=1.0)
        state.apply_event(AlertEvent(side="left", level=2, score=0.61, track_id=3, ts=1.2), now=100.0)
        state.apply_event(AlertEvent(side="right", level=3, score=0.72, track_id=8, ts=1.3), now=100.8)

        still_active = state.expire(now=100.95)
        self.assertEqual(still_active.levels, {"left": 2, "right": 3})

        expired = state.expire(now=101.05)
        self.assertEqual(expired.levels, {"left": 0, "right": 3})

    def test_sources_fuse_by_highest_level_and_expire_independently(self) -> None:
        state = AlertState(event_timeout_s=1.0)
        state.apply_event(AlertEvent(side="right", level=2, source="vision"), now=10.0)
        fused = state.apply_event(AlertEvent(side="right", level=4, source="radar:right_rear"), now=10.1)
        self.assertEqual(fused.levels["right"], 4)

        radar_cleared = state.apply_event(AlertEvent(side="right", level=0, source="radar:right_rear"), now=10.2)
        self.assertEqual(radar_cleared.levels["right"], 2)

        expired = state.expire(now=11.1)
        self.assertEqual(expired.levels["right"], 0)


if __name__ == "__main__":
    unittest.main()

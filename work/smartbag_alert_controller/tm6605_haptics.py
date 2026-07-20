from __future__ import annotations

import os
import subprocess
import time
from contextlib import contextmanager
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Mapping

from alert_core import VALID_SIDES, normalize_level, normalize_side

try:
    import fcntl
except ImportError:  # Allows source tests and --dry-run validation on Windows.
    fcntl = None  # type: ignore[assignment]


I2C_SLAVE = 0x0703
TM6605_ADDRESS = 0x2D  # 7-bit address; the datasheet's write byte is 0x5A.
TCA9548A_DEFAULT_ADDRESS = 0x70
TM6605_EFFECT_REGISTER = 0x04
TM6605_PLAY_REGISTER = 0x0C
TM6605_MEDIUM_DURATION_ALERT = 15  # Approx. 730 ms; the closest built-in medium effect.
TM6605_STRONG_ALERT = 14  # Approx. 190 ms.
I2C0_PINMUX = (
    ("0x102F013c", "0x2031", "Pin3 I2C0_SDA"),
    ("0x102F0140", "0x2031", "Pin5 I2C0_SCL"),
)


@dataclass(frozen=True)
class ScheduledEffect:
    at: float
    side: str
    effect: int


class LinuxI2cBus:
    """Minimal write-only Linux i2c-dev client used by the TM6605."""

    def __init__(self, device: Path, dry_run: bool = False) -> None:
        self.device = device
        self.dry_run = dry_run

    @contextmanager
    def transaction(self):
        """Serialize mux selection with the separate BMI270 process."""
        if self.dry_run or fcntl is None:
            yield
            return
        lock_fd = os.open("/tmp/ss928-i2c0-mux.lock", os.O_CREAT | os.O_RDWR, 0o600)
        try:
            fcntl.flock(lock_fd, fcntl.LOCK_EX)
            yield
        finally:
            fcntl.flock(lock_fd, fcntl.LOCK_UN)
            os.close(lock_fd)

    def write_to(self, address: int, data: bytes, label: str) -> None:
        if self.dry_run:
            print(f"DRY i2c {self.device} addr=0x{address:02X} data={data.hex()}  # {label}", flush=True)
            return
        if fcntl is None:
            raise RuntimeError("TM6605 hardware control requires Linux fcntl/i2c-dev")
        fd = os.open(self.device, os.O_RDWR)
        try:
            fcntl.ioctl(fd, I2C_SLAVE, address)
            written = os.write(fd, data)
            if written != len(data):
                raise OSError(f"short I2C write ({written}/{len(data)}) to 0x{address:02X}")
        finally:
            os.close(fd)


class Tm6605Haptics:
    """Schedules one-shot alert effects for one or two fixed-address TM6605 boards.

    A single left module can attach directly to I2C0.  Two modules need a
    TCA9548A mux because both use address 0x2D. Levels 1/2 do not vibrate.
    """

    def __init__(
        self,
        bus: LinuxI2cBus,
        *,
        mux_address: int | None = None,
        channels: Mapping[str, int] | None = None,
        connected_sides: tuple[str, ...] = ("left",),
        clock: Callable[[], float] = time.monotonic,
    ) -> None:
        self.bus = bus
        self.mux_address = mux_address
        self.channels = dict(channels or {"left": 0, "right": 1})
        if set(self.channels) != set(VALID_SIDES) or any(channel not in range(8) for channel in self.channels.values()):
            raise ValueError("TM6605 channels must provide left/right values in range 0..7")
        self.connected_sides = tuple(normalize_side(side) for side in connected_sides)
        if not self.connected_sides:
            raise ValueError("at least one TM6605 side must be connected")
        self.clock = clock
        self._levels = {side: 0 for side in VALID_SIDES}
        self._pending: list[ScheduledEffect] = []

    def setup(self, skip_pinmux: bool = False) -> None:
        if skip_pinmux:
            return
        for address, value, label in I2C0_PINMUX:
            if self.bus.dry_run:
                print(f"DRY bspmm {address} {value}  # {label}", flush=True)
            else:
                subprocess.run(["bspmm", address, value], check=True)

    def preflight(self) -> None:
        if self.bus.dry_run:
            route = f"TCA9548A 0x{self.mux_address:02X}" if self.mux_address is not None else "direct I2C"
            print(f"DRY preflight {self.bus.device}; {route}, TM6605 0x{TM6605_ADDRESS:02X}, connected={','.join(self.connected_sides)}", flush=True)
            return
        if not self.bus.device.exists():
            raise RuntimeError(f"{self.bus.device} not found")

    def set_levels(self, levels: Mapping[str, int], now: float | None = None) -> None:
        now = self.clock() if now is None else now
        normalized = {side: 0 for side in VALID_SIDES}
        for side, level in levels.items():
            normalized[normalize_side(side)] = normalize_level(level)

        for side in VALID_SIDES:
            new_level = normalized[side]
            if new_level == self._levels[side]:
                continue
            if side not in self.connected_sides:
                self._levels[side] = new_level
                continue
            self._pending = [effect for effect in self._pending if effect.side != side]
            if self._levels[side] >= 3:
                self._set_playback(side, False)
            self._levels[side] = new_level
            if new_level == 3:
                # 3 x ~730 ms medium alerts: ~2 s total, no direct PWM is used.
                self._pending.extend(
                    ScheduledEffect(now + offset, side, TM6605_MEDIUM_DURATION_ALERT)
                    for offset in (0.0, 0.75, 1.50)
                )
            elif new_level == 4:
                # Three full-strength pulses, approximately 190 ms each.
                self._pending.extend(
                    ScheduledEffect(now + offset, side, TM6605_STRONG_ALERT)
                    for offset in (0.0, 0.30, 0.60)
                )
        self.tick(now)

    def tick(self, now: float | None = None) -> None:
        now = self.clock() if now is None else now
        ready = [effect for effect in self._pending if effect.at <= now]
        self._pending = [effect for effect in self._pending if effect.at > now]
        for effect in sorted(ready, key=lambda item: item.at):
            self._play(effect.side, effect.effect)

    def stop_all(self) -> None:
        self._pending.clear()
        for side in self.connected_sides:
            self._set_playback(side, False)
            self._levels[side] = 0
        for side in VALID_SIDES:
            if side not in self.connected_sides:
                self._levels[side] = 0

    def _select_channel(self, side: str) -> None:
        if self.mux_address is None:
            return
        self.bus.write_to(self.mux_address, bytes((1 << self.channels[side],)), f"{side} TM6605 mux channel")

    def _write_register(self, side: str, register: int, value: int) -> None:
        with self.bus.transaction():
            self._select_channel(side)
            self.bus.write_to(TM6605_ADDRESS, bytes((register, value)), f"{side} TM6605 reg 0x{register:02X}")

    def _set_playback(self, side: str, enabled: bool) -> None:
        self._write_register(side, TM6605_PLAY_REGISTER, 1 if enabled else 0)

    def _play(self, side: str, effect: int) -> None:
        self._write_register(side, TM6605_EFFECT_REGISTER, effect)
        self._set_playback(side, True)

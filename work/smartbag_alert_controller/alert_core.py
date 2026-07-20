from __future__ import annotations

import json
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Mapping


VALID_SIDES = ("left", "right")

SIDE_ALIASES = {
    "l": "left",
    "left": "left",
    "左": "left",
    "r": "right",
    "right": "right",
    "右": "right",
}


@dataclass(frozen=True)
class AlertCommand:
    kind: str
    side: str | None = None
    level: int = 0


@dataclass(frozen=True)
class AlertEvent:
    side: str
    level: int
    source: str = "vision"
    score: float | None = None
    track_id: int | None = None
    ts: float | None = None


@dataclass(frozen=True)
class AlertOutput:
    audio_clip: str | None = None
    levels: dict[str, int] | None = None
    expired_sides: tuple[str, ...] = ()


def normalize_side(side: str) -> str:
    normalized = SIDE_ALIASES.get(str(side).strip().lower())
    if normalized not in VALID_SIDES:
        raise ValueError(f"invalid side: {side!r}")
    return normalized


def normalize_level(level: Any, *, allow_zero: bool = True) -> int:
    try:
        value = int(level)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"invalid alert level: {level!r}") from exc
    low = 0 if allow_zero else 1
    if value < low or value > 4:
        raise ValueError(f"alert level must be {low}..4, got {value}")
    return value


def audio_clip_for(side: str, level: int) -> str | None:
    normalized_side = normalize_side(side)
    normalized_level = normalize_level(level)
    if normalized_level <= 0:
        return None
    prefix = "L" if normalized_side == "left" else "R"
    return f"{prefix}{normalized_level}"


def parse_alert_command(command: str) -> AlertCommand:
    text = str(command or "").strip()
    if not text:
        raise ValueError("empty alert command")

    parts = text.upper().split()
    if len(parts) < 2 or parts[0] != "AL":
        raise ValueError(f"unsupported alert command: {command!r}")
    if parts[1] == "CLEAR":
        if len(parts) != 2:
            raise ValueError(f"unsupported clear command: {command!r}")
        return AlertCommand(kind="clear")

    if len(parts) == 2:
        target = parts[1]
        if len(target) < 2:
            raise ValueError(f"unsupported alert target: {command!r}")
        side = normalize_side(target[0])
        level = normalize_level(target[1:])
        return AlertCommand(kind="alert", side=side, level=level)

    if len(parts) == 3:
        side = normalize_side(parts[1])
        level = normalize_level(parts[2].lstrip("L"))
        return AlertCommand(kind="alert", side=side, level=level)

    raise ValueError(f"unsupported alert command: {command!r}")


def parse_vision_alert_jsonl(line: str) -> AlertEvent | None:
    data = json.loads(line)
    if not isinstance(data, dict) or data.get("type") != "vision_alert":
        return None
    return AlertEvent(
        side=normalize_side(str(data["side"])),
        level=normalize_level(data["level"]),
        score=float(data["score"]) if data.get("score") is not None else None,
        track_id=int(data["track_id"]) if data.get("track_id") is not None else None,
        ts=float(data["ts"]) if data.get("ts") is not None else None,
    )


class AlertState:
    def __init__(
        self,
        event_timeout_s: float = 1.0,
        min_audio_interval_s: float = 2.0,
    ) -> None:
        self.event_timeout_s = max(0.05, float(event_timeout_s))
        self.min_audio_interval_s = max(0.0, float(min_audio_interval_s))
        self.levels_by_source_side: dict[tuple[str, str], int] = {}
        self.last_event_mono_by_source_side: dict[tuple[str, str], float] = {}
        self.last_audio_mono_by_clip: dict[str, float] = {}

    def apply_event(self, event: AlertEvent, now: float | None = None) -> AlertOutput:
        now = time.monotonic() if now is None else now
        side = normalize_side(event.side)
        level = normalize_level(event.level)
        source = self._normalize_source(event.source)
        key = (source, side)
        previous_level = self._effective_level(side)
        if level > 0:
            self.levels_by_source_side[key] = level
            self.last_event_mono_by_source_side[key] = now
        else:
            self.levels_by_source_side.pop(key, None)
            self.last_event_mono_by_source_side.pop(key, None)

        effective_level = self._effective_level(side)
        clip = audio_clip_for(side, effective_level)
        if clip is not None and not self._should_emit_audio(clip, previous_level, effective_level, now):
            clip = None
        return self._output(audio_clip=clip)

    def apply_command(self, command: AlertCommand, now: float | None = None) -> AlertOutput:
        now = time.monotonic() if now is None else now
        if command.kind == "clear":
            self.clear()
            return self._output()
        if command.kind != "alert" or command.side is None:
            raise ValueError(f"unsupported alert command kind: {command.kind!r}")
        side = normalize_side(command.side)
        level = normalize_level(command.level)
        key = ("manual", side)
        if level > 0:
            self.levels_by_source_side[key] = level
            self.last_event_mono_by_source_side[key] = now
        else:
            self.levels_by_source_side.pop(key, None)
            self.last_event_mono_by_source_side.pop(key, None)
        clip = audio_clip_for(side, self._effective_level(side))
        if clip:
            self.last_audio_mono_by_clip[clip] = now
        return self._output(audio_clip=clip)

    def clear(self) -> None:
        self.levels_by_source_side.clear()
        self.last_event_mono_by_source_side.clear()
        self.last_audio_mono_by_clip.clear()

    def expire(self, now: float | None = None) -> AlertOutput:
        now = time.monotonic() if now is None else now
        expired: list[str] = []
        for key, last_event_mono in list(self.last_event_mono_by_source_side.items()):
            if now - last_event_mono > self.event_timeout_s:
                _source, side = key
                self.levels_by_source_side.pop(key, None)
                del self.last_event_mono_by_source_side[key]
                if side not in expired:
                    expired.append(side)
        return self._output(expired_sides=tuple(expired))

    def _should_emit_audio(self, clip: str, previous_level: int, level: int, now: float) -> bool:
        if level != previous_level:
            self.last_audio_mono_by_clip[clip] = now
            return True
        last_audio = self.last_audio_mono_by_clip.get(clip)
        if last_audio is None or now - last_audio >= self.min_audio_interval_s:
            self.last_audio_mono_by_clip[clip] = now
            return True
        return False

    def _output(
        self,
        audio_clip: str | None = None,
        expired_sides: tuple[str, ...] = (),
    ) -> AlertOutput:
        return AlertOutput(
            audio_clip=audio_clip,
            levels={side: self._effective_level(side) for side in VALID_SIDES},
            expired_sides=expired_sides,
        )

    def _effective_level(self, side: str) -> int:
        return max(
            (level for (_source, event_side), level in self.levels_by_source_side.items() if event_side == side),
            default=0,
        )

    @staticmethod
    def _normalize_source(source: str) -> str:
        normalized = str(source or "").strip().lower()
        if not normalized:
            raise ValueError("alert source must not be empty")
        return normalized


def record_high_warning_marker(
    event: AlertEvent,
    path: str | Path,
    now_wall: float | None = None,
) -> bool:
    imu_fall_dir = Path(__file__).resolve().parents[1] / "imu_fall_detector"
    if str(imu_fall_dir) not in sys.path:
        sys.path.insert(0, str(imu_fall_dir))
    from fall_fusion import write_high_warning_marker

    return write_high_warning_marker(
        path,
        {
            "source": event.source,
            "side": event.side,
            "level": event.level,
            "ts": time.time() if now_wall is None else float(now_wall),
        },
    )

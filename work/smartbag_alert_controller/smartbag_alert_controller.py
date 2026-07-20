#!/usr/bin/env python3
from __future__ import annotations

import argparse
import itertools
import queue
import shlex
import signal
import subprocess
import sys
import threading
import time
from pathlib import Path
from typing import Iterable

from alert_core import (
    AlertEvent,
    AlertOutput,
    AlertState,
    parse_alert_command,
    parse_vision_alert_jsonl,
    record_high_warning_marker,
)
from ble_nus import BleNusServer
from mr20_radar import MR20RadarWorker, load_radar_configs
from pwm_lights import LinuxSysfsPwm, PwmLights
from tm6605_haptics import LinuxI2cBus, Tm6605Haptics


AUDIO_ROOT = Path("/root/smartbag_alert/audio")
SAMPLE_AUDIO = "/opt/sample/audio/sample_audio"
BLE_NAME = "SS928-SmartBag"
I2S_PINMUX = (
    ("0x102F010C", "0x1202", "Pin12 I2S_BCLK"),
    ("0x102F0108", "0x1102", "Pin38 I2S_WS"),
    ("0x102F0104", "0x1202", "Pin40 I2S_SD_TX"),
)


def eprint(message: str) -> None:
    print(message, file=sys.stderr, flush=True)


class AudioPlayer:
    def __init__(
        self,
        audio_root: Path,
        sample_audio: str = SAMPLE_AUDIO,
        dry_run: bool = False,
        enabled: bool = True,
        default_sleep_s: float = 5.0,
        default_timeout_s: float = 13.0,
        skip_pinmux: bool = False,
    ) -> None:
        self.audio_root = audio_root
        self.sample_audio = sample_audio
        self.dry_run = dry_run
        self.enabled = enabled
        self.default_sleep_s = default_sleep_s
        self.default_timeout_s = default_timeout_s
        self.skip_pinmux = skip_pinmux
        self._queue: "queue.PriorityQueue[tuple[int, int, str]]" = queue.PriorityQueue()
        self._sequence = itertools.count()
        self._stop = threading.Event()
        self._thread: threading.Thread | None = None
        self._process_lock = threading.Lock()
        self._process: subprocess.Popen[bytes] | None = None

    def setup(self) -> None:
        if self.skip_pinmux:
            return
        for addr, value, label in I2S_PINMUX:
            if self.dry_run:
                eprint(f"DRY bspmm {addr} {value}  # {label}")
            else:
                subprocess.run(["bspmm", addr, value], check=True)

    def start(self) -> None:
        if not self.enabled:
            return
        self._thread = threading.Thread(target=self._worker, daemon=True)
        self._thread.start()

    def request(self, clip: str | None) -> None:
        if not self.enabled or not clip:
            return
        try:
            level = int(clip[1:])
        except ValueError:
            level = 0
        self._queue.put((-level, next(self._sequence), clip))

    def clear(self) -> None:
        while True:
            try:
                self._queue.get_nowait()
            except queue.Empty:
                break
        with self._process_lock:
            if self._process is not None and self._process.poll() is None:
                self._process.terminate()

    def stop(self) -> None:
        self._stop.set()
        self.clear()
        if self._thread is not None:
            self._thread.join(timeout=2.0)

    def _worker(self) -> None:
        while not self._stop.is_set():
            try:
                _priority, _sequence, clip = self._queue.get(timeout=0.2)
            except queue.Empty:
                continue
            self._play(clip)

    def _play(self, clip: str) -> None:
        clip_dir = self.audio_root / clip
        audio_file = clip_dir / "audio_chn0.aac"
        if self.dry_run:
            eprint(f"DRY play {clip}: cd {clip_dir} && {self.sample_audio} 2")
            return
        if not audio_file.exists():
            eprint(f"WARN missing audio clip {audio_file}")
            return
        sleep_s, timeout_s = self._timing_for(clip_dir)
        with self._process_lock:
            self._process = subprocess.Popen(
                [self.sample_audio, "2"],
                cwd=str(clip_dir),
                stdin=subprocess.PIPE,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.STDOUT,
            )
            process = self._process
        try:
            deadline = time.monotonic() + max(timeout_s, sleep_s + 1.0)
            while time.monotonic() < deadline and process.poll() is None:
                if time.monotonic() >= deadline - max(timeout_s - sleep_s, 1.0):
                    break
                time.sleep(0.1)
            process.communicate(input=b"\n\n", timeout=max(timeout_s - sleep_s, 1.0))
        except subprocess.TimeoutExpired:
            process.kill()
        finally:
            with self._process_lock:
                if self._process is process:
                    self._process = None

    def _timing_for(self, clip_dir: Path) -> tuple[float, float]:
        hint = clip_dir / "play_hint.txt"
        sleep_s = self.default_sleep_s
        timeout_s = self.default_timeout_s
        if not hint.exists():
            return sleep_s, timeout_s
        try:
            for line in hint.read_text(encoding="utf-8", errors="ignore").splitlines():
                if line.startswith("sleep_seconds="):
                    sleep_s = float(line.split("=", 1)[1])
                elif line.startswith("timeout_seconds="):
                    timeout_s = float(line.split("=", 1)[1])
        except ValueError:
            return self.default_sleep_s, self.default_timeout_s
        return sleep_s, timeout_s


class DetectorProcess:
    def __init__(
        self,
        side: str,
        command: str,
        event_queue: "queue.Queue[AlertEvent]",
        cwd: Path | None = None,
    ) -> None:
        self.side = side
        self.command = command
        self.event_queue = event_queue
        self.cwd = cwd
        self.process: subprocess.Popen[str] | None = None
        self.thread: threading.Thread | None = None

    def start(self) -> None:
        argv = build_detector_command(self.command, self.side)
        self.process = subprocess.Popen(
            argv,
            cwd=str(self.cwd) if self.cwd else None,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding="utf-8",
            errors="replace",
            bufsize=1,
        )
        self.thread = threading.Thread(target=self._reader, daemon=True)
        self.thread.start()
        eprint(f"started {self.side} detector pid={self.process.pid}: {' '.join(argv)}")

    def stop(self) -> None:
        if self.process is not None and self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=3.0)
            except subprocess.TimeoutExpired:
                self.process.kill()

    def _reader(self) -> None:
        if self.process is None or self.process.stdout is None:
            return
        for line in self.process.stdout:
            text = line.strip()
            if not text:
                continue
            try:
                event = parse_vision_alert_jsonl(text)
            except Exception as exc:
                eprint(f"[{self.side}] ignored non-alert line: {text[:160]} ({exc})")
                continue
            if event is not None:
                self.event_queue.put(event)


def build_detector_command(command: str, side: str) -> list[str]:
    argv = shlex.split(command)
    if "--side" not in argv:
        argv.extend(["--side", side])
    if "--emit-alert-jsonl" not in argv:
        argv.append("--emit-alert-jsonl")
    return argv


def start_stdin_reader(
    event_queue: "queue.Queue[AlertEvent]",
    command_queue: "queue.Queue[str]",
    stop_event: threading.Event,
    stop_on_eof: bool = False,
) -> threading.Thread:
    def _run() -> None:
        while not stop_event.is_set():
            line = sys.stdin.readline()
            if not line:
                if stop_on_eof:
                    stop_event.set()
                break
            text = line.strip()
            if not text:
                continue
            if text.upper().startswith("AL "):
                command_queue.put(text)
                continue
            try:
                event = parse_vision_alert_jsonl(text)
            except Exception as exc:
                eprint(f"stdin ignored: {text[:160]} ({exc})")
                continue
            if event is not None:
                event_queue.put(event)

    thread = threading.Thread(target=_run, daemon=True)
    thread.start()
    return thread


def apply_output(output: AlertOutput, haptics: Tm6605Haptics, lights: PwmLights, audio: AudioPlayer) -> None:
    haptics.set_levels(output.levels or {})
    lights.set_levels(output.levels or {})
    audio.request(output.audio_clip)


def run_controller(args: argparse.Namespace) -> int:
    event_queue: "queue.Queue[AlertEvent]" = queue.Queue()
    command_queue: "queue.Queue[str]" = queue.Queue()
    stop_event = threading.Event()
    state = AlertState(event_timeout_s=args.event_timeout)
    haptics = Tm6605Haptics(
        LinuxI2cBus(Path(args.i2c_dev), dry_run=args.dry_run),
        mux_address=args.tm6605_mux_addr if args.tm6605_use_mux else None,
        channels={"left": args.left_tm6605_channel, "right": args.right_tm6605_channel},
        connected_sides=("left", "right") if args.enable_right_tm6605 else ("left",),
    )
    lights = PwmLights(LinuxSysfsPwm(dry_run=args.dry_run))
    audio = AudioPlayer(
        Path(args.audio_root),
        sample_audio=args.sample_audio,
        dry_run=args.dry_run,
        enabled=not args.no_audio,
        default_sleep_s=args.audio_sleep_s,
        default_timeout_s=args.audio_timeout_s,
        skip_pinmux=args.skip_pinmux,
    )
    detectors: list[DetectorProcess] = []
    radars: list[MR20RadarWorker] = []
    ble: BleNusServer | None = None

    def _stop(_signum: int | None = None, _frame: object | None = None) -> None:
        stop_event.set()

    signal.signal(signal.SIGINT, _stop)
    signal.signal(signal.SIGTERM, _stop)

    try:
        haptics.preflight()
        if args.preflight_only:
            return 0
        audio.setup()
        haptics.setup(skip_pinmux=args.skip_pinmux)
        lights.setup(skip_pinmux=args.skip_pinmux)
        audio.start()
        haptics.stop_all()
        lights.stop_all()

        if not args.no_ble:
            try:
                ble = BleNusServer(args.ble_name, command_queue.put)
                ble.start()
                eprint(f"BLE advertising as {args.ble_name}")
            except Exception as exc:
                eprint(f"WARN BLE disabled: {exc}")

        detector_cwd = Path(args.detector_cwd) if args.detector_cwd else None
        for side, command in (("left", args.left_detector), ("right", args.right_detector)):
            if not command:
                continue
            detector = DetectorProcess(side, command, event_queue, cwd=detector_cwd)
            detector.start()
            detectors.append(detector)

        if args.mr20_config:
            try:
                radar_configs, radar_risk = load_radar_configs(args.mr20_config)
                for radar_config in radar_configs:
                    radar = MR20RadarWorker(radar_config, radar_risk, event_queue.put)
                    radar.start()
                    radars.append(radar)
                    eprint(
                        f"started MR20 {radar_config.name}: {radar_config.bind_host}:{radar_config.port} "
                        f"<- {radar_config.radar_ip} ({radar_config.side})"
                    )
            except Exception as exc:
                raise RuntimeError(f"failed to start MR20 radar: {exc}") from exc

        if args.stdin_jsonl:
            start_stdin_reader(
                event_queue,
                command_queue,
                stop_event,
                stop_on_eof=not detectors,
            )

        while not stop_event.is_set() or not event_queue.empty() or not command_queue.empty():
            while True:
                try:
                    event = event_queue.get_nowait()
                except queue.Empty:
                    break
                try:
                    record_high_warning_marker(event, args.fall_warning_marker)
                except Exception as exc:
                    eprint(f"WARN fall warning marker failed: {exc}")
                output = state.apply_event(event)
                apply_output(output, haptics, lights, audio)

            while True:
                try:
                    command_text = command_queue.get_nowait()
                except queue.Empty:
                    break
                try:
                    command = parse_alert_command(command_text)
                    if command.kind == "clear":
                        audio.clear()
                    output = state.apply_command(command)
                    apply_output(output, haptics, lights, audio)
                    if ble is not None:
                        ble.send_line("OK " + command_text.strip())
                except Exception as exc:
                    if ble is not None:
                        ble.send_line("ERR " + str(exc))
                    eprint(f"WARN command failed {command_text!r}: {exc}")

            expired = state.expire()
            if expired.expired_sides:
                haptics.set_levels(expired.levels or {})
                lights.set_levels(expired.levels or {})
            haptics.tick()
            lights.tick()
            time.sleep(args.poll_interval)
    finally:
        stop_event.set()
        for detector in detectors:
            detector.stop()
        for radar in radars:
            radar.stop()
        if ble is not None:
            ble.stop()
        audio.stop()
        haptics.stop_all()
        lights.stop_all()
    return 0


def parse_args(argv: Iterable[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="SS928 dual-camera smart-bag alert controller.")
    parser.add_argument("--left-detector", default="", help="Base command for the left vision detector.")
    parser.add_argument("--right-detector", default="", help="Base command for the right vision detector.")
    parser.add_argument("--detector-cwd", default="", help="Working directory for detector commands.")
    parser.add_argument("--stdin-jsonl", action="store_true", help="Also read vision JSONL or AL commands from stdin.")
    parser.add_argument("--mr20-config", default="", help="Path to MR20 radar JSON configuration.")
    parser.add_argument("--i2c-dev", default="/dev/i2c-0", help="Linux I2C device for the TM6605 mux.")
    parser.add_argument("--tm6605-mux-addr", type=lambda value: int(value, 0), default=0x70, help="TCA9548A 7-bit I2C address.")
    parser.add_argument("--tm6605-use-mux", action="store_true", help="Route TM6605 traffic through the configured TCA9548A.")
    parser.add_argument("--left-tm6605-channel", type=int, default=0, help="TCA9548A channel for the left TM6605.")
    parser.add_argument("--right-tm6605-channel", type=int, default=1, help="TCA9548A channel for the right TM6605.")
    parser.add_argument("--enable-right-tm6605", action="store_true", help="Enable the right TM6605; requires a mux when both modules are connected.")
    parser.add_argument("--event-timeout", type=float, default=1.0, help="Seconds before stale side vibration is stopped.")
    parser.add_argument(
        "--fall-warning-marker",
        default="/tmp/smartbag_last_high_warning.json",
        help="Marker refreshed by camera/radar level-3 or level-4 warnings.",
    )
    parser.add_argument("--poll-interval", type=float, default=0.05, help="Controller loop sleep interval in seconds.")
    parser.add_argument("--audio-root", default=str(AUDIO_ROOT), help="Root containing L1..R4 audio folders.")
    parser.add_argument("--sample-audio", default=SAMPLE_AUDIO, help="Path to sample_audio player.")
    parser.add_argument("--audio-sleep-s", type=float, default=5.0, help="Fallback seconds before sending ENTER to sample_audio.")
    parser.add_argument("--audio-timeout-s", type=float, default=13.0, help="Fallback sample_audio timeout seconds.")
    parser.add_argument("--ble-name", default=BLE_NAME, help="BLE advertisement name.")
    parser.add_argument("--no-ble", action="store_true", help="Disable BLE debug command server.")
    parser.add_argument("--no-audio", action="store_true", help="Disable audio playback.")
    parser.add_argument("--skip-pinmux", action="store_true", help="Skip bspmm pinmux setup.")
    parser.add_argument("--dry-run", action="store_true", help="Print light PWM, haptic, and audio actions without touching hardware.")
    parser.add_argument("--preflight-only", action="store_true", help="Only check that the configured I2C device exists.")
    return parser.parse_args(argv)


def main() -> None:
    raise SystemExit(run_controller(parse_args()))


if __name__ == "__main__":
    main()

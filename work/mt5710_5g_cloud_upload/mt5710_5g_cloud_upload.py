#!/usr/bin/env python3
"""Bring up MT5710 NCM networking and run all real SmartBag upload producers."""

from __future__ import annotations

import argparse
import os
import re
import select
import signal
import shutil
import socket
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Callable, Iterable, Sequence


CLOUDBASE_HOST = "cloud1-d7gdmg27139f4fbf2.service.tcloudbase.com"
TERMINAL_LINES = {"OK", "ERROR", "COMMAND NOT SUPPORT", "NO CARRIER"}


@dataclass(frozen=True)
class RunResult:
    returncode: int
    stdout: str
    stderr: str


CommandRunner = Callable[[list[str], float], RunResult]


def choose_apn(operator: str, imsi: str, override: str) -> str:
    explicit = override.strip()
    if explicit:
        return explicit
    normalized = operator.upper().replace("_", " ").replace("-", " ")
    if "TELECOM" in normalized:
        return "ctnet"
    if "MOBILE" in normalized or "CMCC" in normalized:
        return "cmnet"
    if "UNICOM" in normalized:
        return "3gnet"
    prefix = re.sub(r"\D", "", imsi)[:5]
    if prefix in {"46003", "46005", "46011"}:
        return "ctnet"
    if prefix in {"46000", "46002", "46004", "46007", "46008"}:
        return "cmnet"
    if prefix in {"46001", "46006", "46009"}:
        return "3gnet"
    raise RuntimeError("cannot determine APN; pass --apn")


def parse_registration(lines: Sequence[str]) -> tuple[bool, bool]:
    registered = False
    attached = False
    for line in lines:
        match = re.search(r"\+(?:CEREG|C5GREG):\s*\d+\s*,\s*(\d+)", line)
        if match and match.group(1) in {"1", "5"}:
            registered = True
        match = re.search(r"\+CGATT:\s*(\d+)", line)
        if match:
            attached = match.group(1) == "1"
    return registered, attached


def _driver_name(interface_dir: Path) -> str:
    driver_link = interface_dir / "device" / "driver"
    try:
        return driver_link.resolve(strict=True).name
    except OSError:
        pass
    try:
        for line in (interface_dir / "device" / "uevent").read_text(encoding="utf-8").splitlines():
            if line.startswith("DRIVER="):
                return line.split("=", 1)[1].strip()
    except OSError:
        pass
    return ""


def _usb_identity(device_path: Path) -> tuple[str, str] | None:
    try:
        resolved = device_path.resolve(strict=True)
    except OSError:
        resolved = device_path
    for candidate in (resolved, *resolved.parents):
        try:
            vendor = (candidate / "idVendor").read_text(encoding="ascii").strip().lower()
            product = (candidate / "idProduct").read_text(encoding="ascii").strip().lower()
            if vendor and product:
                return vendor, product
        except OSError:
            continue
    return None


def discover_ncm_interface(
    sys_class_net: Path = Path("/sys/class/net"),
    expected_usb_identity: tuple[str, str] | None = None,
) -> str:
    candidates = []
    for entry in sys_class_net.iterdir():
        if not entry.is_dir() or _driver_name(entry) != "cdc_ncm":
            continue
        identity = _usb_identity(entry / "device")
        if expected_usb_identity is None or identity == expected_usb_identity:
            candidates.append(entry.name)
    candidates.sort()
    if not candidates:
        raise RuntimeError("MT5710 cdc_ncm interface not found")
    if len(candidates) > 1:
        raise RuntimeError("multiple cdc_ncm interfaces found; cannot identify MT5710")
    return candidates[0]


def route_uses_interface(route_text: str, interface: str) -> bool:
    return re.search(rf"(?:^|\s)dev\s+{re.escape(interface)}(?:\s|$)", route_text) is not None


def _add_ncm_host_route(interface: str, cloud_ip: str, command_runner: CommandRunner) -> None:
    default_result = command_runner(["ip", "route", "show", "default", "dev", interface], 10.0)
    _require_success(default_result, "NCM default route lookup")
    match = re.search(
        rf"^default(?:\s+via\s+(\S+))?\s+dev\s+{re.escape(interface)}(?:\s|$)",
        default_result.stdout.strip(),
    )
    if not match:
        raise RuntimeError(f"DHCP did not install a default route for {interface}")
    argv = ["ip", "route", "replace", f"{cloud_ip}/32"]
    if match.group(1):
        argv.extend(["via", match.group(1)])
    argv.extend(["dev", interface])
    _require_success(command_runner(argv, 10.0), "CloudBase NCM host route")


def build_sensor_commands(work_root: Path) -> tuple[list[str], list[str]]:
    dx_dir = work_root / "dx_gp21_tracker"
    bmi_dir = work_root / "linux_bmi270_backpack"
    dx_command = [
        "python3",
        str(dx_dir / "dx_gp21_tracker.py"),
        "--config",
        str(dx_dir / "config.ss928_uart4.json"),
        "--serial-device",
        "/dev/ttyAMA4",
        "--baud",
        "115200",
        "--no-ble",
    ]
    bmi_command = [
        "python3",
        str(bmi_dir / "bmi270_backpack.py"),
        "--config",
        str(bmi_dir / "config.ss928_ble.json"),
        "--backend",
        "i2c",
        "--i2c-bus",
        "0",
        "--i2c-addr",
        "0x68",
        "--i2c-mux-addr",
        "0x70",
        "--i2c-mux-channel",
        "0",
        "--no-ble",
    ]
    return dx_command, bmi_command


def run_command(argv: list[str], timeout_s: float = 30.0) -> RunResult:
    completed = subprocess.run(
        argv,
        capture_output=True,
        text=True,
        timeout=timeout_s,
        check=False,
    )
    return RunResult(completed.returncode, completed.stdout, completed.stderr)


def _require_success(result: RunResult, label: str) -> None:
    if result.returncode != 0:
        detail = result.stderr.strip().splitlines()[-1] if result.stderr.strip() else f"rc={result.returncode}"
        raise RuntimeError(f"{label} failed: {detail}")


def _extract_operator(lines: Iterable[str]) -> str:
    for line in lines:
        match = re.search(r'\+COPS:\s*\d+\s*,\s*\d+\s*,\s*"([^"]+)"', line)
        if match:
            return match.group(1)
    return ""


def _extract_imsi(lines: Iterable[str]) -> str:
    for line in lines:
        if re.fullmatch(r"\d{10,20}", line.strip()):
            return line.strip()
    return ""


class AtSession:
    def __init__(self, port: str, baud: int = 115200) -> None:
        self.port = port
        self.baud = baud
        self.fd: int | None = None

    def __enter__(self) -> "AtSession":
        import termios

        self.fd = os.open(self.port, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
        attrs = termios.tcgetattr(self.fd)
        attrs[0] = 0
        attrs[1] = 0
        attrs[2] |= termios.CLOCAL | termios.CREAD | termios.CS8
        attrs[2] &= ~(termios.PARENB | termios.CSTOPB | termios.CRTSCTS)
        attrs[3] = 0
        speed = getattr(termios, f"B{self.baud}", termios.B115200)
        attrs[4] = speed
        attrs[5] = speed
        termios.tcsetattr(self.fd, termios.TCSANOW, attrs)
        termios.tcflush(self.fd, termios.TCIOFLUSH)
        return self

    def __exit__(self, exc_type: object, exc: object, traceback: object) -> None:
        del exc_type, exc, traceback
        if self.fd is not None:
            os.close(self.fd)
            self.fd = None

    def command(self, command: str, timeout_s: float = 4.0) -> list[str]:
        if self.fd is None:
            raise RuntimeError("AT session is not open")
        os.write(self.fd, (command + "\r\n").encode("ascii"))
        deadline = time.monotonic() + timeout_s
        raw = bytearray()
        lines: list[str] = []
        while time.monotonic() < deadline:
            readable, _, _ = select.select([self.fd], [], [], min(0.2, deadline - time.monotonic()))
            if not readable:
                continue
            try:
                raw.extend(os.read(self.fd, 4096))
            except BlockingIOError:
                continue
            decoded = raw.decode("utf-8", errors="replace")
            lines = [item.strip() for item in re.split(r"[\r\n]+", decoded) if item.strip()]
            filtered = [line for line in lines if line != command]
            if any(
                line in TERMINAL_LINES
                or line.startswith("+CME ERROR")
                or line.startswith("+CMS ERROR")
                or line.startswith("ERROR:")
                for line in filtered
            ):
                return filtered
        return [line for line in lines if line != command]


def prepare_network(
    at_session: object,
    command_runner: CommandRunner = run_command,
    sys_class_net: Path = Path("/sys/class/net"),
    resolver: Callable[[str], str] = socket.gethostbyname,
    apn_override: str = "",
    expected_usb_identity: tuple[str, str] | None = None,
) -> dict[str, str]:
    at_lines: list[str] = []
    health = at_session.command("AT")
    if "OK" not in health:
        raise RuntimeError("MT5710 PCUI did not answer AT")
    sim_lines = at_session.command("AT+CPIN?")
    if not any("+CPIN: READY" in line for line in sim_lines):
        raise RuntimeError("MT5710 SIM is not ready")
    operator_lines = at_session.command("AT+COPS?")
    imsi_lines = at_session.command("AT+CIMI")
    at_lines.extend(at_session.command("AT+CEREG?"))
    at_lines.extend(at_session.command("AT+C5GREG?"))
    at_lines.extend(at_session.command("AT+CGATT?"))
    registered, attached = parse_registration(at_lines)
    if not registered or not attached:
        raise RuntimeError("MT5710 is not registered/attached")
    apn = choose_apn(_extract_operator(operator_lines), _extract_imsi(imsi_lines), apn_override)
    dial_command = f'AT^NDISDUP=1,1,"{apn}"'
    dial_lines = at_session.command(dial_command, timeout_s=12.0)
    already_active = any(line == "ERROR: DUPLICATED" for line in dial_lines)
    if not already_active and any("ERROR" in line or "NO NETWORK SERVICE" in line for line in dial_lines):
        raise RuntimeError("MT5710 NCM dial failed")
    if (
        not already_active
        and "OK" not in dial_lines
        and not any("^NDISSTAT: 1,1" in line for line in dial_lines)
    ):
        raise RuntimeError("MT5710 NCM dial gave no success response")

    interface = discover_ncm_interface(sys_class_net, expected_usb_identity)
    _require_success(command_runner(["ip", "link", "set", interface, "up"], 10.0), "NCM link up")
    _require_success(
        command_runner(["udhcpc", "-i", interface, "-q", "-n", "-t", "8", "-T", "5"], 50.0),
        "NCM DHCP",
    )
    cloud_ip = resolver(CLOUDBASE_HOST)
    route_result = command_runner(["ip", "route", "get", cloud_ip], 10.0)
    _require_success(route_result, "CloudBase route lookup")
    added_route = ""
    if not route_uses_interface(route_result.stdout, interface):
        _add_ncm_host_route(interface, cloud_ip, command_runner)
        added_route = cloud_ip
        route_result = command_runner(["ip", "route", "get", cloud_ip], 10.0)
        _require_success(route_result, "CloudBase route recheck")
        if not route_uses_interface(route_result.stdout, interface):
            raise RuntimeError(f"CloudBase route is not using {interface}")
    return {"interface": interface, "apn": apn, "cloud_ip": cloud_ip, "added_route": added_route}


def _cleanup_network_route(network: dict[str, str] | None, command_runner: CommandRunner) -> None:
    if not network or not network.get("added_route"):
        return
    command_runner(
        ["ip", "route", "del", f"{network['added_route']}/32", "dev", network["interface"]],
        10.0,
    )


def _require_runtime_files(work_root: Path) -> None:
    required = [
        work_root / "smartbag_cloud_uploader" / "telemetry_client.py",
        work_root / "dx_gp21_tracker" / "dx_gp21_tracker.py",
        work_root / "dx_gp21_tracker" / "config.ss928_uart4.json",
        work_root / "linux_bmi270_backpack" / "bmi270_backpack.py",
        work_root / "linux_bmi270_backpack" / "posture_cloud.py",
        work_root / "linux_bmi270_backpack" / "config.ss928_ble.json",
    ]
    missing = [str(path) for path in required if not path.is_file()]
    if missing:
        raise RuntimeError("missing runtime file: " + missing[0])
    for device in (Path("/dev/ttyAMA4"), Path("/dev/i2c-0")):
        if not device.exists():
            raise RuntimeError(f"missing device: {device}")
    for executable in ("python3", "bspmm", "ip", "udhcpc"):
        if shutil.which(executable) is None:
            raise RuntimeError(f"missing executable: {executable}")


def _configure_pinmux(command_runner: CommandRunner) -> None:
    settings = [
        ["bspmm", "0x102F0134", "0x1201"],
        ["bspmm", "0x102F0138", "0x1201"],
        ["bspmm", "0x102F013c", "0x2031"],
        ["bspmm", "0x102F0140", "0x2031"],
    ]
    for argv in settings:
        _require_success(command_runner(argv, 10.0), "SS928 pinmux")


def _supervise(commands: Sequence[list[str]], work_root: Path) -> int:
    children: list[subprocess.Popen[bytes]] = []
    stopping = False

    def stop_handler(signum: int, frame: object) -> None:
        del signum, frame
        nonlocal stopping
        stopping = True

    signal.signal(signal.SIGINT, stop_handler)
    signal.signal(signal.SIGTERM, stop_handler)
    try:
        for command in commands:
            child = subprocess.Popen(command, cwd=str(work_root))
            children.append(child)
            print(f"Started pid={child.pid}: {Path(command[1]).name}", flush=True)
        while not stopping:
            for child in children:
                code = child.poll()
                if code is not None:
                    print(f"Producer pid={child.pid} exited rc={code}", file=sys.stderr, flush=True)
                    return code if code != 0 else 1
            time.sleep(0.5)
        return 0
    finally:
        for child in children:
            if child.poll() is None:
                child.terminate()
        deadline = time.monotonic() + 5.0
        for child in children:
            remaining = max(0.0, deadline - time.monotonic())
            try:
                child.wait(timeout=remaining)
            except subprocess.TimeoutExpired:
                child.kill()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", default="/dev/ttyUSB1", help="MT5710 PCUI serial port")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--apn", default="", help="APN override; normally auto-detected")
    parser.add_argument(
        "--work-root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="Board work directory containing the existing producers",
    )
    parser.add_argument("--skip-network", action="store_true", help="Skip MT5710 dialing for local diagnosis")
    parser.add_argument("--check-only", action="store_true", help="Bring up and verify 5G, then exit")
    return parser


def run_application(
    args: argparse.Namespace,
    *,
    environ: dict[str, str] | os._Environ[str] = os.environ,
    prerequisite_checker: Callable[[Path], None] = _require_runtime_files,
    network_preparer: Callable[..., dict[str, str]] = prepare_network,
    at_factory: Callable[[str, int], object] = AtSession,
    command_runner: CommandRunner = run_command,
    pinmux_configurer: Callable[[CommandRunner], None] = _configure_pinmux,
    supervisor: Callable[[Sequence[list[str]], Path], int] = _supervise,
) -> int:
    work_root = args.work_root.resolve()
    if not args.check_only:
        if not environ.get("SMARTBAG_UPLOAD_TOKEN", ""):
            raise RuntimeError("SMARTBAG_UPLOAD_TOKEN is not set")
        prerequisite_checker(work_root)

    network: dict[str, str] | None = None
    try:
        if not args.skip_network:
            if not Path(args.port).exists():
                raise RuntimeError(f"missing MT5710 PCUI port: {args.port}")
            tty_device = Path("/sys/class/tty") / Path(args.port).name / "device"
            expected_identity = _usb_identity(tty_device)
            with at_factory(args.port, args.baud) as at_session:
                network = network_preparer(
                    at_session,
                    command_runner=command_runner,
                    apn_override=args.apn,
                    expected_usb_identity=expected_identity,
                )
            print(
                f"5G ready: interface={network['interface']} apn_configured=yes "
                f"cloud_ip={network['cloud_ip']}",
                flush=True,
            )
        if args.check_only:
            return 0
        pinmux_configurer(command_runner)
        commands = build_sensor_commands(work_root)
        print("Starting real DX-GP21-A and BMI270 CloudBase producers", flush=True)
        return supervisor(commands, work_root)
    finally:
        _cleanup_network_route(network, command_runner)


def main(argv: Sequence[str] | None = None) -> int:
    args = build_parser().parse_args(argv)
    if hasattr(os, "geteuid") and os.geteuid() != 0:
        print("ERROR: run as root on BOARD-LINUX", file=sys.stderr)
        return 2
    try:
        return run_application(args)
    except (OSError, RuntimeError, subprocess.TimeoutExpired) as exc:
        print(f"ERROR: {exc}", file=sys.stderr, flush=True)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())

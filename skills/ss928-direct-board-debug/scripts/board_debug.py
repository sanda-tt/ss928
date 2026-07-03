#!/usr/bin/env python3
"""SSH/SFTP helper for the SS928 board in this workspace."""

from __future__ import annotations

import argparse
import os
import posixpath
from pathlib import Path
from typing import Iterable

import paramiko


DEFAULT_HOST = "192.168.1.168"
DEFAULT_USER = "root"
DEFAULT_PASSWORD = "ebaina"
DEFAULT_REMOTE_BMI270 = "/root/linux_bmi270_backpack"


def connect(args: argparse.Namespace) -> paramiko.SSHClient:
    password = args.password or os.environ.get("SS928_BOARD_PASSWORD") or DEFAULT_PASSWORD
    if not password:
        raise SystemExit("Missing password. Set SS928_BOARD_PASSWORD or pass --password.")

    client = paramiko.SSHClient()
    client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
    client.connect(
        args.host,
        username=args.user,
        password=password,
        timeout=10,
        banner_timeout=10,
        auth_timeout=10,
    )
    return client


def run(client: paramiko.SSHClient, command: str, timeout: int = 30) -> int:
    stdin, stdout, stderr = client.exec_command(command, timeout=timeout)
    out = stdout.read().decode("utf-8", errors="replace")
    err = stderr.read().decode("utf-8", errors="replace")
    rc = stdout.channel.recv_exit_status()
    print(f"\n$ {command}\n[rc={rc}]")
    if out.strip():
        print(out.rstrip())
    if err.strip():
        print("[stderr]")
        print(err.rstrip())
    return rc


def ensure_remote_dir(sftp: paramiko.SFTPClient, path: str) -> None:
    current = ""
    for part in [p for p in path.split("/") if p]:
        current += "/" + part
        try:
            sftp.stat(current)
        except OSError:
            sftp.mkdir(current)


def upload_folder(
    client: paramiko.SSHClient,
    local_dir: Path,
    remote_dir: str,
    skip_names: Iterable[str] = ("__pycache__",),
) -> None:
    local_dir = local_dir.resolve()
    if not local_dir.is_dir():
        raise SystemExit(f"Local folder not found: {local_dir}")

    sftp = client.open_sftp()
    try:
        ensure_remote_dir(sftp, remote_dir)
        for path in sorted(local_dir.rglob("*")):
            rel = path.relative_to(local_dir)
            if any(part in skip_names for part in rel.parts):
                continue
            remote_path = posixpath.join(remote_dir, *rel.parts)
            if path.is_dir():
                ensure_remote_dir(sftp, remote_path)
                continue
            ensure_remote_dir(sftp, posixpath.dirname(remote_path))
            sftp.put(str(path), remote_path)
            if path.suffix == ".sh":
                sftp.chmod(remote_path, 0o755)
    finally:
        sftp.close()


def command_probe(args: argparse.Namespace) -> int:
    client = connect(args)
    try:
        commands = [
            "hostname; uname -a",
            "python3 --version || python --version",
            "ls -l /dev/i2c-* 2>/dev/null || true",
            "command -v bspmm || true",
            "btmgmt info 2>&1 || true",
            "printf 'show\\nquit\\n' | bluetoothctl 2>&1 | sed -n '1,90p'",
        ]
        rc = 0
        for command in commands:
            rc = max(rc, run(client, command, timeout=25))
        return rc
    finally:
        client.close()


def command_upload(args: argparse.Namespace) -> int:
    client = connect(args)
    try:
        upload_folder(client, Path(args.local), args.remote)
        run(client, f"find {args.remote} -maxdepth 2 -type f | sort | sed -n '1,80p'")
        return 0
    finally:
        client.close()


def command_run(args: argparse.Namespace) -> int:
    client = connect(args)
    try:
        return run(client, args.command, timeout=args.timeout)
    finally:
        client.close()


def command_start_bmi270_ble(args: argparse.Namespace) -> int:
    client = connect(args)
    try:
        upload_folder(client, Path(args.local), args.remote)
        commands = [
            f"cd {args.remote} && sh -n ./start_ss928_ble.sh",
            "for p in $(pgrep -f '[b]mi270_backpack.py' || true); do kill $p || true; done",
            (
                f"cd {args.remote} && rm -f /tmp/bmi270_ble.log && "
                "setsid sh ./start_ss928_ble.sh >/tmp/bmi270_ble.log 2>&1 < /dev/null &"
            ),
            (
                "sleep 4; ps -ef | grep '[b]mi270_backpack.py' || true; "
                "grep -E 'BLE enabled|BLE GATT|BLE advertisement|WARN BLE|Using userspace' "
                "/tmp/bmi270_ble.log || true"
            ),
        ]
        rc = 0
        for command in commands:
            rc = max(rc, run(client, command, timeout=25))
        return rc
    finally:
        client.close()


def command_stop_bmi270(args: argparse.Namespace) -> int:
    client = connect(args)
    try:
        return run(
            client,
            "for p in $(pgrep -f '[b]mi270_backpack.py' || true); do kill $p || true; done; "
            "ps -ef | grep '[b]mi270_backpack.py' || true",
        )
    finally:
        client.close()


def command_log(args: argparse.Namespace) -> int:
    client = connect(args)
    try:
        return run(client, f"tail -n {args.lines} {args.path} 2>/dev/null || true", timeout=20)
    finally:
        client.close()


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="SS928 board SSH/SFTP debug helper")
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--user", default=DEFAULT_USER)
    parser.add_argument("--password", default="")
    sub = parser.add_subparsers(dest="action", required=True)

    sub.add_parser("probe")

    upload = sub.add_parser("upload")
    upload.add_argument("--local", required=True)
    upload.add_argument("--remote", required=True)

    run_cmd = sub.add_parser("run")
    run_cmd.add_argument("command")
    run_cmd.add_argument("--timeout", type=int, default=30)

    start = sub.add_parser("start-bmi270-ble")
    start.add_argument(
        "--local",
        default=str(Path.cwd() / "work" / "linux_bmi270_backpack"),
    )
    start.add_argument("--remote", default=DEFAULT_REMOTE_BMI270)

    sub.add_parser("stop-bmi270")

    log = sub.add_parser("log")
    log.add_argument("--path", default="/tmp/bmi270_ble.log")
    log.add_argument("--lines", type=int, default=80)
    return parser


def main() -> int:
    parser = build_parser()
    args = parser.parse_args()
    if args.action == "probe":
        return command_probe(args)
    if args.action == "upload":
        return command_upload(args)
    if args.action == "run":
        return command_run(args)
    if args.action == "start-bmi270-ble":
        return command_start_bmi270_ble(args)
    if args.action == "stop-bmi270":
        return command_stop_bmi270(args)
    if args.action == "log":
        return command_log(args)
    parser.error(f"unknown action: {args.action}")
    return 2


if __name__ == "__main__":
    raise SystemExit(main())

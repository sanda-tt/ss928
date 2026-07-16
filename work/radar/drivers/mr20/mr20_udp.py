"""Windows UDP receiver for MR20 target trajectory data."""

from __future__ import annotations

import argparse
import csv
import socket
from datetime import datetime, timezone
from pathlib import Path
from typing import Optional

from mr20_parser import MR20FrameError, parse_frame
from radar_target import RadarTarget

DEFAULT_PORT = 2368
DEFAULT_LOG_PATH = Path(__file__).resolve().parents[2] / "logs" / "radar.log"


class MR20UdpReceiver:
    """Bind a UDP endpoint, filter optional source IP, and log target frames."""

    def __init__(
        self,
        host: str = "0.0.0.0",
        port: int = DEFAULT_PORT,
        radar_ip: Optional[str] = None,
        log_path: Path = DEFAULT_LOG_PATH,
        timeout_s: Optional[float] = None,
    ) -> None:
        self.radar_ip = radar_ip
        self.log_path = Path(log_path)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind((host, port))
        self.socket.settimeout(timeout_s)

    @property
    def local_address(self) -> tuple[str, int]:
        return self.socket.getsockname()

    def close(self) -> None:
        self.socket.close()

    def __enter__(self) -> "MR20UdpReceiver":
        return self

    def __exit__(self, *_: object) -> None:
        self.close()

    def receive_target(self) -> tuple[RadarTarget, tuple[str, int]]:
        """Block until a valid 0x60B frame is received and logged."""
        while True:
            payload, source = self.socket.recvfrom(2048)
            if self.radar_ip is not None and source[0] != self.radar_ip:
                continue
            try:
                target = parse_frame(payload)
            except MR20FrameError:
                continue
            self._append_log(target)
            return target, source

    def _append_log(self, target: RadarTarget) -> None:
        self.log_path.parent.mkdir(parents=True, exist_ok=True)
        timestamp = datetime.now(timezone.utc).astimezone().isoformat(timespec="milliseconds")
        with self.log_path.open("a", newline="", encoding="utf-8") as file:
            csv.writer(file).writerow(target.to_csv_fields(timestamp))


def main() -> None:
    parser = argparse.ArgumentParser(description="Receive and parse MR20 UDP target frames.")
    parser.add_argument("--host", default="0.0.0.0", help="local bind IP (default: all interfaces)")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help="local UDP port (default: 2368)")
    parser.add_argument("--radar-ip", help="only accept datagrams from this radar IP")
    parser.add_argument("--log", type=Path, default=DEFAULT_LOG_PATH, help="CSV log path")
    args = parser.parse_args()

    with MR20UdpReceiver(args.host, args.port, args.radar_ip, args.log) as receiver:
        host, port = receiver.local_address
        print(f"Listening for MR20 UDP data on {host}:{port}")
        if args.radar_ip:
            print(f"Accepting only radar IP: {args.radar_ip}")
        print(f"Logging to: {args.log}")
        while True:
            target, _source = receiver.receive_target()
            print(target.to_console())
            print()


if __name__ == "__main__":
    main()

"""MR20 UDP parser and receiver verification entry point.

Run all offline checks with:
    python test_mr20.py
"""

from __future__ import annotations

import socket
import sys
import tempfile
import threading
import time
import unittest
from pathlib import Path

MODULE_DIR = Path(__file__).resolve().parent
if str(MODULE_DIR) not in sys.path:
    sys.path.insert(0, str(MODULE_DIR))

from mr20_parser import MR20FrameError, parse_frame
from mr20_udp import MR20UdpReceiver
from radar_target import RadarTarget


def target_frame(payload: bytes) -> bytes:
    """Build one protocol-compliant 0x60B serial-over-UDP target frame."""
    return bytes((0xAA, 0xAA, 0x0B, 0x06)) + payload + bytes((0x55, 0x55))


class MR20ParserTests(unittest.TestCase):
    def test_parses_official_target_example(self) -> None:
        target = parse_frame(target_frame(bytes((0x57, 0x9D, 0x34, 0x1D, 0x47, 0xA0, 0x02, 0x00))))

        self.assertIsInstance(target, RadarTarget)
        self.assertEqual(target.target_id, 87)
        self.assertEqual(target.x_distance_m, 3.0)
        self.assertEqual(target.y_distance_m, 3.0)
        self.assertEqual(target.x_velocity_mps, 0.0)
        self.assertEqual(target.y_velocity_mps, -56.5)
        self.assertEqual(target.status, "going")

    def test_rejects_invalid_frame(self) -> None:
        with self.assertRaises(MR20FrameError):
            parse_frame(b"\xAA\xAA\x0B")

    def test_formats_target_for_console_and_csv(self) -> None:
        target = RadarTarget(1, 1.2, 15.5, 0.1, -5.2, "going")

        self.assertEqual(
            target.to_console(),
            "Target ID:1\nPosition:\n x=1.2m\n y=15.5m\n\nVelocity:\n vx=0.1m/s\n vy=-5.2m/s\n\nStatus:going",
        )
        fields = target.to_csv_fields("2026-07-16T12:00:00+08:00")
        self.assertEqual(fields, ("2026-07-16T12:00:00+08:00", 1, 1.2, 15.5, 0.1, -5.2))


class MR20UdpReceiverTests(unittest.TestCase):
    def test_receives_udp_target_and_appends_csv_log(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            log_path = Path(temp_dir) / "radar.log"
            with MR20UdpReceiver(host="127.0.0.1", port=0, log_path=log_path, timeout_s=1.0) as receiver:
                port = receiver.local_address[1]
                sender = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                try:
                    sender.sendto(
                        target_frame(bytes((1, 0x9D, 0x34, 0x1D, 0x47, 0xA0, 0x02, 0x00))),
                        ("127.0.0.1", port),
                    )
                    target, source = receiver.receive_target()
                finally:
                    sender.close()

            self.assertEqual(source[0], "127.0.0.1")
            self.assertEqual(target.target_id, 1)
            rows = log_path.read_text(encoding="utf-8").strip().splitlines()
            self.assertEqual(len(rows), 1)
            self.assertEqual(len(rows[0].split(",")), 6)
            self.assertIn(",1,3.0,3.0,0.0,-56.5", rows[0])


if __name__ == "__main__":
    unittest.main(verbosity=2)

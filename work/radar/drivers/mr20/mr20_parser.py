"""Parser for the MR20 14-byte serial-over-UDP target frame."""

from __future__ import annotations

from radar_target import RadarTarget

FRAME_SIZE = 14
FRAME_HEAD = b"\xAA\xAA"
FRAME_TAIL = b"\x55\x55"
TARGET_FRAME_ID_LE = b"\x0B\x06"  # CAN ID 0x60B, low byte first.

STATUS_BY_VALUE = {
    0: "stopped",
    1: "oncoming",
    2: "going",
    3: "crossing",
}


class MR20FrameError(ValueError):
    """Raised when a UDP payload is not a complete 0x60B target frame."""


def parse_frame(frame: bytes) -> RadarTarget:
    """Decode one official MR20 Object_1_General (0x60B) frame.

    Frame layout is AA AA, CAN-ID little-endian, eight data bytes, 55 55.
    The formulas and dynamic-property bit field come from protocol sections
    7.2 and 9.4.
    """
    if len(frame) != FRAME_SIZE:
        raise MR20FrameError(f"MR20 frame must be {FRAME_SIZE} bytes, got {len(frame)}")
    if frame[:2] != FRAME_HEAD or frame[-2:] != FRAME_TAIL:
        raise MR20FrameError("invalid MR20 frame head or tail")
    if frame[2:4] != TARGET_FRAME_ID_LE:
        raise MR20FrameError(f"not an MR20 target frame: 0x{frame[3]:02X}{frame[2]:02X}")

    target_id = frame[4]
    # Normalize to each field's official resolution to avoid binary-float
    # artifacts such as 3.000000000000014 in console output and CSV logs.
    y_distance_m = round((frame[5] * 32 + (frame[6] >> 3)) * 0.1 - 500.0, 1)
    x_distance_m = round((((frame[6] & 0x07) * 256 + frame[7]) * 0.1) - 102.3, 1)
    y_velocity_mps = round(((frame[8] << 2) + (frame[9] >> 6)) * 0.25 - 128.0, 2)
    x_velocity_mps = round(((frame[9] & 0x3F) * 8 + (frame[10] >> 5)) * 0.25 - 64.0, 2)
    status = STATUS_BY_VALUE.get(frame[10] & 0x07, "unknown")

    return RadarTarget(
        target_id=target_id,
        x_distance_m=x_distance_m,
        y_distance_m=y_distance_m,
        x_velocity_mps=x_velocity_mps,
        y_velocity_mps=y_velocity_mps,
        status=status,
    )

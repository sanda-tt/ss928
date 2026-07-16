#!/usr/bin/env python3
"""Local SS928 telemetry uploader simulator. It never embeds device credentials."""

import argparse
import hashlib
import hmac
import json
import os
import sys
import time
import urllib.error
import urllib.request
import uuid


def require_env(name):
    value = os.environ.get(name, "")
    if not value:
        raise RuntimeError("Missing required environment variable: " + name)
    return value


def build_payload(device_id, timestamp):
    reported_at = timestamp * 1000
    return {
        "version": 1,
        "deviceId": device_id,
        "requestId": "sim-" + uuid.uuid4().hex,
        "reportedAt": reported_at,
        "status": {
            "location": {"latitude": 30.65736, "longitude": 104.0832, "accuracyM": 6.5, "valid": True},
            "attitude": {"rollDeg": 1.2, "pitchDeg": -3.4, "yawDeg": 92.1},
            "risk": {"level": 1, "type": "rear_vehicle", "direction": "left_rear"},
            "battery": {"percent": 82},
            "network": {"type": "5g-redcap", "rssiDbm": -81},
            "firmwareVersion": "sim-v1"
        },
        "trackPoints": [],
        "alarms": []
    }


def make_signature(device_id, timestamp, nonce, raw_body, device_secret):
    body_hash = hashlib.sha256(raw_body).hexdigest()
    signing_string = "\n".join([device_id, str(timestamp), nonce, body_hash])
    return hmac.new(device_secret.encode("utf-8"), signing_string.encode("utf-8"), hashlib.sha256).hexdigest()


def main():
    parser = argparse.ArgumentParser(description="Simulate one signed SS928 telemetry upload")
    parser.add_argument("--bad-signature", action="store_true", help="Send an intentionally invalid signature")
    args = parser.parse_args()

    try:
        device_id = require_env("DEVICE_ID")
        device_secret = require_env("DEVICE_SECRET")
        upload_url = require_env("UPLOAD_URL")
    except RuntimeError as error:
        print(str(error), file=sys.stderr)
        return 2

    timestamp = int(time.time())
    nonce = "sim-" + uuid.uuid4().hex
    raw_body = json.dumps(build_payload(device_id, timestamp), separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    signature = make_signature(device_id, timestamp, nonce, raw_body, device_secret)
    if args.bad_signature:
        signature = "0" * 64
    request = urllib.request.Request(upload_url, data=raw_body, method="POST")
    request.add_header("Content-Type", "application/json")
    request.add_header("X-Device-Id", device_id)
    request.add_header("X-Timestamp", str(timestamp))
    request.add_header("X-Nonce", nonce)
    request.add_header("X-Signature", signature)
    try:
        with urllib.request.urlopen(request, timeout=15) as response:
            print("HTTP " + str(response.status))
            print(response.read().decode("utf-8"))
            return 0
    except urllib.error.HTTPError as error:
        print("HTTP " + str(error.code))
        print(error.read().decode("utf-8"))
        return 1
    except urllib.error.URLError as error:
        print("Request failed: " + str(error.reason), file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())

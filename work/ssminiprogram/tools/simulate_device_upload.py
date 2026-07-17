#!/usr/bin/env python3
"""Upload one fixed competition-demo telemetry record for bag001."""

import json
import os
import sys
import time
import urllib.error
import urllib.request


def require_env(name):
    value = os.environ.get(name, "")
    if not value:
        raise RuntimeError("Missing required environment variable: " + name)
    return value


def build_payload():
    reported_at = int(time.time() * 1000)
    location = {"latitude": 30.65736, "longitude": 104.0832}
    return {
        "status": {
            "reportedAt": reported_at,
            "location": location,
            "attitude": {"rollDeg": 1.2, "pitchDeg": -3.4, "yawDeg": 92.1},
            "risk": {"level": 1, "type": "rear_vehicle", "direction": "left_rear"},
            "battery": {"percent": 82},
            "network": {"type": "5g-redcap", "rssiDbm": -81},
            "firmwareVersion": "sim-v1"
        },
        "trackPoints": [{"reportedAt": reported_at, "location": location, "speed": 1.2, "heading": 92}],
        "alarms": [{"reportedAt": reported_at, "alarmType": "rear_vehicle", "riskLevel": 2, "direction": "left_rear", "message": "vehicle approaching", "location": location}]
    }


def main():
    try:
        upload_url = require_env("UPLOAD_URL")
        upload_token = require_env("UPLOAD_TOKEN")
    except RuntimeError as error:
        print(str(error), file=sys.stderr)
        return 2
    raw_body = json.dumps(build_payload(), separators=(",", ":"), ensure_ascii=False).encode("utf-8")
    request = urllib.request.Request(upload_url, data=raw_body, method="POST")
    request.add_header("Content-Type", "application/json")
    request.add_header("X-Upload-Token", upload_token)
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

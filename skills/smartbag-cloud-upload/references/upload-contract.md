# Smartbag upload contract

## Endpoint

```text
POST https://cloud1-d7gdmg27139f4fbf2.service.tcloudbase.com/smartbag-device-ingest/v1/device/telemetry
```

Headers:

```text
Content-Type: application/json
X-Upload-Token: <比赛上传令牌>
```

## Telemetry v1 payload

Use this overall shape:

```json
{
  "version": 1,
  "deviceId": "bag001",
  "requestId": "unique-request-id",
  "reportedAt": 1784160000000,
  "status": {
    "location": {
      "latitude": 30.6701,
      "longitude": 104.0602
    },
    "attitude": {
      "roll": 8.2,
      "pitch": 67.5,
      "yaw": 12.4
    },
    "risk": {
      "level": 3,
      "type": "fall_detected"
    },
    "battery": {
      "percent": 80
    },
    "network": {
      "online": true
    },
    "firmwareVersion": "v1.0.0"
  },
  "trackPoints": [],
  "alarms": []
}
```

Before implementation, confirm field names against `telemetry-core.js`. Remove optional fields that the current module cannot provide.

## Fall event example

```json
{
  "version": 1,
  "deviceId": "bag001",
  "requestId": "fall-1784160000000",
  "reportedAt": 1784160000000,
  "status": {
    "location": {
      "latitude": 30.6701,
      "longitude": 104.0602
    },
    "attitude": {
      "roll": 8.2,
      "pitch": 67.5,
      "yaw": 12.4
    },
    "risk": {
      "level": 3,
      "type": "fall_detected"
    }
  },
  "trackPoints": [],
  "alarms": [
    {
      "alarmType": "fall_detected",
      "riskLevel": 3,
      "message": "检测到用户摔倒",
      "reportedAt": 1784160000000,
      "location": {
        "latitude": 30.6701,
        "longitude": 104.0602
      }
    }
  ]
}
```

## Track upload example

```json
{
  "version": 1,
  "deviceId": "bag001",
  "requestId": "track-1784160000000",
  "reportedAt": 1784160000000,
  "status": {
    "location": {
      "latitude": 30.6701,
      "longitude": 104.0602
    }
  },
  "trackPoints": [
    {
      "latitude": 30.6701,
      "longitude": 104.0602,
      "reportedAt": 1784160000000
    }
  ],
  "alarms": []
}
```

## Minimal Python helper

```python
import os
import time
import uuid
from typing import Any

import requests

UPLOAD_URL = (
    "https://cloud1-d7gdmg27139f4fbf2.service.tcloudbase.com/"
    "smartbag-device-ingest/v1/device/telemetry"
)


def upload_telemetry(payload: dict[str, Any]) -> bool:
    token = os.environ.get("SMARTBAG_UPLOAD_TOKEN", "")
    if not token:
        print("Cloud upload skipped: SMARTBAG_UPLOAD_TOKEN is not set")
        return False

    try:
        response = requests.post(
            UPLOAD_URL,
            json=payload,
            headers={
                "Content-Type": "application/json",
                "X-Upload-Token": token,
            },
            timeout=5,
        )
        response.raise_for_status()
        return True
    except requests.RequestException as exc:
        print(f"Cloud upload failed: {exc}")
        return False


def report_fall_event(
    latitude: float | None,
    longitude: float | None,
    roll: float,
    pitch: float,
    yaw: float,
) -> bool:
    now_ms = int(time.time() * 1000)
    location = {}
    if latitude is not None and longitude is not None:
        location = {"latitude": latitude, "longitude": longitude}

    payload = {
        "version": 1,
        "deviceId": "bag001",
        "requestId": f"fall-{now_ms}-{uuid.uuid4().hex[:8]}",
        "reportedAt": now_ms,
        "status": {
            "location": location,
            "attitude": {"roll": roll, "pitch": pitch, "yaw": yaw},
            "risk": {"level": 3, "type": "fall_detected"},
        },
        "trackPoints": [],
        "alarms": [
            {
                "alarmType": "fall_detected",
                "riskLevel": 3,
                "message": "检测到用户摔倒",
                "reportedAt": now_ms,
                "location": location,
            }
        ],
    }
    return upload_telemetry(payload)
```

Use the repository's existing HTTP library if `requests` is unavailable. Do not add a large framework for this helper.

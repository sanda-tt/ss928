# MR20 eth1 persistent radar network

## Goal

Dedicate the second SS928 Ethernet interface, `eth1`, to an MR20 Ethernet radar while retaining the SSH management path on `eth0`.

## Persistent board configuration

- Board file: `/etc/systemd/network/20-mr20-radar.network`.
- Network manager: `systemd-networkd` (systemd 249).
- MR20 default endpoint from the supplied guide and Python example: `192.168.1.200:2369`.
- Board receive endpoint: `192.168.1.102:2368`.
- `eth1` uses `192.168.1.102/32` and the precise link route `192.168.1.200/32 dev eth1 scope link`.

## Why the prefix is /32

`eth0` is already the SSH management interface on a different physical network using `192.168.1.168/24`. Giving `eth1` another ordinary `192.168.1.0/24` route would make the kernel choose ambiguously between two separate cables. The host address plus single-host route preserves the vendor IP plan while forcing all radar traffic to `eth1`; it does not add a default route or change `eth0`.

## Verification

- `networkctl reload` and `networkctl reconfigure eth1` completed successfully.
- `networkctl status eth1` reports `/etc/systemd/network/20-mr20-radar.network` as the active network file and address `192.168.1.102`.
- `ip route get 192.168.1.200` resolves to `eth1 src 192.168.1.102`.
- The radar cable was not connected at configuration time, so `eth1` showed `no-carrier`; Ethernet negotiation and actual UDP frames remain to be verified after connecting the MR20.

## Do not repeat

Do not change `eth1` to a normal `192.168.1.102/24` while `eth0` remains on `192.168.1.168/24`. Do not add a gateway or default route on `eth1`.

## 2026-07-20 live MR20 validation

- With the MR20 connected to `eth1`, the physical link negotiated at 100 Mbps full duplex and `ethtool` reported `Link detected: yes`.
- `ping -I eth1 192.168.1.200` received two replies with 0% loss and about 0.9 ms average latency.
- A temporary receiver bound to `192.168.1.102:2368` received 217 valid 14-byte MR20 UDP frames in eight seconds, all from `192.168.1.200:2369`.
- The frame mix was `0x60A:201`, `0x201:8`, and `0x700:8`; no `0x60B` target-object frame arrived during the capture. The transport and framing path are therefore proven, while live target decoding needs a person or moving object in the radar field of view.

## 2026-07-20 controller deployment audit

- `/root/smartbag_alert/controller/mr20_radar.py` and `/root/smartbag_alert/config/mr20_radar.json` matched their local SHA-256 hashes exactly.
- `smartbag-alert.service` was enabled but could not stay running: UDP port `2368` had no listener and the radar JSONL log did not exist.
- The service was in an auto-restart loop because `/root/smartbag_alert/controller/start_ss928_smartbag_alert.sh` contained 32 CRLF line endings. Under board `/bin/sh`, `set -eu` became `set -eu\\r` and exited with status 2 before the controller started.
- Required recovery: upload the startup script with LF line endings, then restart the exact `smartbag-alert.service` and verify its UDP listener, live JSONL records, and a target-triggered `0x60B` alert.

## 2026-07-20 startup-script repair result

- The local and board copies of `start_ss928_smartbag_alert.sh` were converted to LF and board `sh -n` validation passed. `.gitattributes` now pins `work/smartbag_alert_controller/*.sh` to LF so a future Windows checkout does not reintroduce CRLF.
- Restarting `smartbag-alert.service` passed the former shell error but exposed a separate pre-existing hardware blocker: PWM setup returned `EINVAL`, then TM6605 mux I2C writes returned `EIO`. The controller exits before it creates the MR20 UDP listener, so actual radar alert output remains unverified until the PWM/TM6605 initialization path is repaired or explicitly configured for the hardware present.

## 2026-07-20 final live state and follow-up

- `smartbag-alert.service` is enabled for boot and active at runtime. It owns UDP `192.168.1.102:2368`; the persistent `/32` eth1 address and host route to `192.168.1.200` remain in effect.
- With the repaired PWM startup sequence and the TCA9548A configuration enabled, the service stays active while BLE, MR20 receiver, dual haptics, and PWM lights are initialized.
- Live MR20 status frames continue to arrive at about 25 Hz from `192.168.1.200`, and are logged as zero-target scans. A moving target inside the configured field is still required to produce a live `0x60B` object frame and prove the end-to-end physical radar alarm.
- Keep eth0 untouched. Do not change eth1 to `/24`, add a gateway, or use the same 192.168.1.0/24 route on both board Ethernet ports.

## 2026-07-20 live moving-target alarm proof

- A real moving target produced MR20 target ID `12` on the right-rear radar. Logged samples were about `0.8–1.1 m` longitudinal, `-0.1–0.1 m` lateral, and `-1.25–-1.5 m/s` longitudinal velocity (`oncoming`).
- After the configured two-frame confirmation, the controller recorded 35 level-1 radar alerts from this target. This proves the live chain `MR20 0x60B target -> UDP receiver -> parser -> risk evaluator -> smartbag alert event`.
- The level is correctly 1: the measured closing speed is below the level-2 minimum of `2 m/s`, even though the target is close and has a short TTC. The current threshold design intentionally requires both the per-level minimum speed and either the per-level TTC or distance condition.

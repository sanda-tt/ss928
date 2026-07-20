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

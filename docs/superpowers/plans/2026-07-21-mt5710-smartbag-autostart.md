# MT5710 SmartBag Autostart Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the existing MT5710 SmartBag uploader start automatically in the background after SS928 power-on without blocking boot.

**Architecture:** Add one `Type=simple` systemd unit that runs the existing launcher and retries failures every five seconds. Reuse the existing BMI270 BLE wrapper inside the launcher, then install the unit while disabling the older standalone BMI270 unit to prevent duplicate sensor ownership.

**Tech Stack:** systemd, POSIX shell, Python 3 standard-library tests

## Global Constraints

- The unit must not wait for `network-online.target` or use `Type=oneshot`.
- All modem and DHCP waits remain bounded by the existing launcher timeouts.
- Preserve BMI270 BLE by invoking `start_ss928_ble.sh`.
- Keep the upload token only in `/root/config/smartbag-upload.env`.
- Do not modify `smartbag-alert.service` or permanent default-route configuration.

---

### Task 1: Non-blocking unit and producer command

**Files:**
- Create: `work/mt5710_5g_cloud_upload/smartbag-5g-upload.service`
- Modify: `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`
- Modify: `work/mt5710_5g_cloud_upload/tests/test_mt5710_5g_cloud_upload.py`
- Create: `work/mt5710_5g_cloud_upload/tests/test_autostart.py`

**Interfaces:**
- Consumes: `start_ss928_5g_upload.sh` and `build_sensor_commands(work_root)`.
- Produces: a systemd unit whose `ExecStart` runs the launcher and a BMI command that runs `start_ss928_ble.sh`.

- [ ] **Step 1: Write failing tests**

Add assertions that the BMI command equals `['/bin/sh', '<work-root>/linux_bmi270_backpack/start_ss928_ble.sh']`. Add a unit-file test requiring `Type=simple`, `Restart=always`, `RestartSec=5`, `TimeoutStopSec=10`, `KillMode=control-group`, `WantedBy=multi-user.target`, and forbidding `network-online.target`, `Type=oneshot`, and `ExecStartPre`.

- [ ] **Step 2: Verify RED**

Run: `python -m unittest discover -s work/mt5710_5g_cloud_upload/tests -p "test_*.py"`

Expected: failure because the service file is absent and the BMI command still invokes Python with `--no-ble`.

- [ ] **Step 3: Implement the minimum**

Use this unit contract:

```ini
[Unit]
Description=SmartBag MT5710 5G cloud upload
After=systemd-udevd.service
StartLimitIntervalSec=0

[Service]
Type=simple
WorkingDirectory=/root/work/mt5710_5g_cloud_upload
Environment=PYTHONUNBUFFERED=1
ExecStart=/root/work/mt5710_5g_cloud_upload/start_ss928_5g_upload.sh
Restart=always
RestartSec=5
TimeoutStopSec=10
KillMode=control-group

[Install]
WantedBy=multi-user.target
```

Change only the BMI child command to `/bin/sh <bmi-dir>/start_ss928_ble.sh`; leave DX and MT5710 behavior unchanged.

- [ ] **Step 4: Verify GREEN**

Run the same unittest command and expect all tests to pass.

- [ ] **Step 5: Commit**

Commit message: `feat: add non-blocking smartbag autostart unit`.

### Task 2: Installer, deployment, and boot proof

**Files:**
- Create: `work/mt5710_5g_cloud_upload/install_autostart.sh`
- Modify: `work/mt5710_5g_cloud_upload/tests/test_autostart.py`
- Modify: `work/mt5710_5g_cloud_upload/README.md`
- Modify: `docs/agent/history/mt5710-smartbag-cloud-upload.md`

**Interfaces:**
- Consumes: `smartbag-5g-upload.service` and root systemd.
- Produces: one root-only install command that installs/enables the unit and disables `bmi270-backpack.service`.

- [ ] **Step 1: Write failing installer test**

Require the installer to use `install -m 0644`, `systemctl daemon-reload`, `systemctl disable --now bmi270-backpack.service`, and `systemctl enable --now smartbag-5g-upload.service`.

- [ ] **Step 2: Verify RED**

Run the autostart test and expect failure because `install_autostart.sh` is absent.

- [ ] **Step 3: Implement and document**

Create a root-checked POSIX shell installer with those four exact operations. Document install, status, logs, disable commands, and the non-blocking/retry behavior.

- [ ] **Step 4: Verify locally and on BOARD-LINUX**

Run all MT5710 tests and `git diff --check`, upload the folder, then run `./install_autostart.sh`. Verify `systemctl is-enabled smartbag-5g-upload.service`, `systemctl show ... -p Type -p Restart -p RestartUSec -p TimeoutStopUSec`, and service logs.

- [ ] **Step 5: Reboot proof and commit**

Run `systemctl reboot`, wait for SSH to return, then confirm boot completion, the new service state, and its journal. Commit message: `feat: enable smartbag 5g upload at boot`.

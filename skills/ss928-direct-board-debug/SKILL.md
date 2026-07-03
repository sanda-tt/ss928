---
name: ss928-direct-board-debug
description: Direct SSH/SFTP operation workflow for the SS928/HiEuler Pi board in this workspace. Use when the user asks Codex to connect to the board, upload files, run board-side debug commands, start or stop a program, inspect logs, debug BMI270/BLE/peripheral bring-up directly, or summarize the live board debugging process.
---

# SS928 Direct Board Debug

## Defaults

- Workspace: `C:\Users\ASUS\Desktop\ss928`.
- Board target: `192.168.1.168`.
- SSH user: `root`.
- Default password: `ebaina`. Use it directly for this board unless the user gives a new password.
- Normal remote project path: `/root/<local-folder-name>`, for example `/root/linux_bmi270_backpack`.
- Use Ethernet SSH/SFTP for normal debugging. Use serial only when SSH fails or boot logs are needed.

## Safety Rules

- Start with read-only checks: port reachability, `hostname`, `uname -a`, `python3 --version`, `ls`.
- Do not edit network, SSH, boot, or system startup configuration unless the user explicitly asks.
- Do not use destructive cleanup. If a process must be restarted, kill only the exact project process with a narrow pattern such as `pgrep -f '[b]mi270_backpack.py'`.
- For long-running programs, always detach and log to `/tmp/<name>.log`; then reconnect and inspect process/log state.
- If a shell command may hang because a background process inherits SSH stdout/stderr, use `setsid ... >/tmp/name.log 2>&1 < /dev/null &` and verify from a fresh SSH command.

## Standard Workflow

1. Confirm reachability.
   - First check TCP 22 from the host if possible.
   - Then SSH with `root@192.168.1.168` and verify identity using `hostname; uname -a`.
2. Upload only the needed subtree.
   - Prefer SFTP/Paramiko for password-backed automation.
   - Skip transient folders such as `__pycache__`.
   - Preserve executable mode for `.sh` files.
3. Run the smallest hardware proof.
   - For I2C sensors: list `/dev/i2c-*`, set pinmux if required, probe chip ID, then run a short data stream.
   - For BLE: inspect `bluetoothctl show` and `btmgmt info` before starting extra BlueZ/dbus stacks.
4. Start the real program detached.
   - Kill only the previous exact process.
   - Start with `setsid` or `nohup`, redirect to `/tmp/...log`, and immediately check `ps` plus log markers.
5. Report exact status.
   - Say what was uploaded, where it lives, which checks passed, what is still unverified, and the one or two commands the user needs next.

## BMI270 BLE Pattern

For `work/linux_bmi270_backpack`:

- Remote path: `/root/linux_bmi270_backpack`.
- First proof:

```sh
cd /root/linux_bmi270_backpack
python3 bmi270_backpack.py --probe-i2c --i2c-bus 0
python3 bmi270_backpack.py --config config.ss928_ble.json --no-ble
```

- Expected I2C success: `/dev/i2c-0 addr=0x68 chip_id=0x24 BMI270`.
- Current board uses system BlueZ successfully. Prefer the default script path that skips `ble.sh`; use `START_BLE_STACK=1` only if no system controller exists.
- Detached start:

```sh
cd /root/linux_bmi270_backpack
for p in $(pgrep -f '[b]mi270_backpack.py' || true); do kill $p || true; done
rm -f /tmp/bmi270_ble.log
setsid sh ./start_ss928_ble.sh >/tmp/bmi270_ble.log 2>&1 < /dev/null &
```

- Success markers:

```text
BLE enabled as BMI270-Backpack
BLE GATT registered
BLE advertisement registered
```

- User-facing next step after success: open `work/ssminiprogram` in WeChat DevTools, real-device preview, scan `BMI270-Backpack`.

## Reusable Script

Use `scripts/board_debug.py` for repeatable SSH/SFTP work. It supports:

- `probe`: read-only board identity and Bluetooth/I2C visibility checks.
- `upload`: upload a local folder to a remote folder.
- `run`: run one command and print output.
- `start-bmi270-ble`: upload/start the BMI270 BLE service pattern.
- `log`: show a remote log tail.
- `stop-bmi270`: stop the BMI270 process narrowly.

Default usage does not need a password argument:

```powershell
python skills/ss928-direct-board-debug/scripts/board_debug.py probe
```

If the board password changes, override it with `--password` or `SS928_BOARD_PASSWORD`.

## Pitfalls

- `ble.sh 0` can start a temporary DBus/bluetoothd stack that conflicts with the system BlueZ service. On the verified board, system BlueZ already sees `hci0`; skip `ble.sh` unless `btmgmt info` shows no controller.
- `ble.sh` colorized export lines contain ANSI escape codes; do not blindly `eval` them.
- SSH commands that launch background programs may appear to hang if stdout/stderr are inherited. Detach with redirected stdin/stdout/stderr and check from a new command.
- A persistent roll near 90 degrees usually means module mounting orientation, not a communication failure.

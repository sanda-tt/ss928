# MT5710 SmartBag Real Upload Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add one executable SS928 entry point that establishes MT5710 NCM networking and starts all existing real SmartBag telemetry producers.

**Architecture:** A standard-library Python supervisor owns modem AT checks, APN selection, NCM DHCP, CloudBase route verification, pinmux, and child process lifetime. Existing DX-GP21-A and BMI270 programs remain the only producers and continue using `smartbag_cloud_uploader`.

**Tech Stack:** Python 3.10 standard library, Linux termios/select/subprocess/sysfs, BusyBox `udhcpc`, SS928 `bspmm`, `unittest`, POSIX shell.

## Global Constraints

- Keep the CloudBase URL, token header, payloads, and `deviceId=bag001` unchanged.
- Upload real DX-GP21-A tracks, processed BMI270 posture/daily totals, and final fall alarms only.
- Do not use MT5710 as GNSS.
- Do not create persistent network, systemd, boot, or CloudBase changes.
- Do not print or commit credentials.
- No physical hardware validation is claimed in this phase.

---

### Task 1: Testable MT5710 and launch policy

**Files:**
- Create: `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`
- Create: `work/mt5710_5g_cloud_upload/tests/test_mt5710_5g_cloud_upload.py`

**Interfaces:**
- Produces: `choose_apn(operator, imsi, override)`, `parse_registration(lines)`, `discover_ncm_interface(sys_class_net)`, `route_uses_interface(text, interface)`, `build_sensor_commands(work_root)`.

- [ ] **Step 1: Write failing pure-policy tests**

Add tests asserting China Telecom selects `ctnet`, Mobile selects `cmnet`, Unicom selects `3gnet`, an override wins, registered/attached responses are accepted, searching/detached responses are rejected, only a `cdc_ncm` sysfs device is selected, route output must name the NCM interface, and sensor commands contain real configs plus `--no-ble` but never `--simulate`.

- [ ] **Step 2: Run tests and verify RED**

Run: `python -B -m unittest discover -s work/mt5710_5g_cloud_upload/tests -v`

Expected: import failure because `mt5710_5g_cloud_upload.py` does not exist.

- [ ] **Step 3: Implement the pure helpers**

Implement the named functions with no third-party dependencies. APN mapping must use operator text first and IMSI MCC/MNC prefixes second. `discover_ncm_interface` must inspect sysfs driver symlinks and never assume an `enx...` name.

- [ ] **Step 4: Run tests and verify GREEN**

Run: `python -B -m unittest discover -s work/mt5710_5g_cloud_upload/tests -v`

Expected: all Task 1 tests pass.

### Task 2: One-command runtime supervisor

**Files:**
- Modify: `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`
- Modify: `work/mt5710_5g_cloud_upload/tests/test_mt5710_5g_cloud_upload.py`
- Create: `work/mt5710_5g_cloud_upload/start_ss928_5g_upload.sh`

**Interfaces:**
- Produces CLI flags `--port`, `--baud`, `--apn`, `--work-root`, `--skip-network`, and `--check-only`; default runtime starts both real producers.

- [ ] **Step 1: Write failing orchestration tests**

Use injected fake AT and command runners to assert the order: credential/prerequisite checks, health queries, dial, NCM discovery, DHCP, route verification, pinmux, then child launch. Assert any failed gate prevents child launch and no secret appears in messages.

- [ ] **Step 2: Run focused tests and verify RED**

Run: `python -B -m unittest discover -s work/mt5710_5g_cloud_upload/tests -v`

Expected: failures for missing orchestration interfaces.

- [ ] **Step 3: Implement minimal runtime**

Add a termios/select AT session, bounded command timeouts, subprocess runner, prerequisite checks, `AT^NDISDUP`, dynamic NCM DHCP, DNS resolution, `ip route get` verification, pinmux, child supervision, and narrow SIGINT/SIGTERM cleanup. The shell wrapper loads `/root/config/smartbag-upload.env` when present and uses `exec python3`.

- [ ] **Step 4: Verify runtime without hardware**

Run:

`python -B -m unittest discover -s work/mt5710_5g_cloud_upload/tests -v`

`python -m py_compile work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`

`python work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py --help`

Expected: tests pass, compilation exits zero, and help lists the documented flags.

### Task 3: Documentation and complete regression

**Files:**
- Create: `work/mt5710_5g_cloud_upload/README.md`
- Create: `docs/agent/history/mt5710-smartbag-cloud-upload.md`
- Modify: `docs/agent/history/README.md`

**Interfaces:**
- Documents: board wiring prerequisites, credential file format, one-command start, expected success markers, temporary route/DNS impact, and post-hardware CloudBase readback steps.

- [ ] **Step 1: Document the exact board command**

Document `BOARD-LINUX: cd /root/work/mt5710_5g_cloud_upload && ./start_ss928_5g_upload.sh`, required external antennas/SIM/sensor wiring, and that hardware is not validated yet.

- [ ] **Step 2: Run all relevant tests**

Run uploader, DX-GP21-A, BMI270, fall fusion, and new MT5710 suites, then `git diff --check`.

Expected: all tests pass and diff check exits zero.

- [ ] **Step 3: Record honest boundaries**

History must state that code paths and mocked orchestration are verified, while real MT5710 dialing, DHCP, sensor reads, 5G CloudBase POST, and database readback remain pending until hardware is connected.

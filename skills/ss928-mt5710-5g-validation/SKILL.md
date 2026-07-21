---
name: ss928-mt5710-5g-validation
description: Use when validating or operating the TD Tech MT5710/MT571X 5G RedCap module on SS928, including USB enumeration, PCUI AT commands, cellular data, voice calling, SmartBag severe-fall SMS alerts, GPS/GNSS status, or modem troubleshooting.
---

# SS928 MT5710 5G Validation

## Board Defaults

- Workspace: `C:\Users\ASUS\Desktop\ss928`.
- Board target: `root@192.168.1.168`.
- Read board credentials only from `.local/device-access.md`; never copy them into
  source, skills, logs, or reports.
- Prefer Ethernet SSH for normal testing. Use serial only if SSH fails or boot logs are needed.
- PCUI control port: `/dev/ttyUSB1`.
- GPS/NMEA reserved port from docs: `/dev/ttyUSB3`.
- Module observed on board: `TD Tech Ltd. TDTECH_MT571X`, `MT5710_CN`, firmware `V100R001C00B095`, product type `Mini-PCIe`.

## Safety

- Start with read-only checks before changing modem state.
- Do not edit persistent Linux network, SSH, startup, or boot configuration.
- Data dialing can change the board default route and `/etc/resolv.conf`; say this clearly after success.
- For repeated tests, discover the NCM interface dynamically instead of assuming `usb0`.

## Documentation Anchors

- MT5710 docs: `07硬件资料/03.外设模块资料/06.5G模组/5G RedCap模块/MT5710_TR6二开资料`.
- Use the MT5710 docs for this board. The `RM500U-CN` folder is a different Quectel module and is only cross-reference material.
- Key MT5710 documents:
  - `MT5710-CN  5G RedCap模组Linux内核驱动集成指导.pdf`
  - `MT5710-CN  5G RedCap模组USB接口应用指南.pdf`
  - `MT5710-CN 5G RedCap模组AT命令手册.pdf`
  - Mini PCIe hardware and development-board guides for antenna wiring.

## First Probe

Verify board identity and modem enumeration:

```sh
hostname
uname -a
ls -l /dev/ttyUSB*
for p in /dev/ttyUSB*; do
  [ -e "$p" ] || continue
  echo "--- $p"
  udevadm info -q property -n "$p" 2>/dev/null | egrep 'ID_USB_INTERFACE_NUM|ID_USB_DRIVER|ID_MODEL|ID_VENDOR|ID_PATH' || true
done
ip -br addr
```

Expected modem nodes:

- `/dev/ttyUSB0` through `/dev/ttyUSB3` exist.
- `/dev/ttyUSB1` is PCUI and responds to AT.
- The USB network device uses `cdc_ncm`.
- The network interface may be renamed from `usb0` to a MAC-based name such as `enx7ecca5347f7b`; always discover it with `ls /sys/class/net/enx*`.

## AT Health Check

Send commands on `/dev/ttyUSB1` at `115200 8N1`:

```text
AT
ATE1
AT+CPIN?
AT+CFUN?
AT^HCSQ?
AT+COPS?
AT+CEREG?
AT+C5GREG?
AT+CGATT?
AT+CGDCONT?
AT^SYSINFOEX
```

Good post-antenna state observed:

```text
+CPIN: READY
+CFUN: 1
^HCSQ: "NR",50,146,29
+CEREG: 2,1
+C5GREG: 2,1,"28041B","00000008FE090005",11,1,"01"
+CGATT: 1
^SYSINFOEX: 2,2,0,1,,11,"NR-5GC",111,"NR-5GC"
```

Interpretation:

- `C5GREG/CEREG stat=1` means registered on the home network.
- `CGATT=1` means packet data attached.
- For `^HCSQ: "NR",rsrp,rsrq,sinr`, estimate `NR RSRP dBm = (rsrp - 1) - 140`.
- Before external antennas were connected, RSRP was about `-126 dBm` and registration stayed in search. After antennas, RSRP improved to about `-91 dBm` and data attach succeeded.

## Data Dialing

The live validation in this workspace used a China Mobile SIM with APN `cmnet`. Treat the APN as a SIM/operator property, not a module property.

Common mainland China public APN choices:

| SIM/operator | Common APN | Notes |
| --- | --- | --- |
| China Mobile | `cmnet` | Verified on this board with the current SIM. |
| China Telecom | `ctnet` | Use when testing a normal Telecom data SIM. |
| China Unicom | `3gnet` | Some older/private profiles may use `uninet`; prefer the APN issued with the SIM. |
| IoT/private APN | carrier-issued APN | May require a dedicated APN, private routing, or account credentials. |

Before dialing a different SIM, check the SIM and registration state:

```text
AT+CIMI
AT+CPIN?
AT+COPS?
AT+CEREG?
AT+C5GREG?
AT+CGATT?
AT+CGDCONT?
```

Operator hints from mainland China IMSI MCC/MNC:

- `46000`, `46002`, `46004`, `46007`, `46008`: usually China Mobile.
- `46001`, `46006`, `46009`: usually China Unicom.
- `46003`, `46005`, `46011`: usually China Telecom.

Use the APN that matches the SIM. For the verified China Mobile SIM:

```text
AT^NDISDUP=1,1,"cmnet"
```

Expected success:

```text
OK
^DSAMBR: 1,1073000,104000,"cmnet",6
^NDISSTAT: 1,1,,,"IPV4"
^NDISSTAT: 1,1,,,"IPV6"
```

Then request DHCP on the discovered NCM interface:

```sh
iface=$(basename $(ls -d /sys/class/net/enx* 2>/dev/null | head -n1))
ip link set "$iface" up
udhcpc -i "$iface" -q -n -t 8 -T 5
ip -br addr show "$iface"
ping -I "$iface" -c 5 -W 4 223.5.5.5
ping -I "$iface" -c 5 -W 5 www.baidu.com
```

Observed success:

- Dial result: `^NDISSTAT: 1,1,,,"IPV4"` and `^NDISSTAT: 1,1,,,"IPV6"`.
- DHCP address: `10.72.12.12`.
- DNS servers assigned by China Mobile: `223.87.253.100`, `223.87.253.253`.
- `ping 223.5.5.5`: `5 transmitted, 5 received, 0% packet loss`.
- `ping www.baidu.com`: resolved and passed, also via IPv6.

Conclusion: cellular data and internet access are validated when antennas are connected and `cmnet` is used.

## Voice Call Test

Use a trailing semicolon for voice dialing:

```text
ATD<number>;
AT+CLCC
ATH
```

Observed successful call-signaling sequence:

```text
ATD<number>;
OK
^CCALLSTATE: 1,0,1
^CCALLSTATE: 1,1,1
+CLCC: 1,0,3,0,0,"<number>",161
+CLCC: 1,0,0,0,0,"<number>",161
ATH
OK
```

Interpretation:

- `CLCC` status `3` is outgoing/ringing.
- `CLCC` status `0` is active.
- This validates voice call signaling. Audio path is not validated unless microphone/speaker or PCM audio wiring is present.

## SmartBag Severe-Fall SMS

The production alarm path keeps the local alarm and CloudBase upload, then
independently sends one Chinese SMS through MT5710. It must not place a voice
call.

- Trigger only `signal=FALL_ALARM` with `alarmType=fall_detected`; warning states
  and raw thresholds do not send SMS.
- Use `build_ucs2_sms()`, `AtSession.send_ucs2_sms()`, and
  `Mt5710SmsNotifier` in
  `work/mt5710_5g_cloud_upload/mt5710_5g_cloud_upload.py`.
- Read the destination only from `SMARTBAG_ALERT_PHONE` in the root-only service
  environment file. Accept only `+` followed by digits or digits alone, with 5
  to 20 digits total. A missing or invalid value disables only the SMS notifier
  before opening the serial port; the BMI/local/CloudBase paths must keep running.
  Never log the number, full PDU, or environment value.
- Use PDU mode because text-mode Chinese SMS returned `+CMS ERROR: 305` on this
  module: send `AT+CMGF=0`, then `AT+CMGS=<TPDU byte count>` where the count
  excludes the SMSC-length byte, wait for `>`, and send the hexadecimal PDU plus
  Ctrl-Z. Use DCS `08` with UTF-16BE UCS2 and keep one message to at most 70 UCS2
  code units.
- Treat delivery to the modem as successful only when the response contains both
  `+CMGS:` and final `OK`. Do not automatically retry a timeout or uncertain
  response, because that can send a duplicate alarm.
- An SMS failure is isolated from local and CloudBase alarm delivery.
- The whole device must still run only through `smartbag-5g-upload.service`.
  Never enable the superseded `bmi270-backpack.service`, start a second BMI270
  process, or let another process hold `/dev/ttyUSB1`.

Safe no-send verification uses a fake AT session and a synthetic phone number.
Assert that one final fall produces exactly one `AT+CMGF=0`, one `AT+CMGS`, and
one PDU terminated by Ctrl-Z; non-final events produce none. Verify a fixed PDU
vector, that local and CloudBase sinks each still run once when SMS raises, and
that captured commands contain no `ATD`, `AT+CLCC`, or `ATH`. Tests must never
open `/dev/ttyUSB1` or contain the real destination number. Upload with matching
SHA-256 hashes, run board unit tests, restart the unique service, and check normal
5G/BMI/BLE logs. Do not create a manual fall trigger or issue `AT+CMGS` on real
hardware without explicit authorization for another live SMS.

## GNSS Positioning Status

The MT5710 AT manual documents these commands:

```text
AT^GNSSENABLE
AT^GNSSPOSMODE
AT^GNSSGETNMEA
AT^GNSSGETINFO
```

Current board firmware does not support them:

```text
AT^GNSSENABLE?
COMMAND NOT SUPPORT
AT^GNSSGETINFO?
COMMAND NOT SUPPORT
AT^GNSSGETNMEA=GGA
COMMAND NOT SUPPORT
```

Common alternative command families are also unsupported:

```text
AT+CGPS?
AT+CGNSPWR?
AT+QGPS?
```

Observed `/dev/ttyUSB3` behavior:

- No NMEA output in a 12 second sample.
- No response to `AT`, `AT^GNSSENABLE?`, or `AT^GNSSGETINFO?`.

Conclusion: data and voice work on the current `MT5710_CN V100R001C00B095` Mini PCIe module, but GNSS positioning is not usable with this firmware/module configuration. Treat this as firmware or product-variant limitation unless the vendor provides an enabling NV item or a GNSS-capable firmware build.

## Pitfalls

- If `AT^NDISDUP=1,1` returns `ERROR: NO NETWORK SERVICE`, check antennas and registration before APN. With no antennas the module can see weak NR traces but remain unregistered.
- `C5GREG/CEREG stat=2` means not registered and searching, not a DHCP problem.
- Do not assume `/dev/ttyUSB3` streaming means GNSS works; verify actual NMEA output or `AT^GNSSGETINFO?`.
- Do not apply RM500U commands blindly; this board uses TD Tech MT5710/MT571X.
- `AT^NDISDUP=1,0` may return `ERROR: Invalid` when no active dial session exists; this is not a failure if the next dial succeeds.

## SmartBag Cloud Upload MVP

Use the repository launcher when the goal is to carry all existing SmartBag
telemetry over MT5710 without changing the sensor programs:

```sh
# BOARD-LINUX
cd /root/work/mt5710_5g_cloud_upload
./start_ss928_5g_upload.sh
```

This direct launcher is for maintenance and foreground diagnosis only. Normal
device operation must enter through `smartbag-5g-upload.service`; do not run the
launcher beside the service.

The launcher checks registration/attach, selects the APN, dials NCM, discovers
the `cdc_ncm` interface dynamically, runs DHCP, and verifies that the resolved
CloudBase address routes through NCM before starting DX-GP21-A and BMI270 with
their existing cloud upload configuration. Use `--check-only` to stop after
the network proof.

Latest verified hardware result on 2026-07-21:

- operator: `CHINA TELECOM`
- APN: `ctnet`
- interface: `enx2a5a7626cdfa`
- DHCP IPv4: `10.118.237.237`
- CloudBase route: `124.223.146.214 via 10.0.0.1 dev enx2a5a7626cdfa`
- three complete test telemetry uploads: HTTP 200, with database and
  `smartbag-app-api` readback confirmed

The interface name and assigned addresses are observations, not constants.
When an already connected module returns `ERROR: DUPLICATED` for the same
`AT^NDISDUP=1,1,"<apn>"` request, reuse the active session and continue with
DHCP and route verification; do not treat it as loss of network service.

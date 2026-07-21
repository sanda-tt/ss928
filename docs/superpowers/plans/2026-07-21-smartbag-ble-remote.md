# SmartBag BLE Remote Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox syntax for tracking.

**Goal:** Add a standalone BLE remote page that triggers left and right SmartBag alerts at levels 1-4, and route the homepage debug entry to it.

**Architecture:** Keep pages/monitor/index as the CloudBase posture dashboard. Add pages/remote/index as a native-WeChat BLE client following the Nordic UART Service lifecycle used by pages/tracks/index, but exposing SmartBag commands only. The existing homepage debug card navigates to this remote page.

**Tech Stack:** Native WeChat Mini Program JavaScript/WXML/WXSS, WeChat BLE APIs, Node.js assert tests.

## Global Constraints

- Keep production code under work/ssminiprogram/miniprogram/; use native JavaScript only.
- Do not change pages/monitor/index, board services, BLE UUIDs, hardware configuration, or CloudBase code.
- Use NUS service 6E400001-B5A3-F393-E0A9-E50E24DCCA9E, RX 6E400002-B5A3-F393-E0A9-E50E24DCCA9E, TX 6E400003-B5A3-F393-E0A9-E50E24DCCA9E.
- Send only AL L1 through AL L4, AL R1 through AL R4, and AL CLEAR, each terminated by a newline.
- BLE link validation requires a physical phone; Developer Tools only validates compilation and page rendering.

---

### Task 1: Lock the remote-page contract with failing tests

**Files:**
- Create: work/ssminiprogram/tests/smartbag-remote-page.test.js
- Modify: work/ssminiprogram/tests/home-ui.test.js

**Interfaces:**
- Consumes: UTF-8 page files under miniprogram/pages/home/ and the new miniprogram/pages/remote/.
- Produces: source-contract tests for the remote route, nine commands, NUS UUIDs, and disconnected-write guard.

- [ ] **Step 1: Write the failing route test**

Add a test named home debug entry opens the SmartBag remote page. It must require data-route="/pages/remote/index" in home WXML and reject data-route="/pages/monitor/index".

- [ ] **Step 2: Write the failing remote-page test**

Read miniprogram/pages/remote/index.js, index.wxml, and app.json with fs.readFileSync. Assert the app route, TARGET_NAME = SS928-SmartBag, all three NUS UUIDs, a write guard starting if (!this.data.connected, and the nine WXML command strings AL L1, AL L2, AL L3, AL L4, AL R1, AL R2, AL R3, AL R4, AL CLEAR.

- [ ] **Step 3: Run red verification**

Run from work/ssminiprogram: node tests/home-ui.test.js; node tests/smartbag-remote-page.test.js.

Expected: the homepage route assertion fails and the new test cannot read the absent remote page.

### Task 2: Implement the isolated BLE remote page and homepage route

**Files:**
- Create: work/ssminiprogram/miniprogram/pages/remote/index.js
- Create: work/ssminiprogram/miniprogram/pages/remote/index.wxml
- Create: work/ssminiprogram/miniprogram/pages/remote/index.wxss
- Create: work/ssminiprogram/miniprogram/pages/remote/index.json
- Modify: work/ssminiprogram/miniprogram/app.json
- Modify: work/ssminiprogram/miniprogram/pages/home/index.wxml

**Interfaces:**
- Consumes: WeChat BLE APIs and the NUS UUIDs advertised by SS928-SmartBag.
- Produces: page handlers startScan, connectFromTap, disconnectDevice, sendAlert, stopAlert.

- [ ] **Step 1: Add the route and update the card**

Append pages/remote/index to app.json pages. Change only the existing home debug-card data route to /pages/remote/index.

- [ ] **Step 2: Implement the minimal BLE client**

Define exact constants NUS_SERVICE_UUID, NUS_RX_UUID, NUS_TX_UUID, and TARGET_NAME. Follow pages/tracks/index.js to open the adapter, scan using the NUS filter, retain the exact target name or NUS advertisement, connect only after tap, discover the service and TX/RX characteristics, enable TX notify, and set connected true only after notification setup succeeds.

Implement writeCommand(command): when connected is false or activeDeviceId/serviceId/rxCharacteristicId is missing, set commandResponse to 请先连接设备 and return before any BLE write. Otherwise call wx.writeBLECharacteristicValue with stringToArrayBuffer(command + newline), update commandResponse to 已发送 plus command on success, and update it to 发送失败 plus errMsg or command on failure.

Use Uint8Array text conversion. Buffer fragmented TX notifications until LF or CRLF, then show the latest non-empty board reply. On unload stop discovery, disable notify once initialized, close the BLE connection, and close the adapter if opened.

- [ ] **Step 3: Implement the compact UI**

Create alertGroups data with left AL L1-AL L4 and right AL R1-AL R4. Bind each group button to sendAlert with data-command. Add an AL CLEAR button bound to stopAlert. All command buttons must be disabled while !connected. Show Bluetooth state, discovered-device connection buttons, current device, last command response, and disconnect control.

Set the page title to SmartBag 蓝牙遥控. Use page-local WXSS, rpx sizing, visually distinct left/right groups, an emphasized level-4 control, a disabled state, and no external assets.

- [ ] **Step 4: Run green verification**

Run from work/ssminiprogram: node tests/home-ui.test.js; node tests/smartbag-remote-page.test.js; node --check miniprogram/pages/remote/index.js; node --check miniprogram/pages/home/index.js.

Expected: all tests print ok and all syntax checks exit 0.

- [ ] **Step 5: Commit the green change**

Stage only app.json, the home WXML route, pages/remote, and the two focused test files. Commit message: feat: add SmartBag BLE alert remote.

### Task 3: Validate source scope and a physical BLE path

**Files:**
- Verify: work/ssminiprogram/miniprogram/pages/remote/index.js
- Verify: work/ssminiprogram/tests/smartbag-remote-page.test.js

**Interfaces:**
- Consumes: completed Task 2 implementation.
- Produces: source-level proof and an explicit physical-device validation result.

- [ ] **Step 1: Run the focused test and syntax set**

Run from work/ssminiprogram: node tests/home-ui.test.js; node tests/smartbag-remote-page.test.js; node --check miniprogram/pages/remote/index.js; node --check miniprogram/pages/monitor/index.js.

Expected: all commands exit 0; the unchanged monitor page remains syntactically valid.

- [ ] **Step 2: Inspect scope**

Run from repository root: git diff --check HEAD^ HEAD; git show --stat --oneline HEAD.

Expected: no whitespace error; only the homepage route, app route, remote page, and focused tests are included.

- [ ] **Step 3: Do physical validation**

In WeChat Developer Tools preview to a phone. Open the homepage bottom debug entry, scan for SS928-SmartBag, connect, issue every L1-L4 and R1-R4 command once, observe a board reply, then issue AL CLEAR and observe acknowledgement. If hardware or BLE is unavailable, record it as unverified rather than a pass.

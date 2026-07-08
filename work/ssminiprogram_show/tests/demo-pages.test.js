const assert = require("assert");
const fs = require("fs");
const path = require("path");

const root = path.join(__dirname, "..", "miniprogram");
const readPage = (page, ext) => fs.readFileSync(path.join(root, "pages", page, "index." + ext), "utf8");

const tracksJs = readPage("tracks", "js");
const tracksWxml = readPage("tracks", "wxml");
const analysisWxml = readPage("index", "wxml");
const analysisJson = readPage("index", "json");
const monitorWxml = readPage("monitor", "wxml");

const tests = [];

const test = (name, fn) => {
  tests.push({ name, fn });
};

test("track demo page does not expose BLE scanning controls", () => {
  assert.strictEqual(tracksWxml.indexOf("startScan"), -1);
  assert.strictEqual(tracksWxml.indexOf("候选设备"), -1);
  assert.strictEqual(tracksWxml.indexOf("扫描"), -1);
  assert.ok(tracksJs.indexOf("createDemoTrackPoints") >= 0);
  assert.ok(tracksJs.indexOf("setInterval") >= 0);
});

test("track demo page only keeps recent position records below the summary", () => {
  assert.strictEqual(tracksWxml.indexOf("板端本地轨迹"), -1);
  assert.strictEqual(tracksWxml.indexOf("track-row"), -1);
  assert.strictEqual(tracksJs.indexOf("trackItems"), -1);
  assert.ok(tracksWxml.indexOf("最近位置记录") >= 0);
  assert.ok(tracksWxml.indexOf("recent-row") >= 0);
});

test("analysis page is posture analysis instead of capture calibration", () => {
  assert.ok(analysisWxml.indexOf("姿态分析") >= 0);
  assert.ok(analysisWxml.indexOf("今日姿态分析") >= 0);
  assert.ok(analysisWxml.indexOf("最近十条数据记录") >= 0);
  assert.strictEqual(analysisWxml.indexOf("采集校准"), -1);
  assert.strictEqual(analysisWxml.indexOf("扫描设备"), -1);
  assert.ok(analysisJson.indexOf("姿态分析") >= 0);
});

test("live posture page hides debug commands and shows reminder copy", () => {
  assert.ok(monitorWxml.indexOf("当前姿态") >= 0);
  assert.ok(monitorWxml.indexOf("已触发震动和语音提醒") >= 0);
  assert.strictEqual(monitorWxml.indexOf("报警调试"), -1);
  assert.strictEqual(monitorWxml.indexOf("ZERO"), -1);
  assert.strictEqual(monitorWxml.indexOf("AL CLEAR"), -1);
  assert.strictEqual(monitorWxml.indexOf("扫描设备"), -1);
});

test("posture demo pages hide concrete angle details", () => {
  assert.strictEqual(analysisWxml.indexOf("record-row__pitch"), -1);
  assert.strictEqual(analysisWxml.indexOf("{{item.pitch}}"), -1);
  assert.strictEqual(monitorWxml.indexOf("Pitch"), -1);
  assert.strictEqual(monitorWxml.indexOf("Roll"), -1);
  assert.strictEqual(monitorWxml.indexOf("角速度"), -1);
  assert.strictEqual(monitorWxml.indexOf("{{pitch}}"), -1);
  assert.strictEqual(monitorWxml.indexOf("{{roll}}"), -1);
  assert.strictEqual(monitorWxml.indexOf("{{gyroDps}}"), -1);
});

let failed = 0;

for (let i = 0; i < tests.length; i += 1) {
  const item = tests[i];
  try {
    item.fn();
    console.log("ok - " + item.name);
  } catch (err) {
    failed += 1;
    console.error("not ok - " + item.name);
    console.error(err && err.stack ? err.stack : err);
  }
}

if (failed) {
  process.exit(1);
}

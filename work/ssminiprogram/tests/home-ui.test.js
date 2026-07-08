const assert = require("assert");
const fs = require("fs");
const path = require("path");

const root = path.join(__dirname, "..", "miniprogram");
const homeJs = fs.readFileSync(path.join(root, "pages", "home", "index.js"), "utf8");
const homeWxml = fs.readFileSync(path.join(root, "pages", "home", "index.wxml"), "utf8");
const homeWxss = fs.readFileSync(path.join(root, "pages", "home", "index.wxss"), "utf8");
const placeholderJs = fs.readFileSync(path.join(root, "pages", "placeholder", "index.js"), "utf8");

const tests = [];

const test = (name, fn) => {
  tests.push({ name, fn });
};

const getFeatureTitles = () => {
  const start = homeJs.indexOf("const FEATURE_ENTRIES");
  const end = homeJs.indexOf("];", start);
  assert.ok(start >= 0 && end > start, "FEATURE_ENTRIES block should exist");
  return Array.from(homeJs.slice(start, end).matchAll(/^\s+title: "([^"]+)"/gm)).map((match) => match[1]);
};

test("home feature entries move settings to the fourth option", () => {
  assert.deepStrictEqual(getFeatureTitles(), [
    "危险报警历史",
    "儿童安全轨迹跟踪",
    "姿态分析和记录",
    "设置"
  ]);
  assert.ok(homeJs.indexOf('subtitle: "参数设置"') >= 0);
  assert.ok(homeJs.indexOf("板子健康数据") < 0);
});

test("home monitor card keeps only the line and monitor circular statuses", () => {
  assert.ok(homeWxml.indexOf('<view class="backpack__shield">安</view>') >= 0);
  assert.ok(homeWxml.indexOf("status-cell__dot") >= 0);
  assert.ok(homeWxml.indexOf(">线<") >= 0);
  assert.ok(homeWxml.indexOf(">监<") >= 0);
  assert.ok(homeWxml.indexOf("{{connectionStatusText}}") >= 0);
  assert.ok(homeWxml.indexOf("{{monitorStatusText}}") >= 0);
  assert.ok(homeWxss.indexOf("status-cell__dot") >= 0);
  assert.ok(homeWxml.indexOf("status-line") < 0);
  assert.ok(homeWxml.indexOf(">连<") < 0);
  assert.ok(homeWxml.indexOf(">电<") < 0);
  assert.ok(homeWxml.indexOf(">盾<") < 0);
});

test("home demo status variables default to online monitoring", () => {
  assert.ok(homeJs.indexOf('connectionStatusText: "设备在线"') >= 0);
  assert.ok(homeJs.indexOf('monitorStatusText: "实时监控"') >= 0);
});

test("settings placeholder text matches the home entry", () => {
  assert.ok(placeholderJs.indexOf("settings") >= 0);
  assert.ok(placeholderJs.indexOf("设置") >= 0);
  assert.ok(placeholderJs.indexOf("参数设置") >= 0);
  assert.ok(placeholderJs.indexOf("板子健康数据") < 0);
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

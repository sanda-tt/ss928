const assert = require("assert");
const demo = require("../miniprogram/utils/demo-data");

const tests = [];

const test = (name, fn) => {
  tests.push({ name, fn });
};

test("demo track starts with 235 People Park points in reasonable ranges", () => {
  const points = demo.createDemoTrackPoints();
  assert.strictEqual(points.length, 235);

  for (let i = 0; i < points.length; i += 1) {
    const point = points[i];
    assert.strictEqual(point.cs, "gcj02");
    assert.ok(point.lat > 30.655 && point.lat < 30.659, "lat out of range: " + point.lat);
    assert.ok(point.lon > 104.055 && point.lon < 104.061, "lon out of range: " + point.lon);
    assert.ok(point.acc >= 4.2 && point.acc <= 8.8, "acc out of range: " + point.acc);
    assert.ok(point.spd >= 0.6 && point.spd <= 1.4, "speed out of range: " + point.spd);
  }
});

test("demo track follows road-like People Park waypoints with visible turns", () => {
  const points = demo.createDemoTrackPoints();
  const anchors = demo.PEOPLE_PARK_ROUTE_ANCHORS;
  assert.ok(Array.isArray(anchors));
  assert.ok(anchors.length >= 8);

  let anchorsTouched = 0;
  for (let i = 0; i < anchors.length; i += 1) {
    const anchor = anchors[i];
    const close = points.some((point) => (
      Math.abs(point.lat - anchor.lat) < 0.00008 &&
      Math.abs(point.lon - anchor.lon) < 0.00008
    ));
    if (close) {
      anchorsTouched += 1;
    }
  }
  assert.ok(anchorsTouched >= anchors.length - 1, "anchors touched: " + anchorsTouched);

  let turnCount = 0;
  for (let i = 2; i < points.length; i += 1) {
    const headingA = Math.atan2(points[i - 1].lat - points[i - 2].lat, points[i - 1].lon - points[i - 2].lon);
    const headingB = Math.atan2(points[i].lat - points[i - 1].lat, points[i].lon - points[i - 1].lon);
    let delta = Math.abs(headingB - headingA);
    if (delta > Math.PI) {
      delta = Math.PI * 2 - delta;
    }
    if (delta > 0.35) {
      turnCount += 1;
    }
  }
  assert.ok(turnCount >= 8, "turnCount: " + turnCount);
});

test("demo track point generator appends the next continuous point", () => {
  const first = demo.createDemoTrackPoints(235);
  const next = demo.createNextTrackPoint(235);
  const last = first[first.length - 1];
  assert.ok(Math.abs(next.lat - last.lat) < 0.00012);
  assert.ok(Math.abs(next.lon - last.lon) < 0.00012);
  assert.ok(next.t > last.t);
});

test("demo local track list exposes six recent records", () => {
  const rows = demo.createDemoTrackItems();
  assert.strictEqual(rows.length, 6);
  assert.ok(rows[0].title.indexOf("人民公园") >= 0);
  assert.ok(rows[0].n >= 235);
});

test("posture demo timeline follows the fixed video clock", () => {
  const start = demo.getPostureDemoState(0);
  assert.strictEqual(start.clockText, "20:17:12");
  assert.strictEqual(start.postureKey, "good");
  assert.strictEqual(start.showReminder, false);

  const boundary = demo.getPostureDemoState(15000);
  assert.strictEqual(boundary.clockText, "20:17:27");
  assert.strictEqual(boundary.postureKey, "bad");
  assert.strictEqual(boundary.showReminder, false);

  const reminder = demo.getPostureDemoState(40000);
  assert.strictEqual(reminder.clockText, "20:17:52");
  assert.strictEqual(reminder.postureKey, "bad");
  assert.strictEqual(reminder.showReminder, true);

  const final = demo.getPostureDemoState(45000);
  assert.strictEqual(final.clockText, "20:17:57");
  assert.strictEqual(final.postureKey, "bad");
  assert.strictEqual(final.showReminder, true);
});

test("posture analysis data is fixed for video demo", () => {
  assert.strictEqual(demo.POSTURE_STATS.goodPercent, 79);
  assert.strictEqual(demo.POSTURE_STATS.badPercent, 21);
  assert.strictEqual(demo.POSTURE_RECORDS.length, 10);
  for (let i = 0; i < demo.POSTURE_RECORDS.length; i += 1) {
    assert.strictEqual(Object.prototype.hasOwnProperty.call(demo.POSTURE_RECORDS[i], "pitch"), false);
    assert.strictEqual(JSON.stringify(demo.POSTURE_RECORDS[i]).indexOf("°"), -1);
  }
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

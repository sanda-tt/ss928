const assert = require("assert");

const telemetry = require("../cloudfunctions/smartbag-device-ingest/lib/telemetry-core");
const appApi = require("../cloudfunctions/smartbag-app-api/lib/app-api-core");

const tests = [];
const test = (name, fn) => tests.push({ name, fn });
const UPLOAD_TOKEN = "competition-upload-token";

const makePayload = () => ({
  status: {
    location: { latitude: 30.65736, longitude: 104.0832 },
    attitude: { rollDeg: 1.2, pitchDeg: -3.4, yawDeg: 92.1 },
    risk: { level: 1, type: "rear_vehicle", direction: "left_rear" },
    battery: { percent: 82 },
    network: { type: "5g-redcap", rssiDbm: -81 },
    firmwareVersion: "sim-v1"
  },
  trackPoints: [{
    reportedAt: 1784160000000,
    location: { latitude: 30.65736, longitude: 104.0832 },
    speed: 1.2,
    heading: 92
  }],
  alarms: [{
    reportedAt: 1784160000000,
    alarmType: "rear_vehicle",
    riskLevel: 2,
    direction: "left_rear",
    message: "vehicle approaching",
    location: { latitude: 30.65736, longitude: 104.0832 }
  }]
});

const makeStore = () => {
  const state = { status: null, tracks: [], alarms: [] };
  return {
    state,
    async setStatus(document) { state.status = document; },
    async insertTrackPoint(document) { state.tracks.push(document); },
    async insertAlarm(document) { state.alarms.push(document); }
  };
};

test("accepts the upload token and writes all records as bag001", async () => {
  const store = makeStore();
  const processor = telemetry.createTelemetryProcessor({ store, uploadToken: UPLOAD_TOKEN, now: () => "2026-07-17T00:00:00.000Z" });
  const result = await processor.process({
    headers: { "x-upload-token": UPLOAD_TOKEN },
    rawBody: Buffer.from(JSON.stringify(makePayload()))
  });
  assert.deepStrictEqual(result, { success: true });
  assert.strictEqual(store.state.status._id, "bag001");
  assert.strictEqual(store.state.status.deviceId, "bag001");
  assert.strictEqual(store.state.tracks[0].deviceId, "bag001");
  assert.strictEqual(store.state.alarms[0].deviceId, "bag001");
});

test("rejects an invalid upload token", async () => {
  const processor = telemetry.createTelemetryProcessor({ store: makeStore(), uploadToken: UPLOAD_TOKEN });
  const result = await processor.process({
    headers: { "x-upload-token": "wrong-token" },
    rawBody: Buffer.from(JSON.stringify(makePayload()))
  });
  assert.strictEqual(result.success, false);
  assert.strictEqual(result.error.code, "INVALID_UPLOAD_TOKEN");
});

test("returns latest status without user binding", async () => {
  const api = appApi.createAppApi({
    repository: { async getStatus(deviceId) { return { deviceId, battery: { percent: 82 } }; } }
  });
  const result = await api.handle({ action: "getLatestStatus" });
  assert.deepStrictEqual(result, { ok: true, data: { status: { deviceId: "bag001", battery: { percent: 82 } } } });
});

test("limits history reads to 100 records", async () => {
  const api = appApi.createAppApi({
    repository: { async listTrackPoints(deviceId, limit) { return Array.from({ length: limit }, (_, index) => ({ deviceId, sequence: index })); } }
  });
  const result = await api.handle({ action: "getTrackPoints" });
  assert.strictEqual(result.ok, true);
  assert.strictEqual(result.data.items.length, 100);
  assert.strictEqual(result.data.items[0].deviceId, "bag001");
});

const run = async () => {
  let failed = 0;
  for (let index = 0; index < tests.length; index += 1) {
    const item = tests[index];
    try {
      await item.fn();
      console.log("ok - " + item.name);
    } catch (error) {
      failed += 1;
      console.error("not ok - " + item.name);
      console.error(error && error.stack ? error.stack : error);
    }
  }
  if (failed) process.exit(1);
};

run();

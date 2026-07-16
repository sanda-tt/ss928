const assert = require("assert");
const crypto = require("crypto");

const telemetry = require("../cloudfunctions/smartbag-device-ingest/lib/telemetry-core");
const appApi = require("../cloudfunctions/smartbag-app-api/lib/app-api-core");

const tests = [];

const test = (name, fn) => tests.push({ name, fn });

const DEVICE_ID = "bag001";
const DEVICE_SECRET = "test-device-secret-not-for-production";
const NOW_SECONDS = 1784160000;

const makePayload = (overrides) => Object.assign({
  version: 1,
  deviceId: DEVICE_ID,
  requestId: "req-000001",
  reportedAt: NOW_SECONDS * 1000,
  status: {
    location: { latitude: 30.65736, longitude: 104.0832, accuracyM: 6.5, valid: true },
    attitude: { rollDeg: 1.2, pitchDeg: -3.4, yawDeg: 92.1 },
    risk: { level: 1, type: "rear_vehicle", direction: "left_rear" },
    battery: { percent: 82 },
    network: { type: "5g-redcap", rssiDbm: -81 },
    firmwareVersion: "v1.0.0"
  },
  trackPoints: [{
    reportedAt: NOW_SECONDS * 1000,
    location: { latitude: 30.65736, longitude: 104.0832, accuracyM: 6.5, valid: true },
    speed: 1.2,
    heading: 92
  }],
  alarms: [{
    reportedAt: NOW_SECONDS * 1000,
    alarmType: "rear_vehicle",
    riskLevel: 2,
    direction: "left_rear",
    message: "vehicle approaching",
    location: { latitude: 30.65736, longitude: 104.0832, accuracyM: 6.5, valid: true }
  }]
}, overrides || {});

const signedRequest = (payload, options) => {
  const body = Buffer.from(JSON.stringify(payload));
  const headers = {
    "content-type": "application/json",
    "x-device-id": DEVICE_ID,
    "x-timestamp": String(NOW_SECONDS),
    "x-nonce": "nonce-000000000001"
  };
  headers["x-signature"] = telemetry.signRequest(DEVICE_SECRET, headers, body);
  return Object.assign({ headers, rawBody: body }, options || {});
};

const makeStore = () => {
  const device = { _id: DEVICE_ID, enabled: true, name: "Test Bag" };
  const state = { device, status: null, tracks: [], alarms: [], logs: [] };
  return {
    state,
    async getDevice(deviceId) { return deviceId === DEVICE_ID ? state.device : null; },
    async updateDevice(deviceId, patch) {
      assert.strictEqual(deviceId, DEVICE_ID);
      Object.assign(state.device, patch);
    },
    async setStatus(document) { state.status = document; },
    async insertTrackPoint(document) {
      if (state.tracks.some((item) => item.requestId === document.requestId)) {
        const error = new Error("duplicate key");
        error.code = "DUPLICATE_KEY";
        throw error;
      }
      state.tracks.push(document);
    },
    async insertAlarm(document) {
      if (state.alarms.some((item) => item.requestId === document.requestId)) {
        const error = new Error("duplicate key");
        error.code = "DUPLICATE_KEY";
        throw error;
      }
      state.alarms.push(document);
    },
    log(entry) { state.logs.push(entry); }
  };
};

test("accepts the exact HMAC-SHA256 signing protocol", () => {
  const request = signedRequest(makePayload());
  const result = telemetry.verifyRequestSignature({
    secret: DEVICE_SECRET,
    headers: request.headers,
    rawBody: request.rawBody
  });
  assert.strictEqual(result.ok, true);
});

test("rejects an incorrect signature without exposing it", () => {
  const request = signedRequest(makePayload());
  request.headers["x-signature"] = "0".repeat(64);
  assert.throws(() => telemetry.verifyRequestSignature({ secret: DEVICE_SECRET, headers: request.headers, rawBody: request.rawBody }), (error) => {
    assert.strictEqual(error.code, "INVALID_SIGNATURE");
    assert.strictEqual(String(error.message).indexOf("0".repeat(64)), -1);
    return true;
  });
});

test("rejects an expired timestamp", () => {
  const request = signedRequest(makePayload(), { headers: Object.assign({}, signedRequest(makePayload()).headers, { "x-timestamp": String(NOW_SECONDS - 301) }) });
  request.headers["x-signature"] = telemetry.signRequest(DEVICE_SECRET, request.headers, request.rawBody);
  assert.throws(() => telemetry.validateSignedRequest({
    secret: DEVICE_SECRET,
    headers: request.headers,
    rawBody: request.rawBody,
    nowSeconds: NOW_SECONDS,
    maxClockSkewSeconds: 300
  }), /TIMESTAMP_EXPIRED/);
});

test("rejects invalid latitude and longitude", () => {
  const payload = makePayload({ status: Object.assign({}, makePayload().status, { location: { latitude: 91, longitude: 104 } }) });
  assert.throws(() => telemetry.validateTelemetry(payload), /INVALID_LOCATION/);
});

test("rejects invalid attitude values", () => {
  const payload = makePayload({ status: Object.assign({}, makePayload().status, { attitude: { rollDeg: 999, pitchDeg: 0, yawDeg: 0 } }) });
  assert.throws(() => telemetry.validateTelemetry(payload), /INVALID_ATTITUDE/);
});

test("rejects a repeated nonce and keeps request IDs separate per history type", async () => {
  const store = makeStore();
  const processor = telemetry.createTelemetryProcessor({ store, resolveSecret: () => DEVICE_SECRET, nowSeconds: () => NOW_SECONDS });
  const request = signedRequest(makePayload());
  const first = await processor.process(request);
  assert.strictEqual(first.ok, true);
  assert.strictEqual(store.state.tracks[0].requestId, "req-000001:track:0");
  assert.strictEqual(store.state.alarms[0].requestId, "req-000001:alarm:0");
  const second = await processor.process(request);
  assert.strictEqual(second.ok, false);
  assert.strictEqual(second.error.code, "REPLAYED_NONCE");
});

test("treats duplicate track and alarm request IDs as idempotent success", async () => {
  const store = makeStore();
  const processor = telemetry.createTelemetryProcessor({ store, resolveSecret: () => DEVICE_SECRET, nowSeconds: () => NOW_SECONDS + 10 });
  const first = signedRequest(makePayload(), { headers: Object.assign({}, signedRequest(makePayload()).headers, { "x-timestamp": String(NOW_SECONDS + 1), "x-nonce": "nonce-000000000002" }) });
  first.headers["x-signature"] = telemetry.signRequest(DEVICE_SECRET, first.headers, first.rawBody);
  await processor.process(first);
  store.state.device.lastNonce = "another-nonce";
  const second = signedRequest(makePayload(), { headers: Object.assign({}, first.headers, { "x-timestamp": String(NOW_SECONDS + 2), "x-nonce": "nonce-000000000003" }) });
  second.headers["x-signature"] = telemetry.signRequest(DEVICE_SECRET, second.headers, second.rawBody);
  const result = await processor.process(second);
  assert.strictEqual(result.ok, true);
  assert.strictEqual(result.data.idempotent, true);
});

test("does not leak the device secret to operational logs", async () => {
  const store = makeStore();
  const processor = telemetry.createTelemetryProcessor({ store, resolveSecret: () => DEVICE_SECRET, nowSeconds: () => NOW_SECONDS });
  const request = signedRequest(makePayload());
  await processor.process(request);
  assert.strictEqual(JSON.stringify(store.state.logs).indexOf(DEVICE_SECRET), -1);
  assert.strictEqual(JSON.stringify(store.state.logs).indexOf(request.headers["x-signature"]), -1);
});

test("denies device data reads for an unbound OPENID", async () => {
  const api = appApi.createAppApi({
    getOpenId: () => "openid-not-bound",
    repository: {
      async findBinding() { return null; }
    }
  });
  const result = await api.handle({ action: "getLatestStatus", deviceId: DEVICE_ID });
  assert.strictEqual(result.ok, false);
  assert.strictEqual(result.error.code, "DEVICE_NOT_BOUND");
});

test("rejects pageSize above the configured maximum", async () => {
  const api = appApi.createAppApi({
    getOpenId: () => "openid-owner",
    repository: {
      async findBinding() { return { deviceId: DEVICE_ID, openid: "openid-owner" }; }
    }
  });
  const result = await api.handle({ action: "getTrackPoints", deviceId: DEVICE_ID, pageSize: 101 });
  assert.strictEqual(result.ok, false);
  assert.strictEqual(result.error.code, "INVALID_PAGE_SIZE");
});

const run = async () => {
  let failed = 0;
  for (let i = 0; i < tests.length; i += 1) {
    const item = tests[i];
    try {
      await item.fn();
      console.log("ok - " + item.name);
    } catch (error) {
      failed += 1;
      console.error("not ok - " + item.name);
      console.error(error && error.stack ? error.stack : error);
    }
  }
  if (failed) {
    process.exit(1);
  }
};

run();

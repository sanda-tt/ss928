const http = require("http");
const cloud = require("wx-server-sdk");
const telemetry = require("./lib/telemetry-core");

cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV });
const db = cloud.database();
const MAX_BODY_BYTES = 64 * 1024;

const corsHeaders = {
  "Access-Control-Allow-Origin": "*",
  "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
  "Access-Control-Allow-Headers": "Content-Type, X-Device-Id, X-Timestamp, X-Nonce, X-Signature"
};

const sendJson = (res, statusCode, body) => {
  res.writeHead(statusCode, Object.assign({ "Content-Type": "application/json; charset=utf-8" }, corsHeaders));
  res.end(JSON.stringify(body));
};

const readRawBody = (req) => new Promise((resolve, reject) => {
  const chunks = [];
  let size = 0;
  let rejected = false;
  req.on("data", (chunk) => {
    size += chunk.length;
    if (size > MAX_BODY_BYTES && !rejected) {
      rejected = true;
      reject(new telemetry.TelemetryError("PAYLOAD_TOO_LARGE", "PAYLOAD_TOO_LARGE", 413));
      req.resume();
      return;
    }
    if (!rejected) {
      chunks.push(chunk);
    }
  });
  req.on("end", () => {
    if (!rejected) {
      resolve(Buffer.concat(chunks));
    }
  });
  req.on("error", reject);
});

const loadDeviceSecrets = () => {
  const raw = process.env.SMARTBAG_DEVICE_SECRETS_JSON;
  if (!raw) {
    return {};
  }
  try {
    const parsed = JSON.parse(raw);
    return parsed && typeof parsed === "object" ? parsed : {};
  } catch (error) {
    return {};
  }
};

const createCloudStore = (database) => ({
  async getDevice(deviceId) {
    try {
      const result = await database.collection("devices").doc(deviceId).get();
      return result && result.data ? result.data : null;
    } catch (error) {
      return null;
    }
  },
  async updateDevice(deviceId, patch) {
    await database.collection("devices").doc(deviceId).update({ data: patch });
  },
  async setStatus(document) {
    await database.collection("device_status").doc(document._id).set({ data: document });
  },
  async insertTrackPoint(document) {
    await database.collection("track_points").add({ data: document });
  },
  async insertAlarm(document) {
    await database.collection("alarm_history").add({ data: document });
  },
  log(entry) {
    console.log(JSON.stringify(entry));
  }
});

const createServer = ({ processor }) => http.createServer(async (req, res) => {
  if (req.method === "OPTIONS") {
    res.writeHead(204, corsHeaders);
    res.end();
    return;
  }
  const url = new URL(req.url || "/", "http://127.0.0.1");
  if (req.method === "GET" && url.pathname === "/health") {
    sendJson(res, 200, { ok: true, data: { service: "smartbag-device-ingest" } });
    return;
  }
  if (url.pathname !== "/v1/device/telemetry") {
    sendJson(res, 404, { ok: false, error: { code: "NOT_FOUND", message: "NOT_FOUND" } });
    return;
  }
  if (req.method !== "POST") {
    sendJson(res, 405, { ok: false, error: { code: "METHOD_NOT_ALLOWED", message: "METHOD_NOT_ALLOWED" } });
    return;
  }
  try {
    const rawBody = await readRawBody(req);
    const result = await processor.process({ headers: req.headers, rawBody });
    sendJson(res, result.ok ? 202 : result.statusCode || 400, result.ok ? result : { ok: false, error: result.error });
  } catch (error) {
    const statusCode = error && error.statusCode ? error.statusCode : 500;
    const code = error && error.code ? error.code : "INTERNAL_ERROR";
    sendJson(res, statusCode, { ok: false, error: { code, message: code } });
  }
});

const secrets = loadDeviceSecrets();
const processor = telemetry.createTelemetryProcessor({
  store: createCloudStore(db),
  resolveSecret: (deviceId) => secrets[deviceId] || ""
});

if (require.main === module) {
  createServer({ processor }).listen(9000);
}

module.exports = { createCloudStore, createServer, readRawBody };

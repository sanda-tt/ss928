const crypto = require("crypto");

const MAX_CLOCK_SKEW_SECONDS = 300;
const MAX_TRACK_POINTS = 100;
const MAX_ALARMS = 100;
const DEVICE_ID_PATTERN = /^[A-Za-z0-9_-]{3,64}$/;
const REQUEST_ID_PATTERN = /^[A-Za-z0-9._:-]{8,128}$/;
const NONCE_PATTERN = /^[A-Za-z0-9._~-]{16,128}$/;

class TelemetryError extends Error {
  constructor(code, message, statusCode) {
    super(message || code);
    this.code = code;
    this.statusCode = statusCode || 400;
  }
}

const fail = (code, message, statusCode) => {
  throw new TelemetryError(code, message, statusCode);
};

const getHeader = (headers, name) => {
  const wanted = String(name).toLowerCase();
  const keys = Object.keys(headers || {});
  for (let index = 0; index < keys.length; index += 1) {
    if (keys[index].toLowerCase() === wanted) {
      const value = headers[keys[index]];
      return Array.isArray(value) ? value[0] : value;
    }
  }
  return "";
};

const sha256Hex = (value) => crypto.createHash("sha256").update(value).digest("hex");

const signingString = (headers, rawBody) => {
  const deviceId = getHeader(headers, "x-device-id");
  const timestamp = getHeader(headers, "x-timestamp");
  const nonce = getHeader(headers, "x-nonce");
  return deviceId + "\n" + timestamp + "\n" + nonce + "\n" + sha256Hex(rawBody);
};

const signRequest = (secret, headers, rawBody) => crypto
  .createHmac("sha256", secret)
  .update(signingString(headers, rawBody))
  .digest("hex");

const safeEqualHex = (left, right) => {
  if (!/^[a-f0-9]{64}$/i.test(String(left || "")) || !/^[a-f0-9]{64}$/i.test(String(right || ""))) {
    return false;
  }
  return crypto.timingSafeEqual(Buffer.from(String(left), "hex"), Buffer.from(String(right), "hex"));
};

const requireFinite = (value, code) => {
  if (typeof value !== "number" || !Number.isFinite(value)) {
    fail(code, code);
  }
  return value;
};

const requireText = (value, code, maxLength) => {
  if (typeof value !== "string" || !value.trim() || value.length > maxLength) {
    fail(code, code);
  }
  return value;
};

const validateLocation = (location) => {
  if (!location || typeof location !== "object") {
    fail("INVALID_LOCATION", "INVALID_LOCATION");
  }
  const latitude = requireFinite(location.latitude, "INVALID_LOCATION");
  const longitude = requireFinite(location.longitude, "INVALID_LOCATION");
  if (latitude < -90 || latitude > 90 || longitude < -180 || longitude > 180) {
    fail("INVALID_LOCATION", "INVALID_LOCATION");
  }
  if (typeof location.accuracyM !== "undefined") {
    const accuracy = requireFinite(location.accuracyM, "INVALID_LOCATION");
    if (accuracy < 0 || accuracy > 100000) {
      fail("INVALID_LOCATION", "INVALID_LOCATION");
    }
  }
  if (typeof location.valid !== "undefined" && typeof location.valid !== "boolean") {
    fail("INVALID_LOCATION", "INVALID_LOCATION");
  }
  return Object.assign({}, location);
};

const validateAttitude = (attitude) => {
  if (!attitude || typeof attitude !== "object") {
    fail("INVALID_ATTITUDE", "INVALID_ATTITUDE");
  }
  const rollDeg = requireFinite(attitude.rollDeg, "INVALID_ATTITUDE");
  const pitchDeg = requireFinite(attitude.pitchDeg, "INVALID_ATTITUDE");
  const yawDeg = requireFinite(attitude.yawDeg, "INVALID_ATTITUDE");
  if (rollDeg < -180 || rollDeg > 180 || pitchDeg < -180 || pitchDeg > 180 || yawDeg < -360 || yawDeg > 360) {
    fail("INVALID_ATTITUDE", "INVALID_ATTITUDE");
  }
  return { rollDeg, pitchDeg, yawDeg };
};

const validateStatus = (status) => {
  if (!status || typeof status !== "object") {
    fail("INVALID_STATUS", "INVALID_STATUS");
  }
  const risk = status.risk;
  const battery = status.battery;
  const network = status.network;
  if (!risk || !battery || !network) {
    fail("INVALID_STATUS", "INVALID_STATUS");
  }
  if (!Number.isInteger(risk.level) || risk.level < 0 || risk.level > 3) {
    fail("INVALID_RISK", "INVALID_RISK");
  }
  const batteryPercent = requireFinite(battery.percent, "INVALID_BATTERY");
  if (batteryPercent < 0 || batteryPercent > 100) {
    fail("INVALID_BATTERY", "INVALID_BATTERY");
  }
  return {
    location: validateLocation(status.location),
    attitude: validateAttitude(status.attitude),
    risk: {
      level: risk.level,
      type: requireText(risk.type, "INVALID_RISK", 64),
      direction: requireText(risk.direction, "INVALID_RISK", 64)
    },
    battery: { percent: batteryPercent },
    network: Object.assign({}, network),
    firmwareVersion: requireText(status.firmwareVersion, "INVALID_FIRMWARE_VERSION", 64)
  };
};

const validateTelemetry = (payload) => {
  if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
    fail("INVALID_JSON", "INVALID_JSON");
  }
  if (payload.version !== 1) {
    fail("UNSUPPORTED_VERSION", "UNSUPPORTED_VERSION");
  }
  const deviceId = requireText(payload.deviceId, "INVALID_DEVICE_ID", 64);
  const requestId = requireText(payload.requestId, "INVALID_REQUEST_ID", 128);
  if (!DEVICE_ID_PATTERN.test(deviceId)) {
    fail("INVALID_DEVICE_ID", "INVALID_DEVICE_ID");
  }
  if (!REQUEST_ID_PATTERN.test(requestId)) {
    fail("INVALID_REQUEST_ID", "INVALID_REQUEST_ID");
  }
  const reportedAt = requireFinite(payload.reportedAt, "INVALID_REPORTED_AT");
  if (!Number.isInteger(reportedAt) || reportedAt <= 0) {
    fail("INVALID_REPORTED_AT", "INVALID_REPORTED_AT");
  }
  const trackPoints = typeof payload.trackPoints === "undefined" ? [] : payload.trackPoints;
  const alarms = typeof payload.alarms === "undefined" ? [] : payload.alarms;
  if (!Array.isArray(trackPoints) || trackPoints.length > MAX_TRACK_POINTS || !Array.isArray(alarms) || alarms.length > MAX_ALARMS) {
    fail("INVALID_HISTORY_BATCH", "INVALID_HISTORY_BATCH");
  }
  const normalizedTracks = trackPoints.map((item, index) => {
    if (!item || typeof item !== "object") {
      fail("INVALID_TRACK_POINT", "INVALID_TRACK_POINT");
    }
    const itemReportedAt = requireFinite(item.reportedAt, "INVALID_TRACK_POINT");
    return {
      requestId: requestId + ":track:" + index,
      deviceId,
      reportedAt: itemReportedAt,
      location: validateLocation(item.location),
      speed: requireFinite(item.speed, "INVALID_TRACK_POINT"),
      heading: requireFinite(item.heading, "INVALID_TRACK_POINT")
    };
  });
  const normalizedAlarms = alarms.map((item, index) => {
    if (!item || typeof item !== "object" || !Number.isInteger(item.riskLevel) || item.riskLevel < 0 || item.riskLevel > 3) {
      fail("INVALID_ALARM", "INVALID_ALARM");
    }
    return {
      requestId: requestId + ":alarm:" + index,
      deviceId,
      reportedAt: requireFinite(item.reportedAt, "INVALID_ALARM"),
      alarmType: requireText(item.alarmType, "INVALID_ALARM", 64),
      riskLevel: item.riskLevel,
      direction: requireText(item.direction, "INVALID_ALARM", 64),
      message: requireText(item.message, "INVALID_ALARM", 256),
      location: validateLocation(item.location)
    };
  });
  return {
    version: 1,
    deviceId,
    requestId,
    reportedAt,
    status: validateStatus(payload.status),
    trackPoints: normalizedTracks,
    alarms: normalizedAlarms
  };
};

const verifyRequestSignature = ({ secret, headers, rawBody }) => {
  if (typeof secret !== "string" || !secret) {
    fail("DEVICE_SECRET_UNAVAILABLE", "DEVICE_SECRET_UNAVAILABLE", 500);
  }
  const signature = getHeader(headers, "x-signature");
  const expected = signRequest(secret, headers, rawBody);
  if (!safeEqualHex(expected, signature)) {
    fail("INVALID_SIGNATURE", "INVALID_SIGNATURE", 401);
  }
  return { ok: true };
};

const validateSignedRequest = ({ secret, headers, rawBody, nowSeconds, maxClockSkewSeconds }) => {
  const contentType = getHeader(headers, "content-type");
  if (!/^application\/json(?:\s*;|\s*$)/i.test(contentType)) {
    fail("INVALID_CONTENT_TYPE", "INVALID_CONTENT_TYPE");
  }
  const deviceId = getHeader(headers, "x-device-id");
  const timestampText = getHeader(headers, "x-timestamp");
  const nonce = getHeader(headers, "x-nonce");
  if (!DEVICE_ID_PATTERN.test(deviceId)) {
    fail("INVALID_DEVICE_ID", "INVALID_DEVICE_ID", 401);
  }
  if (!/^\d{10}$/.test(timestampText)) {
    fail("INVALID_TIMESTAMP", "INVALID_TIMESTAMP", 401);
  }
  if (!NONCE_PATTERN.test(nonce)) {
    fail("INVALID_NONCE", "INVALID_NONCE", 401);
  }
  const timestamp = Number(timestampText);
  const now = typeof nowSeconds === "number" ? nowSeconds : Math.floor(Date.now() / 1000);
  const skew = typeof maxClockSkewSeconds === "number" ? maxClockSkewSeconds : MAX_CLOCK_SKEW_SECONDS;
  if (Math.abs(now - timestamp) > skew) {
    fail("TIMESTAMP_EXPIRED", "TIMESTAMP_EXPIRED", 401);
  }
  verifyRequestSignature({ secret, headers, rawBody });
  let payload;
  try {
    payload = JSON.parse(Buffer.from(rawBody).toString("utf8"));
  } catch (error) {
    fail("INVALID_JSON", "INVALID_JSON");
  }
  const telemetry = validateTelemetry(payload);
  if (telemetry.deviceId !== deviceId) {
    fail("DEVICE_ID_MISMATCH", "DEVICE_ID_MISMATCH", 401);
  }
  return { telemetry, deviceId, timestamp, nonce };
};

const isDuplicateKeyError = (error) => Boolean(error && (error.code === "DUPLICATE_KEY" || error.code === 11000 || /duplicate/i.test(String(error.message || ""))));

const createTelemetryProcessor = ({ store, resolveSecret, nowSeconds, maxClockSkewSeconds }) => {
  if (!store || typeof store.getDevice !== "function" || typeof resolveSecret !== "function") {
    throw new Error("Telemetry processor requires a store and resolveSecret");
  }
  const safeLog = (entry) => {
    if (typeof store.log === "function") {
      store.log(entry);
    }
  };
  return {
    async process({ headers, rawBody }) {
      let deviceId = "";
      try {
        deviceId = getHeader(headers, "x-device-id");
        const secret = resolveSecret(deviceId);
        const validated = validateSignedRequest({
          secret,
          headers,
          rawBody,
          nowSeconds: typeof nowSeconds === "function" ? nowSeconds() : nowSeconds,
          maxClockSkewSeconds
        });
        const device = await store.getDevice(validated.deviceId);
        if (!device || device.enabled !== true) {
          fail("DEVICE_DISABLED", "DEVICE_DISABLED", 403);
        }
        if (device.lastNonce === validated.nonce) {
          fail("REPLAYED_NONCE", "REPLAYED_NONCE", 409);
        }
        if (Number(device.lastAcceptedTimestamp || 0) >= validated.timestamp) {
          fail("STALE_TIMESTAMP", "STALE_TIMESTAMP", 409);
        }
        const receivedAt = new Date((typeof nowSeconds === "function" ? nowSeconds() : nowSeconds || Math.floor(Date.now() / 1000)) * 1000).toISOString();
        await store.setStatus({
          _id: validated.telemetry.deviceId,
          deviceId: validated.telemetry.deviceId,
          reportedAt: validated.telemetry.reportedAt,
          receivedAt,
          location: validated.telemetry.status.location,
          attitude: validated.telemetry.status.attitude,
          risk: validated.telemetry.status.risk,
          battery: validated.telemetry.status.battery,
          network: validated.telemetry.status.network,
          firmwareVersion: validated.telemetry.status.firmwareVersion
        });
        let idempotent = false;
        for (let index = 0; index < validated.telemetry.trackPoints.length; index += 1) {
          try {
            await store.insertTrackPoint(Object.assign({ receivedAt }, validated.telemetry.trackPoints[index]));
          } catch (error) {
            if (!isDuplicateKeyError(error)) {
              throw error;
            }
            idempotent = true;
          }
        }
        for (let index = 0; index < validated.telemetry.alarms.length; index += 1) {
          try {
            await store.insertAlarm(Object.assign({ receivedAt }, validated.telemetry.alarms[index]));
          } catch (error) {
            if (!isDuplicateKeyError(error)) {
              throw error;
            }
            idempotent = true;
          }
        }
        await store.updateDevice(validated.telemetry.deviceId, {
          lastSeenAt: receivedAt,
          lastAcceptedTimestamp: validated.timestamp,
          lastNonce: validated.nonce,
          firmwareVersion: validated.telemetry.status.firmwareVersion,
          updatedAt: receivedAt
        });
        safeLog({ event: "telemetry_accepted", deviceId: validated.telemetry.deviceId, requestId: validated.telemetry.requestId, idempotent });
        return { ok: true, data: { requestId: validated.telemetry.requestId, idempotent } };
      } catch (error) {
        const known = error instanceof TelemetryError;
        const code = known ? error.code : "INTERNAL_ERROR";
        safeLog({ event: "telemetry_rejected", deviceId, code });
        return {
          ok: false,
          error: { code, message: known ? error.message : "INTERNAL_ERROR" },
          statusCode: known ? error.statusCode : 500
        };
      }
    }
  };
};

module.exports = {
  MAX_CLOCK_SKEW_SECONDS,
  TelemetryError,
  createTelemetryProcessor,
  getHeader,
  signRequest,
  validateSignedRequest,
  validateTelemetry,
  verifyRequestSignature
};

const DEVICE_ID = "bag001";

const getHeader = (headers, name) => {
  const wanted = name.toLowerCase();
  const key = Object.keys(headers || {}).find((item) => item.toLowerCase() === wanted);
  const value = key ? headers[key] : "";
  return Array.isArray(value) ? value[0] : value;
};

const createTelemetryProcessor = ({ store, uploadToken, now }) => {
  if (!store || typeof store.setStatus !== "function") {
    throw new Error("Telemetry processor requires a store");
  }
  return {
    async process({ headers, rawBody }) {
      if (!uploadToken || getHeader(headers, "x-upload-token") !== uploadToken) {
        return { success: false, error: { code: "INVALID_UPLOAD_TOKEN" }, statusCode: 401 };
      }
      let payload;
      try {
        payload = JSON.parse(Buffer.from(rawBody).toString("utf8"));
      } catch (error) {
        return { success: false, error: { code: "INVALID_JSON" }, statusCode: 400 };
      }
      if (!payload || typeof payload !== "object" || Array.isArray(payload)) {
        return { success: false, error: { code: "INVALID_JSON" }, statusCode: 400 };
      }
      const receivedAt = typeof now === "function" ? now() : new Date().toISOString();
      await store.setStatus(Object.assign({}, payload.status || {}, {
        _id: DEVICE_ID,
        deviceId: DEVICE_ID,
        receivedAt
      }));
      const trackPoints = Array.isArray(payload.trackPoints) ? payload.trackPoints : [];
      for (let index = 0; index < trackPoints.length; index += 1) {
        await store.insertTrackPoint(Object.assign({}, trackPoints[index], { deviceId: DEVICE_ID, receivedAt }));
      }
      const alarms = Array.isArray(payload.alarms) ? payload.alarms : [];
      for (let index = 0; index < alarms.length; index += 1) {
        await store.insertAlarm(Object.assign({}, alarms[index], { deviceId: DEVICE_ID, receivedAt }));
      }
      return { success: true };
    }
  };
};

module.exports = { DEVICE_ID, createTelemetryProcessor, getHeader };

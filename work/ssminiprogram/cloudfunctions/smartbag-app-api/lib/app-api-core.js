const crypto = require("crypto");

const MAX_PAGE_SIZE = 100;
const DEFAULT_PAGE_SIZE = 50;
const MAX_CURSOR_OFFSET = 10000;
const DEVICE_ID_PATTERN = /^[A-Za-z0-9_-]{3,64}$/;

class ApiError extends Error {
  constructor(code, message) {
    super(message || code);
    this.code = code;
  }
}

const reject = (code) => { throw new ApiError(code, code); };
const success = (data) => ({ ok: true, data });
const failure = (error) => ({ ok: false, error: { code: error.code || "INTERNAL_ERROR", message: error.code || "INTERNAL_ERROR" } });

const requireDeviceId = (value) => {
  if (typeof value !== "string" || !DEVICE_ID_PATTERN.test(value)) {
    reject("INVALID_DEVICE_ID");
  }
  return value;
};

const parsePage = (event) => {
  const pageSize = typeof event.pageSize === "undefined" ? DEFAULT_PAGE_SIZE : Number(event.pageSize);
  if (!Number.isInteger(pageSize) || pageSize < 1 || pageSize > MAX_PAGE_SIZE) {
    reject("INVALID_PAGE_SIZE");
  }
  const offset = typeof event.cursor === "undefined" || event.cursor === null || event.cursor === "" ? 0 : Number(event.cursor);
  if (!Number.isInteger(offset) || offset < 0 || offset > MAX_CURSOR_OFFSET) {
    reject("INVALID_CURSOR");
  }
  return { pageSize, offset };
};

const safeEqualText = (left, right) => {
  const first = Buffer.from(String(left || ""));
  const second = Buffer.from(String(right || ""));
  return first.length === second.length && crypto.timingSafeEqual(first, second);
};

const publicStatus = (item) => {
  if (!item) {
    return null;
  }
  return {
    deviceId: item.deviceId,
    reportedAt: item.reportedAt,
    receivedAt: item.receivedAt,
    location: item.location || null,
    attitude: item.attitude || null,
    risk: item.risk || null,
    battery: item.battery || null,
    network: item.network || null,
    firmwareVersion: item.firmwareVersion || null
  };
};

const publicHistory = (item, kind) => {
  if (kind === "track") {
    return {
      reportedAt: item.reportedAt,
      receivedAt: item.receivedAt,
      location: item.location || null,
      speed: item.speed,
      heading: item.heading
    };
  }
  return {
    reportedAt: item.reportedAt,
    receivedAt: item.receivedAt,
    alarmType: item.alarmType,
    riskLevel: item.riskLevel,
    direction: item.direction,
    message: item.message,
    location: item.location || null
  };
};

const createAppApi = ({ getOpenId, repository, now }) => {
  if (typeof getOpenId !== "function" || !repository) {
    throw new Error("App API requires getOpenId and repository");
  }
  const getCaller = () => {
    const openid = getOpenId();
    if (typeof openid !== "string" || !openid) {
      reject("UNAUTHENTICATED");
    }
    return openid;
  };
  const requireBinding = async (openid, deviceId) => {
    const binding = await repository.findBinding({ openid, deviceId });
    if (!binding) {
      reject("DEVICE_NOT_BOUND");
    }
    return binding;
  };
  const handle = async (event) => {
    try {
      const request = event || {};
      const action = request.action;
      const openid = getCaller();
      if (action === "whoami") {
        return success({ openid });
      }
      const deviceId = requireDeviceId(request.deviceId);
      if (action === "bindDevice") {
        const device = await repository.findDevice(deviceId);
        if (!device || device.enabled !== true) {
          reject("DEVICE_DISABLED");
        }
        const existing = await repository.findBinding({ openid, deviceId });
        if (existing) {
          return success({ deviceId, role: existing.role, alreadyBound: true });
        }
        if (!device.bindingTokenHash || typeof request.pairingToken !== "string") {
          reject("PAIRING_NOT_CONFIGURED");
        }
        const actualHash = crypto.createHash("sha256").update(request.pairingToken).digest("hex");
        if (!safeEqualText(actualHash, device.bindingTokenHash)) {
          reject("INVALID_PAIRING_TOKEN");
        }
        const createdAt = typeof now === "function" ? now() : new Date().toISOString();
        await repository.createBinding({ deviceId, openid, role: "owner", createdAt });
        return success({ deviceId, role: "owner", alreadyBound: false });
      }
      await requireBinding(openid, deviceId);
      if (action === "getLatestStatus") {
        return success({ status: publicStatus(await repository.getStatus(deviceId)) });
      }
      if (action === "getTrackPoints" || action === "getAlarmHistory") {
        const page = parsePage(request);
        const kind = action === "getTrackPoints" ? "track" : "alarm";
        const records = await (kind === "track" ? repository.listTrackPoints(deviceId, page.offset, page.pageSize + 1) : repository.listAlarmHistory(deviceId, page.offset, page.pageSize + 1));
        const hasMore = records.length > page.pageSize;
        const items = records.slice(0, page.pageSize).map((item) => publicHistory(item, kind));
        return success({ items, nextCursor: hasMore ? String(page.offset + page.pageSize) : null, hasMore });
      }
      reject("UNSUPPORTED_ACTION");
    } catch (error) {
      return failure(error);
    }
  };
  return { handle };
};

module.exports = { MAX_PAGE_SIZE, createAppApi };

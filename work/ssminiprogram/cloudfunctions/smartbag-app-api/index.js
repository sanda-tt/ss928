const cloud = require("wx-server-sdk");
const { createAppApi } = require("./lib/app-api-core");

cloud.init({ env: cloud.DYNAMIC_CURRENT_ENV });
const db = cloud.database();

const findOne = async (query) => {
  const result = await query.limit(1).get();
  return result && result.data && result.data.length ? result.data[0] : null;
};

const repository = {
  async findDevice(deviceId) {
    try {
      const result = await db.collection("devices").doc(deviceId).get();
      return result && result.data ? result.data : null;
    } catch (error) {
      return null;
    }
  },
  async findBinding({ openid, deviceId }) {
    return findOne(db.collection("device_bindings").where({ openid, deviceId }));
  },
  async createBinding(document) {
    await db.collection("device_bindings").add({ data: document });
  },
  async getStatus(deviceId) {
    try {
      const result = await db.collection("device_status").doc(deviceId).get();
      return result && result.data ? result.data : null;
    } catch (error) {
      return null;
    }
  },
  async listTrackPoints(deviceId, offset, limit) {
    const result = await db.collection("track_points").where({ deviceId }).orderBy("receivedAt", "desc").skip(offset).limit(limit).get();
    return result && result.data ? result.data : [];
  },
  async listAlarmHistory(deviceId, offset, limit) {
    const result = await db.collection("alarm_history").where({ deviceId }).orderBy("receivedAt", "desc").skip(offset).limit(limit).get();
    return result && result.data ? result.data : [];
  }
};

const api = createAppApi({
  getOpenId: () => cloud.getWXContext().OPENID,
  repository
});

exports.main = async (event, context) => api.handle(event, context);

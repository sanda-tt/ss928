const FUNCTION_NAME = "smartbag-app-api";

const call = (action, data) => wx.cloud.callFunction({
  name: FUNCTION_NAME,
  data: Object.assign({ action }, data || {})
}).then((response) => {
  const result = response && response.result;
  if (!result || result.ok !== true) {
    const error = (result && result.error) || { code: "CLOUD_CALL_FAILED", message: "CLOUD_CALL_FAILED" };
    return Promise.reject(error);
  }
  return result.data;
});

const whoami = () => call("whoami");
const bindDevice = (deviceId, pairingToken) => call("bindDevice", { deviceId, pairingToken });
const getLatestStatus = (deviceId) => call("getLatestStatus", { deviceId });
const getTrackPoints = (deviceId, cursor, pageSize) => call("getTrackPoints", { deviceId, cursor, pageSize });
const getAlarmHistory = (deviceId, cursor, pageSize) => call("getAlarmHistory", { deviceId, cursor, pageSize });

module.exports = {
  bindDevice,
  getAlarmHistory,
  getLatestStatus,
  getTrackPoints,
  whoami
};

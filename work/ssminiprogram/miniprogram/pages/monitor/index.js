const cloudDataSource = require("../../services/cloud-data-source");
const postureUtils = require("../../utils/posture-utils");
const REFRESH_INTERVAL_MS = 1000;
const OFFLINE_AFTER_MS = 60000;
Page({
  data: { deviceLabel: "BMI270-Backpack", statusText: "设备离线", statusLevel: "offline", postureKey: "unknown", postureText: "暂无数据", postureDetail: "未收到设备的最新姿态状态", updateTime: "--:--:--", showReminder: false },
  onShow() { this.loadRealtimePosture(); this.startRefresh(); },
  onHide() { this.stopRefresh(); },
  onUnload() { this.stopRefresh(); },
  goAnalysis() { wx.redirectTo({ url: "/pages/index/index" }); },
  startRefresh() { if (!this.refreshTimer) this.refreshTimer = setInterval(() => this.loadRealtimePosture(), REFRESH_INTERVAL_MS); },
  stopRefresh() { if (this.refreshTimer) { clearInterval(this.refreshTimer); this.refreshTimer = null; } },
  loadRealtimePosture() { return cloudDataSource.getRealtimePosture().then((result) => this.setData(postureUtils.toRealtimeDisplay(postureUtils.normalizeRealtimePosture(result && result.posture), Date.now(), OFFLINE_AFTER_MS))).catch((error) => console.error("realtime posture cloud read failed", error && error.code)); }
});

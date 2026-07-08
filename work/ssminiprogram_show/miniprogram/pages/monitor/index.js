const demoData = require("../../utils/demo-data");

Page({
  data: {
    deviceLabel: "BMI270-Backpack 演示设备",
    statusText: "已连接（实时监控）",
    statusLevel: "connected",
    postureKey: "good",
    postureText: "良好姿态",
    postureDetail: "肩背保持稳定，书包受力均衡",
    updateTime: "20:17:12",
    showReminder: false,
    reminderText: ""
  },

  onLoad() {
    this.startedAt = Date.now();
    this.applyDemoState(0);
    this.demoTimer = setInterval(() => this.tickDemo(), 250);
  },

  onUnload() {
    if (this.demoTimer) {
      clearInterval(this.demoTimer);
      this.demoTimer = null;
    }
  },

  goAnalysis() {
    wx.redirectTo({ url: "/pages/index/index" });
  },

  tickDemo() {
    const elapsed = Date.now() - this.startedAt;
    const clamped = Math.min(elapsed, demoData.POSTURE_DEMO_DURATION_MS);
    this.applyDemoState(clamped);
    if (clamped >= demoData.POSTURE_DEMO_DURATION_MS && this.demoTimer) {
      clearInterval(this.demoTimer);
      this.demoTimer = null;
    }
  },

  applyDemoState(elapsedMs) {
    const state = demoData.getPostureDemoState(elapsedMs);
    this.setData({
      postureKey: state.postureKey,
      postureText: state.postureText,
      postureDetail: state.postureDetail,
      updateTime: state.clockText,
      showReminder: state.showReminder,
      reminderText: state.reminderText
    });
  }
});

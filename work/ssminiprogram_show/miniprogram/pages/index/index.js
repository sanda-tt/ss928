const demoData = require("../../utils/demo-data");

Page({
  data: {
    deviceLabel: "BMI270-Backpack 演示设备",
    statusText: "已连接",
    statusLevel: "connected",
    stats: demoData.POSTURE_STATS,
    records: demoData.POSTURE_RECORDS
  },

  onReady() {
    this.drawPosturePie();
  },

  goMonitor() {
    wx.redirectTo({ url: "/pages/monitor/index" });
  },

  drawPosturePie() {
    if (typeof wx === "undefined" || !wx.createCanvasContext) {
      return;
    }
    const ctx = wx.createCanvasContext("posturePie", this);
    const center = 90;
    const radius = 74;
    const startAngle = -Math.PI / 2;
    const goodAngle = Math.PI * 2 * demoData.POSTURE_STATS.goodPercent / 100;

    ctx.setFillStyle("#eef3f7");
    ctx.beginPath();
    ctx.arc(center, center, radius, 0, Math.PI * 2);
    ctx.fill();

    ctx.setFillStyle("#18a058");
    ctx.beginPath();
    ctx.moveTo(center, center);
    ctx.arc(center, center, radius, startAngle, startAngle + goodAngle);
    ctx.closePath();
    ctx.fill();

    ctx.setFillStyle("#e5534b");
    ctx.beginPath();
    ctx.moveTo(center, center);
    ctx.arc(center, center, radius, startAngle + goodAngle, startAngle + Math.PI * 2);
    ctx.closePath();
    ctx.fill();

    ctx.setFillStyle("#ffffff");
    ctx.beginPath();
    ctx.arc(center, center, 42, 0, Math.PI * 2);
    ctx.fill();

    ctx.setFillStyle("#17212b");
    ctx.setFontSize(16);
    ctx.setTextAlign("center");
    ctx.fillText("今日", center, center - 4);
    ctx.setFontSize(13);
    ctx.fillText("姿态", center, center + 18);
    ctx.draw();
  }
});

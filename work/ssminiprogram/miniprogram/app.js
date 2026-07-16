App({
  globalData: {
    appName: "智能安全背包"
  },
  onLaunch() {
    wx.cloud.init({
      env: "cloud1-d7gdmg27139f4fbf2",
      traceUser: true
    });
  }
});

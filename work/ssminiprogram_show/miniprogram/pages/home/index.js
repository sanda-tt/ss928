const FEATURE_ENTRIES = [
  {
    key: "alerts",
    title: "危险报警历史",
    subtitle: "报警记录 / 时间线",
    icon: "警",
    tone: "red",
    route: "/pages/placeholder/index?kind=alerts"
  },
  {
    key: "tracks",
    title: "儿童安全轨迹跟踪",
    subtitle: "位置轨迹 / 实时查看",
    icon: "轨",
    tone: "blue",
    route: "/pages/tracks/index"
  },
  {
    key: "posture",
    title: "姿态分析和记录",
    subtitle: "姿态识别 / 历史记录",
    icon: "姿",
    tone: "purple",
    route: "/pages/index/index"
  },
  {
    key: "settings",
    title: "设置",
    subtitle: "参数设置",
    icon: "设",
    tone: "green",
    route: "/pages/placeholder/index?kind=settings"
  }
];

Page({
  data: {
    featureEntries: FEATURE_ENTRIES,
    connectionStatusText: "设备在线",
    monitorStatusText: "实时监控"
  },

  openFeature(e) {
    const route = e.currentTarget.dataset.route;
    if (!route) {
      return;
    }
    wx.navigateTo({ url: route });
  }
});

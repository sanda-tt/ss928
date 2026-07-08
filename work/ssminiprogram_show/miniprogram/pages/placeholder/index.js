const alarm = require("../../utils/alarm-utils");

const PLACEHOLDERS = {
  settings: {
    title: "设置",
    subtitle: "参数设置",
    state: "待接入参数设置",
    detail: "后续可在这里配置设备连接、报警阈值和提醒参数。"
  }
};

Page({
  data: {
    title: "功能模块",
    subtitle: "规划中",
    state: "待开发",
    detail: "该入口已预留。",
    isAlertsMode: false,
    isAlarmDetail: false,
    alarmRecords: [],
    selectedAlarm: null,
    alarmMarkers: []
  },

  onLoad(options) {
    const kind = options && options.kind;

    if (kind === "alerts") {
      this.loadAlertPage(options && options.alarmId);
      return;
    }

    const item = PLACEHOLDERS[kind] || PLACEHOLDERS.settings;
    this.setData(Object.assign({ isAlertsMode: false, isAlarmDetail: false }, item));
    wx.setNavigationBarTitle({ title: item.title });
  },

  loadAlertPage(alarmId) {
    const records = alarm.getAlarmRecords();

    if (alarmId) {
      const selected = alarm.findAlarmById(alarmId);
      this.setData({
        isAlertsMode: true,
        isAlarmDetail: true,
        alarmRecords: records,
        selectedAlarm: selected,
        alarmMarkers: alarm.buildAlarmMarker(selected)
      });
      wx.setNavigationBarTitle({ title: "报警详情" });
      return;
    }

    this.setData({
      title: "危险报警历史",
      subtitle: "报警记录 / 时间线",
      isAlertsMode: true,
      isAlarmDetail: false,
      alarmRecords: records,
      selectedAlarm: null,
      alarmMarkers: []
    });
    wx.setNavigationBarTitle({ title: "危险报警历史" });
  },

  openAlarmDetail(e) {
    const id = e.currentTarget.dataset.id;
    if (!id) {
      return;
    }
    wx.navigateTo({
      url: "/pages/placeholder/index?kind=alerts&alarmId=" + encodeURIComponent(id)
    });
  },

  goAlarmList() {
    const pages = getCurrentPages();
    if (pages.length > 1) {
      wx.navigateBack({ delta: 1 });
      return;
    }
    wx.redirectTo({ url: "/pages/placeholder/index?kind=alerts" });
  },

  goHome() {
    wx.navigateBack({
      delta: 1,
      fail: () => wx.redirectTo({ url: "/pages/home/index" })
    });
  }
});

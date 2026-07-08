const demoData = require("../../utils/demo-data");
const trackUtils = require("../../utils/track-utils");

const MAX_MAP_POINTS = 800;

Page({
  data: {
    connected: true,
    statusText: "已连接",
    statusLevel: "connected",
    deviceLabel: "DX-GP21-Track 演示设备",
    currentTrackLabel: "人民公园实时安全轨迹",
    pointCount: 0,
    displayCount: 0,
    hasTrack: true,
    latestMeta: "实时轨迹准备中",
    latitude: demoData.PEOPLE_PARK.latitude,
    longitude: demoData.PEOPLE_PARK.longitude,
    scale: 17,
    markers: [],
    polyline: [],
    liveEnabled: true,
    accuracyText: "--",
    speedText: "--",
    commandResponse: "实时推送已开启",
    recentPoints: []
  },

  onLoad() {
    this.trackPoints = demoData.createDemoTrackPoints();
    this.telemetryTick = this.trackPoints.length;
    this.renderDemoTrack(false);
    this.pointTimer = setInterval(() => this.appendDemoPoint(), 5000);
    this.telemetryTimer = setInterval(() => this.refreshTelemetry(), 3000);
  },

  onUnload() {
    if (this.pointTimer) {
      clearInterval(this.pointTimer);
      this.pointTimer = null;
    }
    if (this.telemetryTimer) {
      clearInterval(this.telemetryTimer);
      this.telemetryTimer = null;
    }
  },

  appendDemoPoint() {
    const next = demoData.createNextTrackPoint(this.trackPoints.length);
    this.trackPoints.push(next);
    this.telemetryTick = this.trackPoints.length;
    this.renderDemoTrack(true);
  },

  refreshTelemetry() {
    this.telemetryTick += 1;
    const telemetry = demoData.createTelemetrySample(this.telemetryTick);
    const last = this.trackPoints[this.trackPoints.length - 1];
    if (!last) {
      return;
    }
    this.setData({
      accuracyText: telemetry.accuracy.toFixed(1) + "m",
      speedText: telemetry.speed.toFixed(1) + "m/s",
      latestMeta: this.formatPointTime(last.t) + "  精度 " + telemetry.accuracy.toFixed(1) + "m  速度 " + telemetry.speed.toFixed(1) + "m/s"
    });
  },

  renderDemoTrack(keepScale) {
    const normalized = [];
    for (let i = 0; i < this.trackPoints.length; i += 1) {
      const point = trackUtils.normalizeTrackPoint(this.trackPoints[i]);
      if (point) {
        normalized.push(point);
      }
    }
    const display = trackUtils.downsampleTrackPoints(normalized, MAX_MAP_POINTS);
    const last = normalized[normalized.length - 1];
    const telemetry = demoData.createTelemetrySample(this.telemetryTick);
    const patch = {
      pointCount: normalized.length,
      displayCount: display.length,
      hasTrack: display.length > 0,
      markers: trackUtils.buildMarkers(display),
      polyline: trackUtils.buildPolyline(display),
      accuracyText: telemetry.accuracy.toFixed(1) + "m",
      speedText: telemetry.speed.toFixed(1) + "m/s",
      recentPoints: this.buildRecentPoints(normalized)
    };
    if (last) {
      patch.latitude = last.latitude;
      patch.longitude = last.longitude;
      patch.latestMeta = this.formatPointTime(last.time) + "  精度 " + telemetry.accuracy.toFixed(1) + "m  速度 " + telemetry.speed.toFixed(1) + "m/s";
      if (!keepScale) {
        patch.scale = 17;
      }
    }
    this.setData(patch);
  },

  buildRecentPoints(points) {
    const rows = [];
    const start = Math.max(0, points.length - 5);
    for (let i = points.length - 1; i >= start; i -= 1) {
      const point = points[i];
      rows.push({
        id: "p-" + i,
        time: this.formatPointTime(point.time),
        location: "人民公园步道 " + (i + 1),
        meta: "精度 " + this.formatNumber(point.accuracy) + "m · 速度 " + this.formatNumber(point.speed) + "m/s"
      });
    }
    return rows;
  },

  centerMap() {
    const last = this.trackPoints[this.trackPoints.length - 1];
    if (!last) {
      return;
    }
    this.setData({
      latitude: last.lat,
      longitude: last.lon,
      scale: 17
    });
  },

  formatPointTime(seconds) {
    const date = new Date(Number(seconds) * 1000);
    return ("0" + date.getHours()).slice(-2) + ":" + ("0" + date.getMinutes()).slice(-2) + ":" + ("0" + date.getSeconds()).slice(-2);
  },

  formatNumber(value) {
    const num = Number(value);
    return Number.isFinite(num) ? num.toFixed(1) : "--";
  }
});

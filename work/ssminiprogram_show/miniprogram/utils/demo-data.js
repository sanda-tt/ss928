const PEOPLE_PARK = {
  name: "四川成都人民公园",
  latitude: 30.656990,
  longitude: 104.057641
};

const INITIAL_TRACK_COUNT = 235;
const TRACK_STEP_SECONDS = 5;
const TRACK_START_SECONDS = 1783569600;
const POSTURE_DEMO_DURATION_MS = 45000;

const POSTURE_STATS = {
  goodPercent: 79,
  badPercent: 21,
  goodLabel: "良好姿态",
  badLabel: "不良姿态"
};

const PEOPLE_PARK_ROUTE_ANCHORS = [
  { name: "人民公园东门", lat: 30.656990, lon: 104.057641 },
  { name: "祠堂街步道", lat: 30.656720, lon: 104.057120 },
  { name: "湖心东侧", lat: 30.656410, lon: 104.056550 },
  { name: "湖心南侧", lat: 30.656050, lon: 104.056030 },
  { name: "西侧林荫道", lat: 30.656390, lon: 104.055520 },
  { name: "少城路南侧", lat: 30.656930, lon: 104.055740 },
  { name: "鹤鸣茶社附近", lat: 30.657360, lon: 104.056180 },
  { name: "保路纪念碑步道", lat: 30.657620, lon: 104.056720 },
  { name: "北侧步道", lat: 30.657300, lon: 104.057290 },
  { name: "人民公园东门", lat: 30.656990, lon: 104.057641 }
];

const clamp = (value, min, max) => Math.max(min, Math.min(max, value));
const round = (value, digits) => {
  const base = Math.pow(10, digits);
  return Math.round(value * base) / base;
};
const pad2 = (value) => ("0" + value).slice(-2);

const formatClock = (secondsFromStart) => {
  const total = 20 * 3600 + 17 * 60 + 12 + secondsFromStart;
  const hour = Math.floor(total / 3600) % 24;
  const minute = Math.floor(total / 60) % 60;
  const second = total % 60;
  return pad2(hour) + ":" + pad2(minute) + ":" + pad2(second);
};

const createTelemetrySample = (index) => {
  const acc = clamp(6.4 + Math.sin(index * 0.71) * 1.5 + Math.cos(index * 0.19) * 0.7, 4.2, 8.8);
  const spd = clamp(1.0 + Math.sin(index * 0.31) * 0.25 + Math.cos(index * 0.13) * 0.15, 0.6, 1.4);
  return {
    accuracy: round(acc, 1),
    speed: round(spd, 1)
  };
};

const createDemoTrackPoint = (index) => {
  const segmentCount = PEOPLE_PARK_ROUTE_ANCHORS.length - 1;
  const routeIndex = index % INITIAL_TRACK_COUNT;
  const progress = routeIndex / (INITIAL_TRACK_COUNT - 1) * segmentCount;
  const segmentIndex = Math.min(segmentCount - 1, Math.floor(progress));
  const segmentProgress = progress - segmentIndex;
  const start = PEOPLE_PARK_ROUTE_ANCHORS[segmentIndex];
  const end = PEOPLE_PARK_ROUTE_ANCHORS[segmentIndex + 1];
  const ease = segmentProgress < 0.5 ?
    2 * segmentProgress * segmentProgress :
    1 - Math.pow(-2 * segmentProgress + 2, 2) / 2;
  const wiggle = Math.sin(index * 1.7) * 0.000018;
  const sideWiggle = Math.cos(index * 1.13) * 0.000012;
  const telemetry = createTelemetrySample(index);
  return {
    t: TRACK_START_SECONDS + index * TRACK_STEP_SECONDS,
    lat: round(start.lat + (end.lat - start.lat) * ease + wiggle, 6),
    lon: round(start.lon + (end.lon - start.lon) * ease + sideWiggle, 6),
    acc: telemetry.accuracy,
    spd: telemetry.speed,
    course: round((Math.atan2(end.lat - start.lat, end.lon - start.lon) * 180 / Math.PI + 360) % 360, 1),
    fix: 1,
    sat: 18 + (index % 5),
    src: "demo_people_park",
    cs: "gcj02"
  };
};

const createDemoTrackPoints = (count) => {
  const total = typeof count === "number" ? count : INITIAL_TRACK_COUNT;
  const points = [];
  for (let i = 0; i < total; i += 1) {
    points.push(createDemoTrackPoint(i));
  }
  return points;
};

const createNextTrackPoint = (index) => createDemoTrackPoint(index);

const createDemoTrackItems = () => {
  const rows = [];
  const names = ["人民公园东门", "鹤鸣茶社", "湖心步道", "辛亥秋保路死事纪念碑", "儿童活动区", "西侧林荫道"];
  for (let i = 0; i < names.length; i += 1) {
    rows.push({
      listIndex: i,
      id: "people-park-" + i,
      title: names[i] + "轨迹",
      timeRange: "20:" + pad2(17 - i) + " - 20:" + pad2(18 - i),
      n: INITIAL_TRACK_COUNT + i * 8,
      status: i === 0 ? "实时更新" : "已同步"
    });
  }
  return rows;
};

const POSTURE_RECORDS = [
  { id: "r10", time: "20:17:52", status: "不良姿态", tone: "bad", detail: "持续弯腰，已触发提醒" },
  { id: "r09", time: "20:17:48", status: "不良姿态", tone: "bad", detail: "背部前倾超过阈值" },
  { id: "r08", time: "20:17:43", status: "不良姿态", tone: "bad", detail: "姿态保持异常" },
  { id: "r07", time: "20:17:38", status: "良好姿态", tone: "good", detail: "短暂恢复直立" },
  { id: "r06", time: "20:17:34", status: "不良姿态", tone: "bad", detail: "前倾趋势明显" },
  { id: "r05", time: "20:17:29", status: "不良姿态", tone: "bad", detail: "进入不良姿态段" },
  { id: "r04", time: "20:17:24", status: "良好姿态", tone: "good", detail: "背部保持稳定" },
  { id: "r03", time: "20:17:20", status: "良好姿态", tone: "good", detail: "行走姿态正常" },
  { id: "r02", time: "20:17:16", status: "良好姿态", tone: "good", detail: "校准基准稳定" },
  { id: "r01", time: "20:17:12", status: "良好姿态", tone: "good", detail: "实时监控开始" }
];

const isGreenFlash = (elapsedMs) => (
  (elapsedMs >= 22500 && elapsedMs < 23250) ||
  (elapsedMs >= 33000 && elapsedMs < 33750)
);

const getPostureDemoState = (elapsedMs) => {
  const clamped = clamp(Number(elapsedMs) || 0, 0, POSTURE_DEMO_DURATION_MS);
  const elapsedSeconds = Math.floor(clamped / 1000);
  const good = clamped < 15000 || isGreenFlash(clamped);
  const reminder = clamped >= 40000;
  return {
    elapsedMs: clamped,
    clockText: formatClock(elapsedSeconds),
    postureKey: good ? "good" : "bad",
    postureText: good ? "良好姿态" : "不良姿态",
    postureDetail: good ? "肩背保持稳定，书包受力均衡" : "背部前倾持续，姿态需要纠正",
    showReminder: reminder,
    reminderText: reminder ? "已触发震动和语音提醒" : ""
  };
};

module.exports = {
  INITIAL_TRACK_COUNT,
  PEOPLE_PARK,
  PEOPLE_PARK_ROUTE_ANCHORS,
  POSTURE_DEMO_DURATION_MS,
  POSTURE_RECORDS,
  POSTURE_STATS,
  createDemoTrackItems,
  createDemoTrackPoints,
  createNextTrackPoint,
  createTelemetrySample,
  getPostureDemoState
};

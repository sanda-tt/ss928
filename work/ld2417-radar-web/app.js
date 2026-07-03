"use strict";

const $ = (selector) => document.querySelector(selector);

const ui = {
  canvas: $("#radarCanvas"),
  serialSupport: $("#serialSupport"),
  connectionState: $("#connectionState"),
  statusText: $("#statusText"),
  connectButton: $("#connectButton"),
  disconnectButton: $("#disconnectButton"),
  demoButton: $("#demoButton"),
  baudRate: $("#baudRate"),
  maxRange: $("#maxRange"),
  rangeValue: $("#rangeValue"),
  leftAngle: $("#leftAngle"),
  leftAngleValue: $("#leftAngleValue"),
  rightAngle: $("#rightAngle"),
  rightAngleValue: $("#rightAngleValue"),
  useYAxisDistance: $("#useYAxisDistance"),
  targetCount: $("#targetCount"),
  maxSpeedReadout: $("#maxSpeedReadout"),
  nearestReadout: $("#nearestReadout"),
  targetTable: $("#targetTable"),
  frameRate: $("#frameRate"),
  parserStats: $("#parserStats"),
  rawLog: $("#rawLog"),
  clearLogButton: $("#clearLogButton"),
};

const ctx = ui.canvas.getContext("2d");
const parser = new LD2417.Parser();
const targets = new Map();
const rawLogLines = [];

const state = {
  port: null,
  reader: null,
  connected: false,
  reading: false,
  demoTimer: null,
  demoStart: 0,
  frameTimes: [],
  lastTableRender: 0,
  fovHalfDeg: 60,
  ttlMs: 2600,
};

const config = {
  maxRangeM: Number(ui.maxRange.value),
  leftAngleDeg: Number(ui.leftAngle.value),
  rightAngleDeg: Number(ui.rightAngle.value),
  useYAxisDistance: ui.useYAxisDistance.checked,
};

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function lerp(a, b, t) {
  return a + (b - a) * t;
}

function degToRad(deg) {
  return (deg * Math.PI) / 180;
}

function speedColor(speedKmh, alpha = 1) {
  const stops = [
    { at: 0, rgb: [57, 189, 214] },
    { at: 0.35, rgb: [34, 166, 108] },
    { at: 0.62, rgb: [224, 181, 27] },
    { at: 1, rgb: [215, 53, 53] },
  ];
  const ratio = clamp(speedKmh / 175, 0, 1);

  let left = stops[0];
  let right = stops[stops.length - 1];
  for (let i = 0; i < stops.length - 1; i += 1) {
    if (ratio >= stops[i].at && ratio <= stops[i + 1].at) {
      left = stops[i];
      right = stops[i + 1];
      break;
    }
  }

  const t = right.at === left.at ? 0 : (ratio - left.at) / (right.at - left.at);
  const rgb = left.rgb.map((channel, index) => Math.round(lerp(channel, right.rgb[index], t)));
  return `rgba(${rgb[0]}, ${rgb[1]}, ${rgb[2]}, ${alpha})`;
}

function setBadge(element, text, tone = "") {
  element.textContent = text;
  element.className = `badge ${tone}`.trim();
}

function setStatus(text) {
  ui.statusText.textContent = text;
}

function updateControls() {
  ui.rangeValue.textContent = `${config.maxRangeM} m`;
  ui.leftAngleValue.textContent = `${config.leftAngleDeg}°`;
  ui.rightAngleValue.textContent = `${config.rightAngleDeg}°`;
  ui.disconnectButton.disabled = !state.connected && !state.demoTimer;
  ui.connectButton.disabled = !("serial" in navigator) || state.connected;
  ui.demoButton.textContent = state.demoTimer ? "停止演示" : "演示";
}

function appendLog(line) {
  rawLogLines.unshift(line);
  while (rawLogLines.length > 80) rawLogLines.pop();
  ui.rawLog.textContent = rawLogLines.join("\n");
}

function frameStamp() {
  const now = new Date();
  return now.toLocaleTimeString("zh-CN", { hour12: false });
}

function noteFrame() {
  const now = performance.now();
  state.frameTimes.push(now);
  state.frameTimes = state.frameTimes.filter((time) => now - time < 1000);
  ui.frameRate.textContent = `${state.frameTimes.length} fps`;
  ui.parserStats.textContent = `${parser.frames} 帧`;
}

function displayAngleFor(target) {
  if (target.directionCode === 1) return config.leftAngleDeg;
  if (target.directionCode === 2) return config.rightAngleDeg;
  return 0;
}

function directionText(target) {
  if (target.directionCode === 1) return "左";
  if (target.directionCode === 2) return "右";
  return `未知 ${target.directionCode}`;
}

function targetKey(target) {
  return `${target.directionCode}:${target.id}`;
}

function ingestFrame(frame) {
  const now = performance.now();
  noteFrame();
  appendLog(`${frameStamp()}  ${frame.rawHex}`);

  frame.targets.forEach((target, index) => {
    const key = targetKey(target);
    const previous = targets.get(key);
    const displayAngleDeg = displayAngleFor(target);
    const updated = {
      ...target,
      displayAngleDeg,
      rowIndex: index,
      lastSeen: now,
      history: previous?.history ? previous.history.slice(-24) : [],
    };

    updated.history.push({
      at: now,
      distanceM: target.distanceM,
      speedKmh: target.speedKmh,
      angleDeg: displayAngleDeg,
    });

    targets.set(key, updated);
  });
}

function consumeBytes(bytes) {
  const frames = parser.push(bytes);
  frames.forEach(ingestFrame);
  if (parser.errors > 0 || parser.droppedBytes > 0) {
    ui.parserStats.textContent = `${parser.frames} 帧 · 丢 ${parser.droppedBytes}`;
  }
}

async function connectSerial() {
  try {
    stopDemo();
    state.port = await navigator.serial.requestPort();
    await state.port.open({
      baudRate: Number(ui.baudRate.value),
      dataBits: 8,
      stopBits: 1,
      parity: "none",
      flowControl: "none",
    });
    state.connected = true;
    setBadge(ui.connectionState, "已连接", "good");
    setStatus(`串口已打开，波特率 ${ui.baudRate.value}。`);
    updateControls();
    readSerialLoop();
  } catch (error) {
    setStatus(`连接失败：${error.message}`);
  }
}

async function readSerialLoop() {
  if (!state.port || state.reading) return;
  state.reading = true;

  try {
    while (state.port?.readable) {
      state.reader = state.port.readable.getReader();
      try {
        while (true) {
          const { value, done } = await state.reader.read();
          if (done) break;
          if (value) consumeBytes(value);
        }
      } finally {
        state.reader.releaseLock();
        state.reader = null;
      }
    }
  } catch (error) {
    if (state.connected) setStatus(`读取中断：${error.message}`);
  } finally {
    state.reading = false;
  }
}

async function disconnectSerial() {
  stopDemo();
  try {
    if (state.reader) await state.reader.cancel();
    if (state.port) await state.port.close();
  } catch (error) {
    setStatus(`断开时出现问题：${error.message}`);
  } finally {
    state.reader = null;
    state.port = null;
    state.connected = false;
    setBadge(ui.connectionState, "未连接");
    setStatus("已断开。");
    updateControls();
  }
}

function makeDemoTargets(t) {
  const firstDistance = 16 + ((t * 8) % 76);
  const secondDistance = 68 - ((t * 5) % 42);
  const fastSpeed = 42 + 58 * (0.5 + 0.5 * Math.sin(t * 0.8));
  const slowSpeed = 12 + 28 * (0.5 + 0.5 * Math.sin(t * 1.35 + 1.2));
  const targetsOut = [
    {
      id: 1,
      directionCode: 1,
      distanceM: firstDistance,
      speedKmh: fastSpeed,
      highSpeed: fastSpeed > 80,
      associated: true,
      trackAge: Math.floor(t * 5) & 0xff,
    },
    {
      id: 2,
      directionCode: 2,
      distanceM: Math.max(6, secondDistance),
      speedKmh: slowSpeed,
      highSpeed: slowSpeed > 80,
      associated: true,
      trackAge: Math.floor(t * 4) & 0xff,
    },
  ];

  if (Math.floor(t) % 7 >= 4) {
    targetsOut.push({
      id: 3,
      directionCode: Math.floor(t) % 2 ? 1 : 2,
      distanceM: 24 + 18 * (0.5 + 0.5 * Math.cos(t * 1.1)),
      speedKmh: 105 + 36 * (0.5 + 0.5 * Math.sin(t * 1.6)),
      highSpeed: true,
      associated: true,
      trackAge: Math.floor(t * 3) & 0xff,
    });
  }

  return targetsOut;
}

function startDemo() {
  if (state.demoTimer) {
    stopDemo();
    return;
  }

  if (state.connected) disconnectSerial();
  state.demoStart = performance.now();
  parser.reset();
  targets.clear();
  setBadge(ui.connectionState, "演示中", "good");
  setStatus("正在播放模拟 LD2417 数据帧。");
  state.demoTimer = window.setInterval(() => {
    const elapsed = (performance.now() - state.demoStart) / 1000;
    consumeBytes(LD2417.makeFrame(makeDemoTargets(elapsed)));
  }, 90);
  updateControls();
}

function stopDemo() {
  if (!state.demoTimer) return;
  window.clearInterval(state.demoTimer);
  state.demoTimer = null;
  if (!state.connected) {
    setBadge(ui.connectionState, "未连接");
    setStatus("演示已停止。");
  }
  updateControls();
}

function resizeCanvas() {
  const rect = ui.canvas.getBoundingClientRect();
  const dpr = window.devicePixelRatio || 1;
  const width = Math.max(1, Math.round(rect.width * dpr));
  const height = Math.max(1, Math.round(rect.height * dpr));
  if (ui.canvas.width !== width || ui.canvas.height !== height) {
    ui.canvas.width = width;
    ui.canvas.height = height;
  }
  ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  return { width: rect.width, height: rect.height };
}

function pointFor(cx, cy, radiusPx, distanceM, angleDeg) {
  const angle = degToRad(angleDeg);
  let plotRangeM = distanceM;
  if (config.useYAxisDistance) {
    const cos = Math.max(0.18, Math.cos(angle));
    plotRangeM = distanceM / cos;
  }
  const r = (clamp(plotRangeM, 0, config.maxRangeM) / config.maxRangeM) * radiusPx;
  return {
    x: cx + Math.sin(angle) * r,
    y: cy - Math.cos(angle) * r,
    r,
    clipped: plotRangeM > config.maxRangeM,
  };
}

function arcPath(cx, cy, radius, startDeg, endDeg) {
  ctx.arc(cx, cy, radius, degToRad(startDeg) - Math.PI / 2, degToRad(endDeg) - Math.PI / 2, false);
}

function drawGrid(cx, cy, radius, width, height) {
  ctx.save();
  ctx.clearRect(0, 0, width, height);

  ctx.fillStyle = "#07100e";
  ctx.fillRect(0, 0, width, height);

  ctx.beginPath();
  ctx.moveTo(cx, cy);
  arcPath(cx, cy, radius, -state.fovHalfDeg, state.fovHalfDeg);
  ctx.closePath();
  ctx.fillStyle = "rgba(20, 73, 55, 0.34)";
  ctx.fill();

  ctx.lineWidth = 1;
  ctx.strokeStyle = "rgba(127, 216, 181, 0.30)";
  const ringStep = config.maxRangeM <= 30 ? 5 : config.maxRangeM <= 60 ? 10 : 20;
  for (let m = ringStep; m <= config.maxRangeM; m += ringStep) {
    const ringR = (m / config.maxRangeM) * radius;
    ctx.beginPath();
    arcPath(cx, cy, ringR, -state.fovHalfDeg, state.fovHalfDeg);
    ctx.stroke();

    const label = `${m}m`;
    ctx.fillStyle = "rgba(218, 244, 232, 0.58)";
    ctx.font = "12px Microsoft YaHei, Segoe UI, sans-serif";
    ctx.fillText(label, cx + 8, cy - ringR - 3);
  }

  [-60, -30, 0, 30, 60].forEach((deg) => {
    const point = pointFor(cx, cy, radius, config.maxRangeM, deg);
    ctx.beginPath();
    ctx.moveTo(cx, cy);
    ctx.lineTo(point.x, point.y);
    ctx.strokeStyle = deg === 0 ? "rgba(222, 255, 240, 0.55)" : "rgba(127, 216, 181, 0.24)";
    ctx.stroke();
    ctx.fillStyle = "rgba(218, 244, 232, 0.70)";
    ctx.font = "12px Microsoft YaHei, Segoe UI, sans-serif";
    const labelX = cx + Math.sin(degToRad(deg)) * (radius + 14);
    const labelY = cy - Math.cos(degToRad(deg)) * (radius + 14);
    ctx.fillText(`${deg}°`, labelX - 14, labelY + 4);
  });

  ctx.fillStyle = "rgba(218, 244, 232, 0.70)";
  ctx.font = "13px Microsoft YaHei, Segoe UI, sans-serif";
  ctx.fillText("左", Math.max(18, cx - radius * 0.72), cy - radius * 0.36);
  ctx.fillText("中", cx - 7, cy - radius - 20);
  ctx.fillText("右", Math.min(width - 32, cx + radius * 0.72), cy - radius * 0.36);

  ctx.restore();
}

function drawTarget(target, cx, cy, radius, now) {
  const age = now - target.lastSeen;
  const alpha = clamp(1 - age / state.ttlMs, 0, 1);
  const color = speedColor(target.speedKmh, alpha);
  const point = pointFor(cx, cy, radius, target.distanceM, target.displayAngleDeg);

  ctx.save();
  target.history.forEach((sample, index) => {
    const sampleAge = now - sample.at;
    if (sampleAge > state.ttlMs) return;
    const p = pointFor(cx, cy, radius, sample.distanceM, sample.angleDeg);
    const a = clamp((1 - sampleAge / state.ttlMs) * (index / target.history.length), 0, 0.42);
    ctx.beginPath();
    ctx.arc(p.x, p.y, 3.5, 0, Math.PI * 2);
    ctx.fillStyle = speedColor(sample.speedKmh, a);
    ctx.fill();
  });

  const sectorStart = target.directionCode === 1 ? -state.fovHalfDeg : 0;
  const sectorEnd = target.directionCode === 1 ? 0 : state.fovHalfDeg;
  ctx.beginPath();
  arcPath(cx, cy, point.r, sectorStart, sectorEnd);
  ctx.strokeStyle = speedColor(target.speedKmh, 0.22 * alpha);
  ctx.lineWidth = 4;
  ctx.stroke();

  ctx.beginPath();
  ctx.arc(point.x, point.y, target.highSpeed ? 10 : 8, 0, Math.PI * 2);
  ctx.fillStyle = color;
  ctx.shadowColor = speedColor(target.speedKmh, 0.72 * alpha);
  ctx.shadowBlur = 18;
  ctx.fill();

  ctx.lineWidth = 2;
  ctx.strokeStyle = point.clipped ? "rgba(255,255,255,0.96)" : "rgba(7,16,14,0.82)";
  ctx.stroke();
  ctx.shadowBlur = 0;

  const label = `#${target.id} ${target.speedKmh.toFixed(0)}km/h`;
  ctx.font = "12px Microsoft YaHei, Segoe UI, sans-serif";
  const textW = ctx.measureText(label).width;
  const labelX = clamp(point.x + 12, 8, ui.canvas.clientWidth - textW - 10);
  const labelY = clamp(point.y - 10, 22, ui.canvas.clientHeight - 14);
  ctx.fillStyle = "rgba(4, 12, 10, 0.72)";
  ctx.fillRect(labelX - 5, labelY - 15, textW + 10, 22);
  ctx.fillStyle = "rgba(245, 255, 250, 0.94)";
  ctx.fillText(label, labelX, labelY);
  ctx.restore();
}

function activeTargets(now) {
  for (const [key, target] of targets) {
    if (now - target.lastSeen > state.ttlMs) targets.delete(key);
  }
  return Array.from(targets.values()).sort((a, b) => a.distanceM - b.distanceM);
}

function renderReadout(list) {
  ui.targetCount.textContent = String(list.length);
  if (!list.length) {
    ui.maxSpeedReadout.textContent = "0.0 km/h";
    ui.nearestReadout.textContent = "-- m";
    return;
  }

  const maxSpeed = Math.max(...list.map((target) => target.speedKmh));
  const nearest = Math.min(...list.map((target) => target.distanceM));
  ui.maxSpeedReadout.textContent = `${maxSpeed.toFixed(1)} km/h`;
  ui.nearestReadout.textContent = `${nearest.toFixed(2)} m`;
}

function renderTargetTable(list) {
  if (!list.length) {
    ui.targetTable.innerHTML = '<tr><td colspan="5" class="empty">暂无目标</td></tr>';
    return;
  }

  ui.targetTable.innerHTML = list
    .map((target) => {
      const status = [
        target.highSpeed ? '<span class="status-pill hot">高速</span>' : "",
        target.associated ? '<span class="status-pill locked">关联</span>' : "",
      ]
        .filter(Boolean)
        .join(" ");

      return `<tr>
        <td>${target.id}</td>
        <td>${directionText(target)} ${target.displayAngleDeg}°</td>
        <td>${target.distanceM.toFixed(2)} m</td>
        <td style="color:${speedColor(target.speedKmh, 1)}">${target.speedKmh.toFixed(1)} km/h</td>
        <td>${status || `0x${target.statusRaw.toString(16).padStart(4, "0")}`}</td>
      </tr>`;
    })
    .join("");
}

function draw() {
  const now = performance.now();
  const { width, height } = resizeCanvas();
  const cx = width / 2;
  const cy = height * 0.9;
  const radius = Math.min(width * 0.43, height * 0.78);
  const list = activeTargets(now);

  drawGrid(cx, cy, radius, width, height);
  list.forEach((target) => drawTarget(target, cx, cy, radius, now));
  renderReadout(list);

  if (now - state.lastTableRender > 160) {
    renderTargetTable(list);
    state.lastTableRender = now;
  }

  requestAnimationFrame(draw);
}

function bindEvents() {
  ui.connectButton.addEventListener("click", connectSerial);
  ui.disconnectButton.addEventListener("click", disconnectSerial);
  ui.demoButton.addEventListener("click", startDemo);
  ui.clearLogButton.addEventListener("click", () => {
    rawLogLines.length = 0;
    ui.rawLog.textContent = "";
  });

  ui.maxRange.addEventListener("input", () => {
    config.maxRangeM = Number(ui.maxRange.value);
    updateControls();
  });
  ui.leftAngle.addEventListener("input", () => {
    config.leftAngleDeg = Number(ui.leftAngle.value);
    targets.forEach((target) => {
      if (target.directionCode === 1) target.displayAngleDeg = config.leftAngleDeg;
    });
    updateControls();
  });
  ui.rightAngle.addEventListener("input", () => {
    config.rightAngleDeg = Number(ui.rightAngle.value);
    targets.forEach((target) => {
      if (target.directionCode === 2) target.displayAngleDeg = config.rightAngleDeg;
    });
    updateControls();
  });
  ui.useYAxisDistance.addEventListener("change", () => {
    config.useYAxisDistance = ui.useYAxisDistance.checked;
  });

  if ("serial" in navigator) {
    navigator.serial.addEventListener("disconnect", () => {
      state.connected = false;
      state.port = null;
      setBadge(ui.connectionState, "未连接", "bad");
      setStatus("串口设备已移除。");
      updateControls();
    });
  }
}

function init() {
  if ("serial" in navigator) {
    setBadge(ui.serialSupport, "Web Serial 可用", "good");
  } else {
    setBadge(ui.serialSupport, "Web Serial 不可用", "bad");
    setStatus("请使用支持 Web Serial 的 Chrome 或 Edge，并通过 localhost 打开本程序。");
  }

  updateControls();
  bindEvents();
  requestAnimationFrame(draw);
}

init();

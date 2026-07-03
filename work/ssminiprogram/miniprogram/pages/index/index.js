const TARGET_DEVICE_NAME = "BMI270-Backpack";
const NUS_SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";
const NUS_RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";
const NUS_TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

const EMPTY_FRAME = {
  roll: "--",
  pitch: "--",
  yaw: "--",
  speed: "--",
  accelG: "--",
  gyroDps: "--",
  stationary: "--",
  ax: "--",
  ay: "--",
  az: "--",
  gx: "--",
  gy: "--",
  gz: "--",
  alerts: "无",
  lastUpdate: "等待数据",
};

const normalizeUuid = (uuid) => String(uuid || "").toUpperCase();

const isSameUuid = (left, right) => normalizeUuid(left) === normalizeUuid(right);

const clamp = (value, min, max) => Math.max(min, Math.min(max, value));

const pad2 = (value) => {
  const text = "0" + value;
  return text.slice(text.length - 2);
};

const formatTime = (date) => (
  pad2(date.getHours()) + ":" + pad2(date.getMinutes()) + ":" + pad2(date.getSeconds())
);

Page({
  data: {
    targetName: TARGET_DEVICE_NAME,
    adapterReady: false,
    scanning: false,
    connecting: false,
    connected: false,
    statusText: "未连接",
    statusLevel: "idle",
    scanButtonText: "扫描设备",
    deviceId: "",
    deviceName: "",
    deviceLabel: "未连接",
    devices: [],
    hasDevices: false,
    connectingDeviceId: "",
    frame: EMPTY_FRAME,
    frameCount: 0,
    sampleHz: "0",
    commandResponse: "等待命令",
    logs: [],
    horizonStyle: "",
    bodyStyle: "",
  },

  onLoad() {
    this.deviceMap = {};
    this.activeDeviceId = "";
    this.serviceId = "";
    this.txCharacteristicId = "";
    this.rxCharacteristicId = "";
    this.receiveText = "";
    this.sampleTimes = [];

    wx.onBluetoothDeviceFound((res) => this.handleDeviceFound(res));
    wx.onBLECharacteristicValueChange((res) => this.handleCharacteristicValue(res));
    wx.onBLEConnectionStateChange((res) => this.handleConnectionStateChange(res));
    wx.onBluetoothAdapterStateChange((res) => {
      if (!res.available) {
        this.resetConnection("手机蓝牙不可用");
        this.setData({
          adapterReady: false,
          scanning: false,
          scanButtonText: "扫描设备",
          statusLevel: "warn",
        });
      }
    });
  },

  onUnload() {
    this.stopScan();
    if (this.activeDeviceId) {
      wx.closeBLEConnection({ deviceId: this.activeDeviceId });
    }
    if (this.data.adapterReady) {
      wx.closeBluetoothAdapter({});
    }
  },

  openAdapter() {
    if (this.data.adapterReady) {
      return Promise.resolve();
    }
    return new Promise((resolve, reject) => {
      wx.openBluetoothAdapter({
        success: () => {
          this.setData({
            adapterReady: true,
            statusText: "蓝牙已就绪",
            statusLevel: "idle",
          });
          resolve();
        },
        fail: (err) => {
          this.setData({
            statusText: "请打开手机蓝牙和定位权限",
            statusLevel: "warn",
          });
          reject(err);
        },
      });
    });
  },

  startScan() {
    if (this.data.scanning) {
      return;
    }
    this.openAdapter()
      .then(() => {
        this.deviceMap = {};
        this.setData({
          devices: [],
          hasDevices: false,
          scanning: true,
          connecting: false,
          scanButtonText: "扫描中",
          statusText: "正在扫描 BMI270-Backpack",
          statusLevel: "busy",
        });
        wx.startBluetoothDevicesDiscovery({
          allowDuplicatesKey: true,
          services: [NUS_SERVICE_UUID],
          success: () => {},
          fail: (err) => {
            this.setData({
              scanning: false,
              scanButtonText: "重新扫描",
              statusText: "扫描失败：" + (err.errMsg || "未知错误"),
              statusLevel: "warn",
            });
          },
        });
      })
      .catch(() => {});
  },

  stopScan(silent) {
    if (!this.data.scanning) {
      return;
    }
    wx.stopBluetoothDevicesDiscovery({
      complete: () => {
        const patch = {
          scanning: false,
          scanButtonText: "重新扫描",
        };
        if (silent !== true) {
          patch.statusText = this.data.connected ? "已连接" : "已停止扫描";
          patch.statusLevel = this.data.connected ? "connected" : "idle";
        }
        this.setData(patch);
      },
    });
  },

  handleDeviceFound(res) {
    const devices = res.devices || [];
    let changed = false;

    for (let i = 0; i < devices.length; i += 1) {
      const item = devices[i];
      const name = item.name || item.localName || "";
      const serviceList = item.advertisServiceUUIDs || [];
      let hasNusService = false;
      for (let j = 0; j < serviceList.length; j += 1) {
        if (isSameUuid(serviceList[j], NUS_SERVICE_UUID)) {
          hasNusService = true;
          break;
        }
      }

      if (name.indexOf(TARGET_DEVICE_NAME) === -1 && !hasNusService) {
        continue;
      }

      this.deviceMap[item.deviceId] = {
        deviceId: item.deviceId,
        name: name || TARGET_DEVICE_NAME,
        rssi: typeof item.RSSI === "number" ? item.RSSI : -100,
        hasNusService,
      };
      changed = true;
    }

    if (!changed) {
      return;
    }

    const list = Object.keys(this.deviceMap).map((key) => this.deviceMap[key]);
    list.sort((left, right) => {
      const leftName = left.name === TARGET_DEVICE_NAME ? 1 : 0;
      const rightName = right.name === TARGET_DEVICE_NAME ? 1 : 0;
      if (leftName !== rightName) {
        return rightName - leftName;
      }
      return right.rssi - left.rssi;
    });

    this.setData({
      devices: list,
      hasDevices: list.length > 0,
      statusText: "发现 " + list.length + " 个候选设备",
      statusLevel: "busy",
    });
  },

  connectFromTap(e) {
    const deviceId = e.currentTarget.dataset.deviceId;
    const deviceName = e.currentTarget.dataset.deviceName || TARGET_DEVICE_NAME;
    if (!deviceId || this.data.connecting) {
      return;
    }
    this.connectToDevice(deviceId, deviceName);
  },

  connectToDevice(deviceId, deviceName) {
    this.stopScan(true);
    this.receiveText = "";
    this.sampleTimes = [];
    this.setData({
      connecting: true,
      connectingDeviceId: deviceId,
      statusText: "正在连接 " + deviceName,
      statusLevel: "busy",
      deviceLabel: deviceName,
    });

    wx.createBLEConnection({
      deviceId,
      timeout: 10000,
      success: () => {
        this.activeDeviceId = deviceId;
        this.setData({
          deviceId,
          deviceName,
          deviceLabel: deviceName,
          statusText: "发现服务中",
        });
        setTimeout(() => {
          this.setupNus(deviceId, deviceName);
        }, 500);
      },
      fail: (err) => {
        this.setData({
          connecting: false,
          connectingDeviceId: "",
          statusText: "连接失败：" + (err.errMsg || "未知错误"),
          statusLevel: "warn",
        });
      },
    });
  },

  setupNus(deviceId, deviceName) {
    this.getServices(deviceId)
      .then((services) => {
        const service = this.pickService(services);
        if (!service) {
          throw new Error("未找到 NUS 服务");
        }
        this.serviceId = service.uuid;
        return this.getCharacteristics(deviceId, service.uuid);
      })
      .then((characteristics) => {
        const picked = this.pickCharacteristics(characteristics);
        if (!picked.tx) {
          throw new Error("未找到 TX notify 特征");
        }
        if (!picked.rx) {
          throw new Error("未找到 RX write 特征");
        }
        this.txCharacteristicId = picked.tx.uuid;
        this.rxCharacteristicId = picked.rx.uuid;
        return this.enableNotify(deviceId, this.serviceId, this.txCharacteristicId);
      })
      .then(() => {
        this.setData({
          connecting: false,
          connected: true,
          connectingDeviceId: "",
          deviceId,
          deviceName,
          deviceLabel: deviceName,
          statusText: "已连接，等待姿态数据",
          statusLevel: "connected",
          commandResponse: "已订阅 TX notify",
        });
      })
      .catch((err) => {
        wx.closeBLEConnection({ deviceId });
        this.resetConnection("连接未完成：" + err.message);
        this.setData({ statusLevel: "warn" });
      });
  },

  getServices(deviceId) {
    return new Promise((resolve, reject) => {
      wx.getBLEDeviceServices({
        deviceId,
        success: (res) => resolve(res.services || []),
        fail: reject,
      });
    });
  },

  getCharacteristics(deviceId, serviceId) {
    return new Promise((resolve, reject) => {
      wx.getBLEDeviceCharacteristics({
        deviceId,
        serviceId,
        success: (res) => resolve(res.characteristics || []),
        fail: reject,
      });
    });
  },

  enableNotify(deviceId, serviceId, characteristicId) {
    return new Promise((resolve, reject) => {
      wx.notifyBLECharacteristicValueChange({
        deviceId,
        serviceId,
        characteristicId,
        state: true,
        success: resolve,
        fail: reject,
      });
    });
  },

  pickService(services) {
    for (let i = 0; i < services.length; i += 1) {
      if (isSameUuid(services[i].uuid, NUS_SERVICE_UUID)) {
        return services[i];
      }
    }
    return null;
  },

  pickCharacteristics(characteristics) {
    let tx = null;
    let rx = null;
    for (let i = 0; i < characteristics.length; i += 1) {
      const item = characteristics[i];
      const props = item.properties || {};
      if (isSameUuid(item.uuid, NUS_TX_UUID)) {
        tx = item;
      }
      if (isSameUuid(item.uuid, NUS_RX_UUID)) {
        rx = item;
      }
      if (!tx && (props.notify || props.indicate)) {
        tx = item;
      }
      if (!rx && (props.write || props.writeNoResponse)) {
        rx = item;
      }
    }
    return { tx, rx };
  },

  handleCharacteristicValue(res) {
    if (this.activeDeviceId && res.deviceId !== this.activeDeviceId) {
      return;
    }
    if (this.txCharacteristicId && !isSameUuid(res.characteristicId, this.txCharacteristicId)) {
      return;
    }

    this.receiveText += this.arrayBufferToString(res.value);
    const lines = this.receiveText.split("\n");
    this.receiveText = lines.pop() || "";

    for (let i = 0; i < lines.length; i += 1) {
      this.handleIncomingLine(lines[i]);
    }
  },

  handleIncomingLine(line) {
    const text = String(line || "").trim();
    if (!text) {
      return;
    }

    if (text.charAt(0) === "{") {
      try {
        const frame = JSON.parse(text);
        if (typeof frame.r !== "undefined" && typeof frame.p !== "undefined" && typeof frame.y !== "undefined") {
          this.updateFrame(frame);
        } else {
          this.setData({ commandResponse: text });
        }
      } catch (err) {
        this.addLog("JSON 解析失败：" + text.slice(0, 40));
      }
      return;
    }

    this.setData({ commandResponse: text });
    this.addLog(text);
  },

  updateFrame(frame) {
    const roll = this.numberOr(frame.r, 0);
    const pitch = this.numberOr(frame.p, 0);
    const yaw = this.numberOr(frame.y, 0);
    const accel = Array.isArray(frame.a) ? frame.a : [];
    const gyro = Array.isArray(frame.w) ? frame.w : [];
    const now = Date.now();

    this.sampleTimes.push(now);
    while (this.sampleTimes.length && now - this.sampleTimes[0] > 1000) {
      this.sampleTimes.shift();
    }

    const pitchOffset = clamp(pitch * 1.1, -46, 46);
    const rollAngle = clamp(roll, -70, 70);
    const alerts = Array.isArray(frame.al) && frame.al.length ? frame.al.join(", ") : "无";

    this.setData({
      "frame.roll": roll.toFixed(1),
      "frame.pitch": pitch.toFixed(1),
      "frame.yaw": yaw.toFixed(1),
      "frame.speed": this.numberOr(frame.s, 0).toFixed(2),
      "frame.accelG": this.numberOr(frame.ag, 0).toFixed(2),
      "frame.gyroDps": this.numberOr(frame.gyr, 0).toFixed(1),
      "frame.stationary": frame.st ? "静止" : "运动",
      "frame.ax": this.numberOr(accel[0], 0).toFixed(2),
      "frame.ay": this.numberOr(accel[1], 0).toFixed(2),
      "frame.az": this.numberOr(accel[2], 0).toFixed(2),
      "frame.gx": this.numberOr(gyro[0], 0).toFixed(1),
      "frame.gy": this.numberOr(gyro[1], 0).toFixed(1),
      "frame.gz": this.numberOr(gyro[2], 0).toFixed(1),
      "frame.alerts": alerts,
      "frame.lastUpdate": formatTime(new Date(now)),
      frameCount: this.data.frameCount + 1,
      sampleHz: String(this.sampleTimes.length),
      statusText: "姿态数据接收中",
      statusLevel: "connected",
      horizonStyle: "transform: translateY(" + pitchOffset + "rpx) rotateZ(" + rollAngle + "deg);",
      bodyStyle: "transform: rotateZ(" + rollAngle + "deg);",
    });
  },

  sendCommand(e) {
    const command = e.currentTarget.dataset.command;
    this.writeCommand(command);
  },

  writeCommand(command) {
    if (!this.data.connected || !this.activeDeviceId || !this.serviceId || !this.rxCharacteristicId) {
      this.setData({
        commandResponse: "未连接，无法发送 " + command,
      });
      return;
    }

    wx.writeBLECharacteristicValue({
      deviceId: this.activeDeviceId,
      serviceId: this.serviceId,
      characteristicId: this.rxCharacteristicId,
      value: this.stringToArrayBuffer(command + "\n"),
      success: () => {
        this.setData({ commandResponse: "已发送 " + command });
      },
      fail: (err) => {
        this.setData({ commandResponse: "发送失败：" + (err.errMsg || command) });
      },
    });
  },

  disconnectDevice() {
    const deviceId = this.activeDeviceId || this.data.deviceId;
    if (!deviceId) {
      this.resetConnection("未连接");
      return;
    }
    wx.closeBLEConnection({
      deviceId,
      complete: () => {
        this.resetConnection("已断开");
      },
    });
  },

  handleConnectionStateChange(res) {
    if (!this.activeDeviceId || res.deviceId !== this.activeDeviceId) {
      return;
    }
    if (!res.connected) {
      this.resetConnection("连接已断开");
      this.setData({ statusLevel: "warn" });
    }
  },

  resetConnection(statusText) {
    this.activeDeviceId = "";
    this.serviceId = "";
    this.txCharacteristicId = "";
    this.rxCharacteristicId = "";
    this.receiveText = "";
    this.sampleTimes = [];
    this.setData({
      connecting: false,
      connected: false,
      connectingDeviceId: "",
      deviceId: "",
      deviceName: "",
      deviceLabel: "未连接",
      statusText,
      statusLevel: "idle",
      sampleHz: "0",
    });
  },

  addLog(text) {
    const logs = [text].concat(this.data.logs || []);
    this.setData({ logs: logs.slice(0, 4) });
  },

  numberOr(value, fallback) {
    const num = Number(value);
    return Number.isFinite(num) ? num : fallback;
  },

  arrayBufferToString(buffer) {
    const data = new Uint8Array(buffer);
    let text = "";
    for (let i = 0; i < data.length; i += 1) {
      text += String.fromCharCode(data[i]);
    }
    return text;
  },

  stringToArrayBuffer(text) {
    const buffer = new ArrayBuffer(text.length);
    const data = new Uint8Array(buffer);
    for (let i = 0; i < text.length; i += 1) {
      data[i] = text.charCodeAt(i) & 0xff;
    }
    return buffer;
  },
});

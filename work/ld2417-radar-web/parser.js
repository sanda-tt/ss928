(function (root, factory) {
  if (typeof module === "object" && module.exports) {
    module.exports = factory();
  } else {
    root.LD2417 = factory();
  }
})(typeof self !== "undefined" ? self : this, function () {
  "use strict";

  const DATA_HEADER = [0xaa, 0xaa];
  const DATA_TAIL = [0x55, 0x55];
  const TARGET_SIZE = 8;

  function toUint8Array(chunk) {
    if (chunk instanceof Uint8Array) return chunk;
    if (Array.isArray(chunk)) return Uint8Array.from(chunk);
    if (chunk instanceof ArrayBuffer) return new Uint8Array(chunk);
    throw new TypeError("Unsupported chunk type");
  }

  function concatBytes(a, b) {
    const out = new Uint8Array(a.length + b.length);
    out.set(a, 0);
    out.set(b, a.length);
    return out;
  }

  function readUInt16LE(bytes, offset) {
    return bytes[offset] | (bytes[offset + 1] << 8);
  }

  function writeUInt16LE(bytes, offset, value) {
    bytes[offset] = value & 0xff;
    bytes[offset + 1] = (value >> 8) & 0xff;
  }

  function bytesToHex(bytes) {
    return Array.from(bytes, (b) => b.toString(16).padStart(2, "0").toUpperCase()).join(" ");
  }

  function findHeader(buffer, startAt) {
    for (let i = startAt; i <= buffer.length - 2; i += 1) {
      if (buffer[i] === DATA_HEADER[0] && buffer[i + 1] === DATA_HEADER[1]) return i;
    }
    return -1;
  }

  function directionLabel(code) {
    if (code === 1) return "left";
    if (code === 2) return "right";
    return "unknown";
  }

  function parseFrame(raw) {
    const bytes = toUint8Array(raw);
    if (bytes.length < 5) throw new Error("Frame is too short");
    if (bytes[0] !== DATA_HEADER[0] || bytes[1] !== DATA_HEADER[1]) {
      throw new Error("Missing LD2417 data frame header");
    }

    const count = bytes[2];
    const expectedLength = 2 + 1 + count * TARGET_SIZE + 2;
    if (bytes.length !== expectedLength) {
      throw new Error(`Invalid frame length: expected ${expectedLength}, got ${bytes.length}`);
    }
    if (bytes[expectedLength - 2] !== DATA_TAIL[0] || bytes[expectedLength - 1] !== DATA_TAIL[1]) {
      throw new Error("Missing LD2417 data frame tail");
    }

    const targets = [];
    let offset = 3;
    for (let i = 0; i < count; i += 1) {
      const id = bytes[offset];
      const directionCode = bytes[offset + 1];
      const distanceRaw = readUInt16LE(bytes, offset + 2);
      const speedRaw = readUInt16LE(bytes, offset + 4);
      const statusRaw = readUInt16LE(bytes, offset + 6);

      targets.push({
        id,
        directionCode,
        directionLabel: directionLabel(directionCode),
        distanceM: distanceRaw / 100,
        speedKmh: speedRaw / 100,
        statusRaw,
        highSpeed: Boolean(statusRaw & 0x0001),
        associated: Boolean(statusRaw & 0x0002),
        trackAge: (statusRaw >> 8) & 0xff,
        rawHex: bytesToHex(bytes.slice(offset, offset + TARGET_SIZE)),
      });

      offset += TARGET_SIZE;
    }

    return {
      count,
      targets,
      raw: bytes,
      rawHex: bytesToHex(bytes),
    };
  }

  class Parser {
    constructor() {
      this.buffer = new Uint8Array(0);
      this.frames = 0;
      this.droppedBytes = 0;
      this.errors = 0;
    }

    reset() {
      this.buffer = new Uint8Array(0);
      this.frames = 0;
      this.droppedBytes = 0;
      this.errors = 0;
    }

    push(chunk) {
      this.buffer = concatBytes(this.buffer, toUint8Array(chunk));
      const frames = [];

      while (this.buffer.length >= 5) {
        const headerAt = findHeader(this.buffer, 0);
        if (headerAt < 0) {
          this.droppedBytes += Math.max(0, this.buffer.length - 1);
          this.buffer = this.buffer.slice(-1);
          break;
        }

        if (headerAt > 0) {
          this.droppedBytes += headerAt;
          this.buffer = this.buffer.slice(headerAt);
        }

        if (this.buffer.length < 3) break;
        const count = this.buffer[2];
        const frameLength = 2 + 1 + count * TARGET_SIZE + 2;

        if (frameLength > 2048) {
          this.errors += 1;
          this.droppedBytes += 1;
          this.buffer = this.buffer.slice(1);
          continue;
        }

        if (this.buffer.length < frameLength) break;

        if (this.buffer[frameLength - 2] !== DATA_TAIL[0] || this.buffer[frameLength - 1] !== DATA_TAIL[1]) {
          this.errors += 1;
          this.droppedBytes += 1;
          this.buffer = this.buffer.slice(1);
          continue;
        }

        const raw = this.buffer.slice(0, frameLength);
        frames.push(parseFrame(raw));
        this.frames += 1;
        this.buffer = this.buffer.slice(frameLength);
      }

      return frames;
    }
  }

  function makeFrame(targets) {
    const count = targets.length;
    const frameLength = 2 + 1 + count * TARGET_SIZE + 2;
    const bytes = new Uint8Array(frameLength);
    bytes[0] = DATA_HEADER[0];
    bytes[1] = DATA_HEADER[1];
    bytes[2] = count;

    let offset = 3;
    targets.forEach((target, index) => {
      const distanceRaw = Math.max(0, Math.min(65535, Math.round(target.distanceM * 100)));
      const speedRaw = Math.max(0, Math.min(65535, Math.round(target.speedKmh * 100)));
      const statusRaw =
        (target.statusRaw ?? 0) |
        (target.highSpeed ? 0x0001 : 0) |
        (target.associated === false ? 0 : 0x0002) |
        ((target.trackAge ?? index) & 0xff) << 8;

      bytes[offset] = target.id ?? index + 1;
      bytes[offset + 1] = target.directionCode ?? (target.directionLabel === "right" ? 2 : 1);
      writeUInt16LE(bytes, offset + 2, distanceRaw);
      writeUInt16LE(bytes, offset + 4, speedRaw);
      writeUInt16LE(bytes, offset + 6, statusRaw);
      offset += TARGET_SIZE;
    });

    bytes[frameLength - 2] = DATA_TAIL[0];
    bytes[frameLength - 1] = DATA_TAIL[1];
    return bytes;
  }

  return {
    DATA_HEADER,
    DATA_TAIL,
    TARGET_SIZE,
    Parser,
    parseFrame,
    makeFrame,
    bytesToHex,
  };
});


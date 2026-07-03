"use strict";

const assert = require("assert");
const LD2417 = require("./parser.js");

const frame = LD2417.makeFrame([
  {
    id: 1,
    directionCode: 1,
    distanceM: 12.34,
    speedKmh: 56.78,
    highSpeed: false,
    associated: true,
    trackAge: 15,
  },
  {
    id: 2,
    directionCode: 2,
    distanceM: 35,
    speedKmh: 92,
    highSpeed: true,
    associated: true,
    trackAge: 40,
  },
]);

const parser = new LD2417.Parser();
const firstHalf = frame.slice(0, 6);
const secondHalf = frame.slice(6);

assert.strictEqual(parser.push(firstHalf).length, 0);
const parsed = parser.push(secondHalf);

assert.strictEqual(parsed.length, 1);
assert.strictEqual(parsed[0].count, 2);
assert.strictEqual(parsed[0].targets[0].id, 1);
assert.strictEqual(parsed[0].targets[0].directionCode, 1);
assert.strictEqual(parsed[0].targets[0].distanceM, 12.34);
assert.strictEqual(parsed[0].targets[0].speedKmh, 56.78);
assert.strictEqual(parsed[0].targets[0].associated, true);
assert.strictEqual(parsed[0].targets[0].trackAge, 15);
assert.strictEqual(parsed[0].targets[1].id, 2);
assert.strictEqual(parsed[0].targets[1].directionCode, 2);
assert.strictEqual(parsed[0].targets[1].distanceM, 35);
assert.strictEqual(parsed[0].targets[1].speedKmh, 92);
assert.strictEqual(parsed[0].targets[1].highSpeed, true);
assert.strictEqual(parsed[0].targets[1].trackAge, 40);

console.log("parser ok");

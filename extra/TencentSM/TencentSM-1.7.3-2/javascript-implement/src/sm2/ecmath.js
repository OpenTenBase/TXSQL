/* eslint-disable eqeqeq */
/* eslint-disable camelcase */
const { BigInteger } = require('jsbn');
const SM2Curve = require('./curve');
const SM2Point = require('./point');

class ECMath {
  constructor() {
    this.curve = new SM2Curve;
    this.G = new SM2Point('32C4AE2C1F1981195F9904466A39C9948FE30BBFF2660BE1715A4589334C74C7', 'BC3736A2F4F6779C59BDCEE36B692153D0A9877CC62A474002DF32E52139F0A0');
    this.zero = new BigInteger('0');
    this.one = new BigInteger('1');
    this.two = new BigInteger('2');
    this.three = new BigInteger('3');
    this.infinity = new SM2Point(null, null); // 无穷远点
    this.preCompute();
  }
  /**
   * kG预计算
   */
  preCompute() {
    const map = new Map();
    for (let i = 1;i < 256;i++) {
      map.set(`${i}`, this.preComputeMultiplyG(new BigInteger(`${i}`)));
    }
    this.map = map;
  }
  preComputeMultiplyG(k) {
    if (k.equals(this.one)) {
      return this.G;
    }
    let sum = this.G;
    for (let i = k.bitLength() - 2; i >= 0; i--) {
      sum = this.ECPointDoubling(sum);
      const kBit = k.testBit(i);
      if (true == kBit) {
        sum = this.ECPointAdd(sum, this.G);
      }
    }
    return sum;
  }
  /**
   * 相加
   *
   * 标准射影坐标系：
   *
   * λ1 = x1 * z2
   * λ2 = x2 * z1
   * λ3 = λ1 − λ2
   * λ4 = y1 * z2
   * λ5 = y2 * z1
   * λ6 = λ4 − λ5
   * λ7 = λ1 + λ2
   * λ8 = z1 * z2
   * λ9 = λ3^2
   * λ10 = λ3 * λ9
   * λ11 = λ8 * λ6^2 − λ7 * λ9
   * x3 = λ3 * λ11
   * y3 = λ6 * (λ9 * λ1 − λ11) − λ4 * λ10
   * z3 = λ10 * λ8
   */
  ECPointAdd(p, q) {
    const mq = this.curve.p;
    if (p.isInfinity()) {
      return q;
    } if (q.isInfinity()) {
      return p;
    }
    const x1 = p.X;
    const y1 = p.Y;
    const z1 = p.Z;
    const x2 = q.X;
    const y2 = q.Y;
    const z2 = q.Z;

    const w1 = x1.multiply(z2).mod(mq);
    const w2 = x2.multiply(z1).mod(mq);
    const w3 = w1.subtract(w2);
    const w4 = y1.multiply(z2).mod(mq);
    const w5 = y2.multiply(z1).mod(mq);
    const w6 = w4.subtract(w5);

    if (this.zero.equals(w3)) {
      if (this.zero.equals(w6)) {
        return this.ECPointDoubling(p); // p == q，计算自加
      }
      return this.infinity; // p == -q，则返回无穷远点
    }

    const w7 = w1.add(w2);
    const w8 = z1.multiply(z2).mod(mq);
    const w9 = w3.square().mod(mq);
    const w10 = w3.multiply(w9).mod(mq);
    const w11 = w8.multiply(w6.square()).subtract(w7.multiply(w9))
      .mod(mq);

    const x3 = w3.multiply(w11).mod(mq);
    const y3 = w6.multiply(w9.multiply(w1).subtract(w11)).subtract(w4.multiply(w10))
      .mod(mq);
    const z3 = w10.multiply(w8).mod(mq);

    return new SM2Point(x3, y3, z3);
  }

  /**
   * 自加
   *
   * 标准射影坐标系：
   *
   * λ1 = 3 * x1^2 + a * z1^2
   * λ2 = 2 * y1 * z1
   * λ3 = y1^2
   * λ4 = λ3 * x1 * z1
   * λ5 = λ2^2
   * λ6 = λ1^2 − 8 * λ4
   * x3 = λ2 * λ6
   * y3 = λ1 * (4 * λ4 − λ6) − 2 * λ5 * λ3
   * z3 = λ2 * λ5
   */
  ECPointDoubling(p) {
    // console.time('ECPointDoubling');
    if (p.isInfinity()) return p;
    if (!p.Y.signum()) return this.infinity;

    const x1 = p.X;
    const y1 = p.Y;
    const z1 = p.Z;
    const q = this.curve.p;
    const { a } = this.curve;

    const w1 = x1.square().multiply(this.three)
      .add(a.multiply(z1.square()))
      .mod(q);
    const w2 = y1.shiftLeft(1).multiply(z1)
      .mod(q);
    const w3 = y1.square().mod(q);
    const w4 = w3.multiply(x1).multiply(z1)
      .mod(q);
    const w5 = w2.square().mod(q);
    const w6 = w1.square().subtract(w4.shiftLeft(3))
      .mod(q);

    const x3 = w2.multiply(w6).mod(q);
    const y3 = w1.multiply(w4.shiftLeft(2).subtract(w6)).subtract(w5.shiftLeft(1).multiply(w3))
      .mod(q);
    const z3 = w2.multiply(w5).mod(q);
    const point = new SM2Point(x3, y3, z3);
    // console.timeEnd('ECPointDoubling');
    return point;
  }

  ECPointMultiply(p, k) {
    if (k.equals(this.one)) {
      return p;
    }
    let sum = p;
    for (let i = k.bitLength() - 2; i >= 0; i--) {
      sum = this.ECPointDoubling(sum);
      const kBit = k.testBit(i);
      if (true == kBit) {
        sum = this.ECPointAdd(sum, p);
      }
    }
    return sum;
  }

  ECPointMultiplyG(k) {
    if (k.equals(this.one)) {
      return this.G;
    }
    const length = k.bitLength();
    const firstLength = length % 8 === 0 ? 8 : length % 8;
    let sum = this.map.get(k.shiftRight(length - firstLength).toString());

    for (let i = k.bitLength() - firstLength - 1; i > 0; i -= 8) {
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      sum = this.ECPointDoubling(sum);
      let count = 0;
      for (let j = 0;j < 8;j++) {
        count = 2 * count;
        if (k.testBit(i - j)) {
          count += 1;
        }
      }
      if (count !== 0) {
        sum = this.ECPointAdd(sum, this.map.get(`${count}`));
      }
    }
    return sum;
  }

  ECPointIsOnCurve(p) {
    const left = p.Y.multiply(p.Y).mod(this.curve.p);
    const right = p.X.multiply(p.X).multiply(p.X)
      .add(p.X.multiply(this.curve.a))
      .add(this.curve.b)
      .mod(this.curve.p);;
    return left.equals(right);
  }
}

module.exports = ECMath;

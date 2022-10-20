const { BigInteger } = require('jsbn');
const SM2Curve = require('./curve');

class SM2Point {
  constructor(x, y, z) {
    if (typeof x === 'string' && typeof y === 'string') {
      this.X = new BigInteger(x, 16);
      this.Y = new BigInteger(y, 16);
    } else {
      this.X = x;
      this.Y = y;
    }
    // 标准射影坐标系：zinv == null 或 z * zinv == 1
    this.Z = (z === null || z === undefined) ? BigInteger.ONE : z;
    this.zinv = null;
    this.curve = new SM2Curve();
  }
  getX() {
    if (this.zinv === null) this.zinv = this.Z.modInverse(this.curve.p);
    return this.X.multiply(this.zinv).mod(this.curve.p);
  }
  getY() {
    if (this.zinv === null) this.zinv = this.Z.modInverse(this.curve.p);
    return this.Y.multiply(this.zinv).mod(this.curve.p);
  }
  /**
   * 是否是无穷远点
   */
  isInfinity() {
    if ((this.X === null) && (this.Y === null)) return true;
    return this.Z.equals(BigInteger.ZERO) && !this.Y.equals(BigInteger.ZERO);
  }
}

module.exports = SM2Point;

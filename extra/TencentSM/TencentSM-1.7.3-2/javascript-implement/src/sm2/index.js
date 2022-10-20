/* eslint-disable new-cap */
/* eslint-disable eqeqeq */
/* eslint-disable no-plusplus */
/* eslint-disable no-param-reassign */
/* eslint-disable camelcase */
const { BigInteger, SecureRandom } = require('jsbn');
const SM2Curve = require('./curve');
const SM2Point = require('./point');
const SM2Digest = require('./digest');
const SM3 = require('../sm3');
const ECMath = require('./ecmath');
const utils = require('../utils');
const asn = require('asn1.js');
const bn = asn.bignum;
const tool = require('./tool');
const rng = new SecureRandom();

const ASN1_SM2EncryptFormat = asn.define('ASN1_SM2EncryptFormat', function () {
  this.seq().obj(this.key('X').int(), this.key('Y').int(), this.key('Hash').octstr(), this.key('Cipher').octstr());
});

const ASN1_SM2SignFormat = asn.define('ASN1_SM2SignFormat', function () {
  this.seq().obj(this.key('R').int(), this.key('S').int());
});

const ASN1_SM2PublicKeyObjID = asn.define('ASN1_SM2PublicKeyObjID', function () {
  this.seq().obj(
    this.key('algorithm').objid(),
    this.key('subAlgorithm').objid(),
  );
});

const ASN1_SM2PublicKey = asn.define('ASN1_SM2PublicKey', function () {
  this.seq().obj(
    this.key('algorithm').use(ASN1_SM2PublicKeyObjID),
    this.key('key').bitstr());
});

class SM2 {
  constructor() {
    this.curve = new SM2Curve();
    this.ecmath = new ECMath();
    this.sm3 = new SM3();
  }

  encodePublicKeyToDer(strPublicKey){

    const output = ASN1_SM2PublicKey.encode(
      {
        algorithm: {algorithm: '1.2.840.10045.2.1'.split('.'), subAlgorithm: '1.2.156.10197.1.301'.split('.') },
        key: { data: Buffer.from(strPublicKey, 'hex') },
      },
      'der',
    );

    return utils.byteArrayToHex(output);
  }

  decodePublicKeyFromDer(derPublicKey){

    const sm2PublicKey = ASN1_SM2PublicKey.decode(Buffer.from(derPublicKey, 'hex'), 'der');
    return utils.byteArrayToHex(sm2PublicKey.key.data);
  }

  encrypt(publicKey, plain) {
    const k = new BigInteger(this.curve.n.bitLength(), rng)
      .mod(this.curve.n.subtract(BigInteger.ONE))
      .add(BigInteger.ONE);
    const kG = this.ecmath.ECPointMultiplyG(k);
    const kGX = tool.normalize(kG.getX().toByteArray());

    const kGY = tool.normalize(kG.getY().toByteArray());

    // let C1 = kGX.concat(kGY);

    const Pb = new SM2Point(publicKey.substr(2, 64), publicKey.substr(2 + 64, 64));
    const kPb = this.ecmath.ECPointMultiply(Pb, k);

    const kPbX = tool.normalize(kPb.getX().toByteArray());

    const kPbY = tool.normalize(kPb.getY().toByteArray());

    const X2Y2 = kPbX.concat(kPbY);

    const t = SM2Digest.kdf(X2Y2, plain.length);

    let tz = true;
    for (let i = 0; i < t.length; i++) {
      if (t[i] != 0) {
        tz = false;
        break;
      }
    }
    if (tz) {
      return this.encrypt(publicKey, plain);
    }

    const C2 = new Array(plain.length);
    for (let i = 0; i < plain.length; i++) {
      C2[i] = plain[i] ^ t[i];
    }

    const X2MY2 = kPbX.concat(Array.prototype.slice.call(plain)).concat(kPbY);

    this.sm3.init();
    this.sm3.update(X2MY2);
    const C3 = this.sm3.final();

    const output = ASN1_SM2EncryptFormat.encode(
      {
        X: new bn(utils.byteArrayToHex(kGX), 16),
        Y: new bn(utils.byteArrayToHex(kGY), 16),
        Hash: C3,
        Cipher: C2,
      },
      'der',
    );

    return utils.byteArrayToHex(output);
  }

  decrypt(privateKey, cipher) {
    const cipherbuf = Buffer.from(utils.hexToByteArray(cipher));

    const cipherSt = ASN1_SM2EncryptFormat.decode(cipherbuf, 'der');

    const C1 = new SM2Point(cipherSt.X.toString(16), cipherSt.Y.toString(16));
    const dB = new BigInteger(privateKey, 16);
    const dBC1 = this.ecmath.ECPointMultiply(C1, dB);

    const dBC1X = tool.normalize(dBC1.getX().toByteArray());

    const dBC1Y = tool.normalize(dBC1.getY().toByteArray());

    const X2Y2 = dBC1X.concat(dBC1Y);

    const t = SM2Digest.kdf(X2Y2, cipherSt.Cipher.length);

    let tz = true;
    for (let i = 0; i < t.length; i++) {
      if (t[i] != 0) {
        tz = false;
        break;
      }
    }
    if (tz) {
      return -1;
    }

    const M = new Array(cipherSt.Cipher.length);
    for (let i = 0; i < cipherSt.Cipher.length; i++) {
      M[i] = cipherSt.Cipher[i] ^ t[i];
    }

    const X2MY2 = dBC1X.concat(M).concat(dBC1Y);

    this.sm3.init();
    this.sm3.update(X2MY2);
    const u = Buffer.from(this.sm3.final());

    if (u.equals(cipherSt.Hash)) {
      return M;
    }
    return -1;
  }

  sign(publicKey, privateKey, msg, id) {
    if (publicKey.startsWith('0x')) publicKey = publicKey.slice(2);
    if (privateKey.startsWith('0x')) privateKey = privateKey.slice(2);
    const Pb = new SM2Point(publicKey.substr(2, 64), publicKey.substr(2 + 64, 64));

    const dgst = SM2Digest.SM2MessageDigest(msg, id, Pb);

    const k = new BigInteger(this.curve.n.bitLength(), rng)
      .mod(this.curve.n.subtract(BigInteger.ONE))
      .add(BigInteger.ONE);
    const kG = this.ecmath.ECPointMultiplyG(k);

    // const kGX = tool.normalize(kG.X.toByteArray());

    // const kGY = tool.normalize(kG.Y.toByteArray());

    const e = new BigInteger(utils.byteArrayToHex(dgst), 16);

    const r = e.add(kG.getX()).mod(this.curve.n);

    if (r.equals(this.ecmath.zero)) {
      return this.sign(publicKey, privateKey, msg, id);
    }

    const rk = r.add(k);
    if (rk.equals(this.curve.n)) {
      return this.sign(publicKey, privateKey, msg, id);
    }

    const dA = new BigInteger(privateKey, 16);

    const dA_1_invert = dA.add(this.ecmath.one).modInverse(this.curve.n);

    const rdA = r.multiply(dA);

    const s = dA_1_invert.multiply(k.add(rdA.negate())).mod(this.curve.n);

    if (s.equals(this.ecmath.zero)) {
      return this.sign(publicKey, privateKey, msg, id);
    }

    const output = ASN1_SM2SignFormat.encode(
      {
        R: new bn(r.toString(16), 16),
        S: new bn(s.toString(16), 16),
      },
      'der',
    );

    return utils.byteArrayToHex(output);
  }

  verify(publicKey, msg, id, sig) {
    if (publicKey.startsWith('0x')) publicKey = publicKey.slice(2);
    const sigBuf = Buffer.from(utils.hexToByteArray(sig));

    const sigSt = ASN1_SM2SignFormat.decode(sigBuf, 'der');

    // const ss = sigSt.R.toString(16);

    const r = new BigInteger(sigSt.R.toString(16), 16);
    const s = new BigInteger(sigSt.S.toString(16), 16);

    if (!(r.compareTo(this.ecmath.zero) > 0 && r.compareTo(this.curve.n) < 0)) {
      return false;
    }

    if (!(s.compareTo(this.ecmath.zero) > 0 && s.compareTo(this.curve.n) < 0)) {
      return false;
    }

    const t = r.add(s).mod(this.curve.n);
    if (t.equals(this.ecmath.zero)) {
      return false;
    }

    const sG = this.ecmath.ECPointMultiplyG(s);

    const Pa = new SM2Point(publicKey.substr(2, 64), publicKey.substr(2 + 64, 64));

    const tPa = this.ecmath.ECPointMultiply(Pa, t);

    const x1y1 = this.ecmath.ECPointAdd(sG, tPa);

    const dgst = SM2Digest.SM2MessageDigest(msg, id, Pa);

    const e = new BigInteger(utils.byteArrayToHex(dgst), 16);

    const R = e.add(x1y1.getX()).mod(this.curve.n);

    if (R.equals(r)) {
      return true;
    }
    return false;
  }
  /**
   * 生成公私钥
   */
  genKeyPair() {
    const k = new BigInteger(this.curve.n.bitLength(), rng)
      .mod(this.curve.n.subtract(BigInteger.ONE))
      .add(BigInteger.ONE);
    const kG = this.ecmath.ECPointMultiplyG(k);
    const kGX = tool.normalize(kG.getX().toByteArray());
    const kGY = tool.normalize(kG.getY().toByteArray());
    const publicKey = utils.byteArrayToHex([4, ...kGX, ...kGY]);
    return {
      privateKey: tool.leftPad(k.toString(16), 64),
      publicKey,
    };
  }
  /**
   * 计算公钥
   */
  getPublicKeyFromPrivateKey(privateKey) {
    const kG = this.ecmath.ECPointMultiplyG(new BigInteger(privateKey, 16));
    const kGX = tool.normalize(kG.getX().toByteArray());
    const kGY = tool.normalize(kG.getY().toByteArray());
    const publicKey = utils.byteArrayToHex([4, ...kGX, ...kGY]);
    return publicKey;
  }

  /**
   * 密钥生成函数
   * public 外部调用
   * @param {*} share Array
   * @param {*} keylen number
   */
  kdf(share, keylen) {
    return SM2Digest.kdf(share, keylen);
  }
}

module.exports = SM2;

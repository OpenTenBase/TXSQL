/* eslint-disable no-param-reassign */
/* eslint-disable no-plusplus */
const SM3 = require('../sm3');
const SM2Curve = require('./curve');

const sm2p256v1 = new SM2Curve;
const sm3 = new SM3;

const tool = require('./tool');

function SM2GetZ(id, publicKey) {
  const p = new Array(2 + id.length + 32 * 6);

  const idBitLen = id.length * 8;

  p[0] = ((idBitLen >> 8) & 0xff);
  p[1] = idBitLen & 0xff;

  for (let i = 0;i < id.length;i++) {
    p[2 + i] = id[i];
  }

  let offset = 2 + id.length;

  const aArray = tool.normalize(sm2p256v1.a.toByteArray());

  for (let i = 0;i < aArray.length;i++) {
    p[offset + i] = (aArray[i] >>> 0) % 256;
  }
  offset += aArray.length;

  const bArray = tool.normalize(sm2p256v1.b.toByteArray());

  for (let i = 0;i < bArray.length;i++) {
    p[offset + i] = (bArray[i] >>> 0) % 256;
  }
  offset += bArray.length;

  const GxArray = tool.normalize(sm2p256v1.Gx.toByteArray());

  for (let i = 0;i < GxArray.length;i++) {
    p[offset + i] = (GxArray[i] >>> 0) % 256;
  }
  offset += GxArray.length;

  const GyArray = tool.normalize(sm2p256v1.Gy.toByteArray());

  for (let i = 0;i < GyArray.length;i++) {
    p[offset + i] = (GyArray[i] >>> 0) % 256;
  }
  offset += GyArray.length;;

  const XArray = tool.normalize(publicKey.getX().toByteArray());

  for (let i = 0;i < XArray.length;i++) {
    p[offset + i] = (XArray[i] >>> 0) % 256;
  }
  offset += XArray.length;

  const YArray = tool.normalize(publicKey.getY().toByteArray());

  for (let i = 0;i < YArray.length;i++) {
    p[offset + i] = (YArray[i] >>> 0) % 256;
  }
  offset += YArray.length;

  sm3.init();
  sm3.update(p);
  return  sm3.final();
}

function SM2MessageDigest(msg, id, publicKey) {
  const z = SM2GetZ(id, publicKey);

  sm3.init();
  sm3.update(z);
  sm3.update(msg);
  return sm3.final();
}


function kdf(share, keylen) {
  if (keylen > 0xffffffff * 32) {
    return -1;
  }
  if (share.length > 1024) {
    return -1;
  }

  let newCounter = 1;
  let rlen = keylen;
  let dgst;

  const outkey = [];
  let outkeyPtr = 0;

  share.push((newCounter >>> 24) % 256);
  share.push((newCounter >>> 16) % 256);
  share.push((newCounter >>> 8) % 256);
  share.push((newCounter >>> 0) % 256);

  while (rlen > 0) {
    sm3.init();
    sm3.update(share);
    dgst = sm3.final();

    if (rlen <= 32) {
      for (let i = 0; i < rlen; i++) {
        outkey[outkeyPtr + i] = dgst[i];
      }
    } else {
      const len = keylen >= 32 ? 32 : keylen;

      for (let i = 0; i < len; i++) {
        outkey[outkeyPtr + i] = dgst[i];
      }
    }

    rlen -= 32;
    outkeyPtr += 32;
    newCounter++;

    share[share.length - 4] = (newCounter >>> 24) % 256;
    share[share.length - 3] = (newCounter >>> 16) % 256;
    share[share.length - 2] = (newCounter >>> 8) % 256;
    share[share.length - 1] = (newCounter >>> 0) % 256;
  }

  return outkey;
}

module.exports = {
  SM2MessageDigest,
  SM2GetZ,
  kdf,
};

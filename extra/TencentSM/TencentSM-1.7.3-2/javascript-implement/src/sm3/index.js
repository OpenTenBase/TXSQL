/* eslint-disable max-len */
/* eslint-disable prefer-destructuring */
/* eslint-disable no-plusplus */
/* eslint-disable no-param-reassign */
const utils = require('../utils');

const T0 = (0x79cc4519);
const T1 = (0xf3988a32);
const T2 = (0xe7311465);
const T3 = (0xce6228cb);
const T4 = (0x9cc45197);
const T5 = (0x3988a32f);
const T6 = (0x7311465e);
const T7 = (0xe6228cbc);
const T8 = (0xcc451979);
const T9 = (0x988a32f3);
const T10 = (0x311465e7);
const T11 = (0x6228cbce);
const T12 = (0xc451979c);
const T13 = (0x88a32f39);
const T14 = (0x11465e73);
const T15 = (0x228cbce6);
const T16 = (0x9d8a7a87);
const T17 = (0x3b14f50f);
const T18 = (0x7629ea1e);
const T19 = (0xec53d43c);
const T20 = (0xd8a7a879);
const T21 = (0xb14f50f3);
const T22 = (0x629ea1e7);
const T23 = (0xc53d43ce);
const T24 = (0x8a7a879d);
const T25 = (0x14f50f3b);
const T26 = (0x29ea1e76);
const T27 = (0x53d43cec);
const T28 = (0xa7a879d8);
const T29 = (0x4f50f3b1);
const T30 = (0x9ea1e762);
const T31 = (0x3d43cec5);
const T32 = (0x7a879d8a);
const T33 = (0xf50f3b14);
const T34 = (0xea1e7629);
const T35 = (0xd43cec53);
const T36 = (0xa879d8a7);
const T37 = (0x50f3b14f);
const T38 = (0xa1e7629e);
const T39 = (0x43cec53d);
const T40 = (0x879d8a7a);
const T41 = (0x0f3b14f5);
const T42 = (0x1e7629ea);
const T43 = (0x3cec53d4);
const T44 = (0x79d8a7a8);
const T45 = (0xf3b14f50);
const T46 = (0xe7629ea1);
const T47 = (0xcec53d43);
const T48 = (0x9d8a7a87);
const T49 = (0x3b14f50f);
const T50 = (0x7629ea1e);
const T51 = (0xec53d43c);
const T52 = (0xd8a7a879);
const T53 = (0xb14f50f3);
const T54 = (0x629ea1e7);
const T55 = (0xc53d43ce);
const T56 = (0x8a7a879d);
const T57 = (0x14f50f3b);
const T58 = (0x29ea1e76);
const T59 = (0x53d43cec);
const T60 = (0xa7a879d8);
const T61 = (0x4f50f3b1);
const T62 = (0x9ea1e762);
const T63 = (0x3d43cec5);


const Tj = [T0, T1, T2, T3, T4, T5, T6, T7, T8, T9,
  T10, T11, T12, T13, T14, T15, T16, T17, T18, T19,
  T20, T21, T22, T23, T24, T25, T26, T27, T28, T29,
  T30, T31, T32, T33, T34, T35, T36, T37, T38, T39,
  T40, T41, T42, T43, T44, T45, T46, T47, T48, T49,
  T50, T51, T52, T53, T54, T55, T56, T57, T58, T59,
  T60, T61, T62, T63];

const SM3_BLOCK_SIZE = 64;
const W = new Array(68);

function ROTATELEFT(X, n) {
  const r0 = (X) << (n);
  const r1 = (X) >>> (32 - n);
  return r0 | r1;
}

function P0(x) {
  return ((x) ^  ROTATELEFT((x), 9)  ^ ROTATELEFT((x), 17));
}

function P1(x) {
  return ((x) ^  ROTATELEFT((x), 15) ^ ROTATELEFT((x), 23));
}

function FF0(x, y, z) {
  return ((x) ^ (y) ^ (z));
}

function FF1(x, y, z) {
  return (((x) & (y)) | ((x) & (z)) | ((y) & (z)));
}

function GG0(x, y, z) {
  return ((x) ^ (y) ^ (z));
}

function GG1(x, y, z) {
  return (((x) & (y)) | ((~(x)) & (z)));
}

function ONEROUND(i, A, B, C, D, E, F, G, H) {
  let TT2 = ROTATELEFT(A, 12);
  const TT1 = ROTATELEFT(TT2 + E + Tj[i], 7);
  TT2 = TT2 ^ TT1;
  if (i <= 15) {
    D = (D + FF0(A, B, C) + TT2 + (W[i] ^ W[i + 4]));
    H = (H + GG0(E, F, G) + TT1 + W[i]);
  } else {
    D = (D + FF1(A, B, C) + TT2 + (W[i] ^ W[i + 4]));
    H = (H + GG1(E, F, G) + TT1 + W[i]);
  }
  B = ROTATELEFT(B, 9);
  F = ROTATELEFT(F, 19);
  H = P0(H);
  return [B, D, F, H];
}

class SM3 {
  constructor() {
    this.digest = new Array(8);
    this.nblocks = 0;
    this.block = new Array(64);
    this.num = 0;
  }

  init() {
    this.digest[0] = 0x7380166F;
    this.digest[1] = 0x4914B2B9;
    this.digest[2] = 0x172442D7;
    this.digest[3] = 0xDA8A0600;
    this.digest[4] = 0xA96F30BC;
    this.digest[5] = 0x163138AA;
    this.digest[6] = 0xE38DEE4D;
    this.digest[7] = 0xB0FB0E4E;

    for (let i = 0;i < 64;i++) {
      this.block[i] = 0;
    }

    this.nblocks = 0;
    this.num = 0;
  }

  SM3Compress(charBlocks) {
    let i = 0;
    let ret = new Array(4);

    let A = this.digest[0];
    let B = this.digest[1];
    let C = this.digest[2];
    let D = this.digest[3];
    let E = this.digest[4];
    let F = this.digest[5];
    let G = this.digest[6];
    let H = this.digest[7];

    const blocks = new Array(16);

    for (let i = 0;i + 3 < 64;i += 4) {
      blocks[i / 4] = (charBlocks[i] << 24) + (charBlocks[i + 1] << 16) + (charBlocks[i + 2] << 8) + (charBlocks[i + 3]);
    }

    for (i = 0; i < 4; i++) {
      W[i] = (blocks[i]);
    }

    for (i = 0; i <= 8; i = i + 4) {
      W[i + 4] = (blocks[i + 4]);
      ret = ONEROUND(i, A, B, C, D, E, F, G, H);
      B = ret[0];
      D = ret[1];
      F = ret[2];
      H = ret[3];

      W[i + 5] = (blocks[i + 5]);
      ret = ONEROUND(i + 1, D, A, B, C, H, E, F, G);
      A = ret[0];
      C = ret[1];
      E = ret[2];
      G = ret[3];

      W[i + 6] = (blocks[i + 6]);
      ret = ONEROUND(i + 2, C, D, A, B, G, H, E, F);
      D = ret[0];
      B = ret[1];
      H = ret[2];
      F = ret[3];

      W[i + 7] = (blocks[i + 7]);
      ret = ONEROUND(i + 3, B, C, D, A, F, G, H, E);
      C = ret[0];
      A = ret[1];
      G = ret[2];
      E = ret[3];
    }

    for (i = 12; i <= 60; i = i + 4) {
      W[i + 4] = P1(W[i - 12] ^ W[i - 5] ^ ROTATELEFT(W[i + 1], 15)) ^ ROTATELEFT(W[i - 9], 7) ^ W[i - 2];
      ret = ONEROUND(i, A, B, C, D, E, F, G, H);
      B = ret[0];
      D = ret[1];
      F = ret[2];
      H = ret[3];

      W[i + 5] = P1(W[i - 11] ^ W[i - 4] ^ ROTATELEFT(W[i + 2], 15)) ^ ROTATELEFT(W[i - 8], 7) ^ W[i - 1];
      ret = ONEROUND(i + 1, D, A, B, C, H, E, F, G);
      A = ret[0];
      C = ret[1];
      E = ret[2];
      G = ret[3];

      W[i + 6] = P1(W[i - 10] ^ W[i - 3] ^ ROTATELEFT(W[i + 3], 15)) ^ ROTATELEFT(W[i - 7], 7) ^ W[i];
      ret = ONEROUND(i + 2, C, D, A, B, G, H, E, F);
      D = ret[0];
      B = ret[1];
      H = ret[2];
      F = ret[3];

      W[i + 7] = P1(W[i - 9] ^ W[i - 2] ^ ROTATELEFT(W[i + 4], 15)) ^ ROTATELEFT(W[i - 6], 7) ^ W[i + 1];
      ret = ONEROUND(i + 3, B, C, D, A, F, G, H, E);
      C = ret[0];
      A = ret[1];
      G = ret[2];
      E = ret[3];
    }

    this.digest[0] ^= A;
    this.digest[1] ^= B;
    this.digest[2] ^= C;
    this.digest[3] ^= D;
    this.digest[4] ^= E;
    this.digest[5] ^= F;
    this.digest[6] ^= G;
    this.digest[7] ^= H;

    for (let i = 0;i < 8;i++) {
      this.digest[i] = this.digest[i] >>> 0;
    }
  }

  update(data) {
    let datalen = data.length;

    for (let i = 0; i < datalen; i++) {
      data[i] = (data[i] >>> 0) % 256;
    }

    if (this.num) {
      const left = SM3_BLOCK_SIZE - this.num;
      if (datalen < left) {
        for (let i = 0;i < datalen;i++) {
          this.block[this.num + i] = data[i];
        }
        this.num += datalen;
        return;
      }
      for (let i = 0;i < left;i++) {
        this.block[this.num + i] = data[i];
      }
      this.SM3Compress(this.block);
      this.nblocks++;
      data = data.slice(left);
      datalen -= left;
    }
    while (datalen >= SM3_BLOCK_SIZE) {
      this.SM3Compress(data);
      this.nblocks++;
      data = data.slice(SM3_BLOCK_SIZE);
      datalen -= SM3_BLOCK_SIZE;
    }
    this.num = datalen;
    if (datalen) {
      for (let i = 0;i < datalen;i++) {
        this.block[i] = data[i];
      }
    }
  }

  final() {
    this.block[this.num] = 0x80;

    if (this.num + 9 <= SM3_BLOCK_SIZE) {
      for (let i = 0;i < SM3_BLOCK_SIZE - this.num - 9;i++) {
        this.block[this.num + 1 + i] = 0;
      }
    } else {
      for (let i = 0;i < SM3_BLOCK_SIZE - this.num - 1;i++) {
        this.block[this.num + 1 + i] = 0;
      }
      this.SM3Compress(this.block);
      for (let i = 0;i < SM3_BLOCK_SIZE - 8;i++) {
        this.block[i] = 0;
      }
    }


    const count0 = (this.nblocks >>> 23);
    const count1 = ((this.nblocks << 9) + (this.num << 3));

    this.block[SM3_BLOCK_SIZE - 8] = (count0 >>> 24) % 256;
    this.block[SM3_BLOCK_SIZE - 7] = (count0 >>> 16) % 256;
    this.block[SM3_BLOCK_SIZE - 6] = (count0 >>> 8) % 256;
    this.block[SM3_BLOCK_SIZE - 5] = (count0 >>> 0) % 256;

    this.block[SM3_BLOCK_SIZE - 4] = (count1 >>> 24) % 256;
    this.block[SM3_BLOCK_SIZE - 3] = (count1 >>> 16) % 256;
    this.block[SM3_BLOCK_SIZE - 2] = (count1 >>> 8) % 256;
    this.block[SM3_BLOCK_SIZE - 1] = (count1 >>> 0) % 256;

    const digest = new Array(32);

    this.SM3Compress(this.block);

    for (let i = 0;i < 8;i++) {
      digest[i * 4] = (this.digest[i] >>> 24) % 256;
      digest[i * 4 + 1] = (this.digest[i] >>> 16) % 256;
      digest[i * 4 + 2] = (this.digest[i] >>> 8) % 256;
      digest[i * 4 + 3] = (this.digest[i] >>> 0) % 256;
    }

    return digest;
  }

  hashForBinaryArray(data) {
    this.init();
    this.update(data);
    const retBinaryArr = this.final();
    return utils.byteArrayToHex(retBinaryArr);
  }

  hashForUTF8String(str) {
    const binaryArr = utils.stringToByteArrayInUtf8(str);
    return this.hashForBinaryArray(binaryArr);
  }
}

module.exports = SM3;

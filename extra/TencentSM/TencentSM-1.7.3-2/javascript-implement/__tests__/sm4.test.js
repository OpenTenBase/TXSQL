const SM4 = require('../src/sm4');
const base64js = require('base64-js');
const utils = require('../src/utils');

test('encrypt to base64 by cbc', () => {
  const sm4Config = {
    // encrypt/decypt main key; cannot be omitted
    key: 'JeF8U9wHFOMfs2Y8',

    // optional; can be 'cbc' or 'ecb'
    mode: 'cbc',
    iv: 'UISwD9fW6cFh9SNS', // default is null
    // this is the cipher data's type; Can be 'base64', 'text'(means raw string), 'hex'(means hex string)
    cipherType: 'base64', // default is base64
  };
  const sm4 = new SM4(sm4Config);
  expect(sm4.encrypt('中国国密加解密算法')).toBe('j/+HgSpv8RZQI2YtSq0L1RnemiSokMm1VvLHSTt245U=');
});

test('encrypt to raw text by cbc', () => {
  const sm4Config = {
    // encrypt/decypt main key; cannot be omitted
    key: 'JeF8U9wHFOMfs2Y8',

    // optional; can be 'cbc' or 'ecb'
    mode: 'cbc',
    iv: 'UISwD9fW6cFh9SNS', // default is null
    // this is the cipher data's type; Can be 'base64', 'text'(means raw string), 'hex'(means hex string)
    cipherType: 'text', // default is hex
  };
  const sm4 = new SM4(sm4Config);
  expect(sm4.encrypt('中国国密加解密算法')).toBe(utils.utf8ByteArrayToString(base64js.toByteArray('j/+HgSpv8RZQI2YtSq0L1RnemiSokMm1VvLHSTt245U=')));
});

test('encrypt to hex by cbc', () => {
  const sm4Config = {
    // encrypt/decypt main key; cannot be omitted
    key: 'JeF8U9wHFOMfs2Y8',

    // optional; can be 'cbc' or 'ecb'
    mode: 'cbc',
    iv: 'UISwD9fW6cFh9SNS', // default is null
    // this is the cipher data's type; Can be 'base64', 'text'(means raw string), 'hex'(means hex string)
    cipherType: 'hex', // default is hex
  };
  const sm4 = new SM4(sm4Config);
  expect(sm4.encrypt('中国国密加解密算法')).toBe('8fff87812a6ff1165023662d4aad0bd519de9a24a890c9b556f2c7493b76e395');
});

test('encrypt to array text by cbc,output hex', () => {
  const sm4keyArray = utils.stringToByteArrayInUtf8('JeF8U9wHFOMfs2Y8');
  const sm4ivArray = utils.stringToByteArrayInUtf8('UISwD9fW6cFh9SNS');

  const sm4Config = {
  // encrypt/decypt main key; cannot be omitted
    key: sm4keyArray,
    // optional; can be 'cbc' or 'ecb'
    mode: 'cbc',
    iv: sm4ivArray, // default is null
  };

  const sm4 = new SM4(sm4Config);
  const sm4plainArray = utils.stringToByteArrayInUtf8('中国国密加解密算法');

  const sm4Cipher = sm4.encrypt(sm4plainArray);
  const sm4Plain = sm4.decrypt(sm4Cipher);
  expect('中国国密加解密算法').toBe(sm4Plain);
  // console.log(sm4Cipher);
  // console.log(sm4Plain);
});

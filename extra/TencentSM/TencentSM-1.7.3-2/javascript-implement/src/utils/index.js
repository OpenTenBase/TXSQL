/* eslint-disable no-prototype-builtins */
/* eslint-disable no-plusplus */
/* eslint-disable no-param-reassign */
const base64js = require('base64-js');

function textEncode(str) {
  if (typeof window !== 'undefined' && window.TextEncoder) {
    return new window.TextEncoder('utf-8').encode(str);
  }
  const { TextEncoder } = require('./TextEncoderAndDecoder');
  const encoder = new TextEncoder();
  return encoder.encode(str);
}

function textDecode(arr) {
  if (typeof window !== 'undefined' && window.TextDecoder) {
    return new window.TextDecoder('utf-8').decode(arr);
  }
  const { TextDecoder } = require('./TextEncoderAndDecoder');
  const decoder = new TextDecoder();
  return decoder.decode(arr);
}

/**
 * Converts a JS string to an UTF-8 uint8array.
 *
 * @static
 * @param {String} str 16-bit unicode string.
 * @return {Uint8Array} UTF-8 Uint8Array.
 */
function stringToByteArrayInUtf8(str) {
  return textEncode(str);
}

/**
 * Converts an UTF-8 uint8array to a JS string.
 *
 * @static
 * @param {Uint8Array} strByteArray UTF-8 Uint8Array.
 * @return {String} 16-bit unicode string.
 */
function utf8ByteArrayToString(strByteArray) {
  if (strByteArray === -1) return;
  if (Array.isArray(strByteArray)) {
    strByteArray = new Uint8Array(strByteArray);
  }
  return textDecode(strByteArray);
}

function byteArrayToBase64(strByteArray) {
  return base64js.fromByteArray(strByteArray);
}

function base64ToByteArray(base64) {
  return base64js.toByteArray(base64);
}

/**
 * 转成ascii码数组
 */
function hexToByteArray(strHex) {
  if (typeof strHex !== 'string') {
    throw new TypeError('Expected input to be a string');
  }

  if ((strHex.length % 2) !== 0) {
    throw new RangeError('Expected string to be an even number of characters');
  }

  const view = new Uint8Array(strHex.length / 2);

  for (let i = 0; i < strHex.length; i += 2) {
    view[i / 2] = parseInt(strHex.substring(i, i + 2), 16);
  }

  return view;
}

/**
 * 转成16进制串
 */
function byteArrayToHex(arr) {
  for (let i = 0; i < arr.length; i++) {
    arr[i] = (arr[i] >>> 0) % 256;
  }

  const words = [];
  let j = 0;
  for (let i = 0; i < arr.length * 2; i += 2) {
    words[i >>> 3] |= parseInt(arr[j], 10) << (24 - (i % 8) * 4);
    j++;
  }

  // 转换到16进制
  const hexChars = [];
  for (let i = 0; i < arr.length; i++) {
    const bite = (words[i >>> 2] >>> (24 - (i % 4) * 8)) & 0xff;
    hexChars.push((bite >>> 4).toString(16));
    hexChars.push((bite & 0x0f).toString(16));
  }

  return hexChars.join('');
  // return Array.prototype.map.call(strByteArray, x => ('00' + x.toString(16)).slice(-2)).join('');
}

module.exports = {
  stringToByteArrayInUtf8,
  utf8ByteArrayToString,
  byteArrayToBase64,
  base64ToByteArray,
  byteArrayToHex,
  hexToByteArray,
};


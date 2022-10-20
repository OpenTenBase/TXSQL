function TextEncoder() {
}

TextEncoder.prototype.encode = function (string) {
  const octets = [];
  const { length } = string;
  let i = 0;
  while (i < length) {
    const codePoint = string.codePointAt(i);
    let c = 0;
    let bits = 0;
    if (codePoint <= 0x0000007F) {
      c = 0;
      bits = 0x00;
    } else if (codePoint <= 0x000007FF) {
      c = 6;
      bits = 0xC0;
    } else if (codePoint <= 0x0000FFFF) {
      c = 12;
      bits = 0xE0;
    } else if (codePoint <= 0x001FFFFF) {
      c = 18;
      bits = 0xF0;
    }
    octets.push(bits | (codePoint >> c));
    c -= 6;
    while (c >= 0) {
      octets.push(0x80 | ((codePoint >> c) & 0x3F));
      c -= 6;
    }
    i += codePoint >= 0x10000 ? 2 : 1;
  }
  return octets;
};

function TextDecoder() {
}

TextDecoder.prototype.decode = function (octets) {
  let string = '';
  let i = 0;
  while (i < octets.length) {
    let octet = octets[i];
    let bytesNeeded = 0;
    let codePoint = 0;
    if (octet <= 0x7F) {
      bytesNeeded = 0;
      codePoint = octet & 0xFF;
    } else if (octet <= 0xDF) {
      bytesNeeded = 1;
      codePoint = octet & 0x1F;
    } else if (octet <= 0xEF) {
      bytesNeeded = 2;
      codePoint = octet & 0x0F;
    } else if (octet <= 0xF4) {
      bytesNeeded = 3;
      codePoint = octet & 0x07;
    }
    if (octets.length - i - bytesNeeded > 0) {
      let k = 0;
      while (k < bytesNeeded) {
        octet = octets[i + k + 1];
        codePoint = (codePoint << 6) | (octet & 0x3F);
        k += 1;
      }
    } else {
      codePoint = 0xFFFD;
      bytesNeeded = octets.length - i;
    }
    string += String.fromCodePoint(codePoint);
    i += bytesNeeded + 1;
  }
  return string;
};

module.exports = { TextDecoder, TextEncoder };

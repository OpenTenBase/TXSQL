function normalize(array) {
  let result = [...array];
  if (result.length > 32) {
    result = result.slice(result.length - 32, result.length);
  } else if (result.length < 32) {
    for (let i = 0; i < 32 - result.length; i++) {
      result.unshift(0);
    }
  }
  return result;
}
/**
 * 补全16进制字符串
 */
function leftPad(input, num) {
  if (input.length >= num) return input;
  return (new Array(num - input.length + 1)).join('0') + input;
}

module.exports = {
  normalize,
  leftPad,
};


import resolve from 'rollup-plugin-node-resolve';
import commonjs from 'rollup-plugin-commonjs';
const path = require('path');

const resolveFile = function (filePath) {
  return path.join(__dirname, '..', filePath);
};
module.exports = [
  {
    input: resolveFile('build/sm2.build.js'),
    output: {
      file: resolveFile('dist/sm2.js'),
      format: 'umd',
      name: 'SM2Lib',
    },
    plugins: [commonjs(), resolve()],
  },
  {
    input: resolveFile('build/sm3.build.js'),
    output: {
      file: resolveFile('dist/sm3.js'),
      format: 'umd',
      name: 'SM3Lib',
    },
    plugins: [commonjs(), resolve()],
  },
  {
    input: resolveFile('build/sm4.build.js'),
    output: {
      file: resolveFile('dist/sm4.js'),
      format: 'umd',
      name: 'SM4Lib',
    },
    plugins: [commonjs(), resolve()],
  },
  // {
  //   input: resolveFile('src/utils/index.js'),
  //   output: {
  //     file: resolveFile('dist/utils.js'),
  //     format: 'umd',
  //     name: 'util',
  //   },
  //   plugins: [commonjs(), resolve()],
  // },
];

const path = require('path');
const TerserPlugin = require('terser-webpack-plugin');
const { target } = process.env;
const config = {
  mode: 'production',
  entry: {
    SM4: path.join(__dirname, '../build/sm4.build.js'), // 入口文件
    SM2: path.join(__dirname, '../build/sm2.build.js'),
    SM3: path.join(__dirname, '../build/sm3.build.js'),
    SM: path.join(__dirname, '../build/sm.build.js'),
  },
  output: {
    filename: '[name].js', // 打包后输出文件的文件设置为btn.js
  },
  optimization: {
    minimize: true,
    minimizer: [
      new TerserPlugin(),
    ],
  },
  module: {
  },
};
if (!target) {
  config.output.library = '[name]Lib';
  config.output.path =  path.join(__dirname, '../dist'); // 打包后的文件存放在dist/commonjs文件夹
  config.output.libraryTarget = 'umd';
} else {
  config.output.path =  path.join(__dirname, '../dist/commonjs'); // 打包后的文件存放在dist/commonjs文件夹
  config.output.libraryTarget = target;
}
module.exports = config;

import resolve from 'rollup-plugin-node-resolve';
import commonjs from 'rollup-plugin-commonjs';
import { uglify } from 'rollup-plugin-uglify';

export default {
  input: 'src/sm3/index.js',
  output: {
    file: 'dist/sm3.bundle..min.js',
    format: 'umd',
    name: 'sm3',
  },
  watch: {
    exclude: 'node_modules/**',
  },
  plugins: [commonjs(), resolve(), uglify()],
};

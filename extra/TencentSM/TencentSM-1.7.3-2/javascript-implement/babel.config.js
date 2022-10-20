module.exports = {
  presets: [
    [
      '@babel/env',
      {
        targets: { node: 'current' },
        modules: false,
      },
    ],
  ],
  plugins: ['@babel/plugin-external-helpers'],
};

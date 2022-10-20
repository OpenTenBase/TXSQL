module.exports = {
  root: true,
  env: {
    node: true,
    jest: true,
  },
  extends: ['@tencent/eslint-config-tencent'],
  rules: {
    'no-console': 'off',
    'no-debugger': 'off',
    'no-param-reassign': [
      'error',
      {
        props: true,
        ignorePropertyModificationsFor: [
          'state', // for vuex state
          'acc', // for reduce accumulators
          'e', // for e.returnvalue
        ],
      },
    ],
    'max-len': [
      'error',
      {
        code: 120,
        ignoreUrls: true,
        ignoreStrings: true,
        ignoreTemplateLiterals: true,
        ignoreRegExpLiterals: true,
        ignoreTrailingComments: true,
      },
    ],
  },
};

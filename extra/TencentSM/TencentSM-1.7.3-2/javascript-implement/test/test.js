/* eslint-disable no-unused-vars */
const SMLib = require('../dist/commonjs/SM');
const { SM2 } = SMLib;
console.log(SM2);
const { utils } = SMLib;
const sm2 = new SM2();
const msgByteArray = utils.stringToByteArrayInUtf8('Navigate your code with ease. Click on function and method calls to jump to their definitions or references in the same repository.');
const idByteArray = utils.stringToByteArrayInUtf8('1234567812345678');

const cipher = sm2.encrypt(
  '0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13',
  msgByteArray,
);
const plainByteArray = sm2.decrypt('3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8', cipher);

const sign = sm2.sign(
  '041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD',
  '3B03B35C2F26DBC56F6D33677F1B28AF15E45FE9B594A6426BDCAD4A69FF976B',
  msgByteArray,
  idByteArray,
);
console.log(sign);

const ret = sm2.verify(
  '041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD',
  msgByteArray,
  idByteArray,
  sign,
);
console.log(ret);
const keyPair = sm2.genKeyPair();
console.log(keyPair);

// sm3
const { SM3 } = SMLib;
const sm3 = new SM3();
const begin = new Date().getTime();
for (let i = 0; i < 1000; i++) {
  const result = sm3.hashForUTF8String('This website stores cookies on your computer. These cookies are used to collect information about how you interact with our website and allow us to remember you. We use this information in order to improve and customize your browsing experience and for analytics and metrics about our visitors both on this website and other media. To find out more about the cookies we use, see our Privacy Policy.');
}
const end = new Date().getTime();
const diff = end - begin;
const performance = 1000.0 / (diff / 1000.0);
console.log(performance);

const { SM4 } = SMLib;
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
console.log('sm4Cipher', sm4Cipher);
console.log('sm4Plain', sm4Plain);

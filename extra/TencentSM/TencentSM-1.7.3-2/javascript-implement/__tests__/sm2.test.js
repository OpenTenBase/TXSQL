const SM2 = require('../src/sm2');
const utils = require('../src/utils');
const plainText = `Navigate your code with ease. Click on function
and method calls to jump to their definitions or references in the same repository.`;
const msgByteArray = utils.stringToByteArrayInUtf8(plainText);
const idByteArray = utils.stringToByteArrayInUtf8('1234567812345678');

test('genKeyPair publickey equals publicKeyFromPrivateKey', () => {
  const sm2 = new SM2();
  // 生成公私密钥对
  const keypair = sm2.genKeyPair();
  // 根据私钥生成公钥
  const publicKeyFromPrivateKey = sm2.getPublicKeyFromPrivateKey(keypair.privateKey);
  expect(keypair.publicKey).toBe(publicKeyFromPrivateKey);
});

test('pressure test', () => {
  const sm2 = new SM2();
  // 压力测试则将index最大值改为10000
  for (let index = 0; index < 1; index++) {
    const keypair = sm2.genKeyPair();
    const cipher = sm2.encrypt(keypair.publicKey, msgByteArray);
    const plainByteArray = sm2.decrypt(keypair.privateKey, cipher);
    expect(utils.utf8ByteArrayToString(plainByteArray)).toBe(plainText);
    if (utils.utf8ByteArrayToString(plainByteArray) !== plainText) {
      throw new Error('no equal');
    }

    const sign = sm2.sign(
      '041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD',
      '3B03B35C2F26DBC56F6D33677F1B28AF15E45FE9B594A6426BDCAD4A69FF976B',
      msgByteArray,
      idByteArray,
    );

    const ret = sm2.verify(
      '041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD',
      msgByteArray,
      idByteArray,
      sign,
    );

    const derPub = sm2.encodePublicKeyToDer('041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD');
    console.log('encode sm2 public key result:');
    console.log(derPub);

    const base64DerPub = 'MFkwEwYHKoZIzj0CAQYIKoEcz1UBgi0DQgAERovwvXRw85YoQHSkrquLCRpNp/P+XDSyp9EIlH1yoEyFLSx9BWdE2ab+kC9gVkVHE1LKW9gS6o0xsvyGsztviA==';

    const strPub = sm2.decodePublicKeyFromDer(utils.byteArrayToHex(utils.base64ToByteArray(base64DerPub)));
    console.log('decode sm2 public key result:');
    console.log(strPub);

    const strPub2 = sm2.decodePublicKeyFromDer(derPub);
    console.log(strPub2);

    if (!ret) {
      throw new Error('verify fail');
    }
  }
});

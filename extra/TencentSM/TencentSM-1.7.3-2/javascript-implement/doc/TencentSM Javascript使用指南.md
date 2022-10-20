## TencentSM Javascript使用指南

### SM2

#### 1. 下载并引入sm2.js

```
<script src="sm2.js"></script>
```

#### 2. api介绍

sm2.js中包含算法库``SM2``，及工具库``utils``。

##### SM2

- 初始化sm2对象:

```

const sm2 = new SM2Lib.SM2;
```

- 加密函数``encrypt``:
  - 入参
    - 公钥          `类型`:16进制字符串
    - Utf8格式明文  `类型`:string
  - 返回值
    - 密文，`类型`:16进制字符串

> ``encrypt``: sm2.encrypt(publicKey,sm2plainArray)

- 解密函数``decrypt``:
  - 入参
    - 私钥   `类型`:16进制字符串
    - 密文   `类型`:16进制字符串
  - 返回值
    - 明文 `类型`:ByteArray，可通过`util`中提供的方法进行转换

> ``decrypt``: sm2.decrypt(privateKey,sm2Cipher)

- 签名函数``sign``:
  - 入参
    - 公钥        `类型`:16进制字符串
    - 私钥        `类型`:16进制字符串
    - 待签名消息   `类型`:ByteArray
    - id          `类型`:ByteArray
  - 返回值
    - 签名结果， `类型`:16进制字符串

> ``sign``: sm2.sign(publicKey,privateKey,msg,id)

- 验签函数``verify``:
  - 入参
    - 公钥        `类型`:16进制字符串
    - 原消息      `类型`:16进制字符串
    - id          `类型`:ByteArray
    - 待验签消息   `类型`:ByteArray
  - 返回值
    - 验签结果 `类型`:boolean

> ``verify``: sm2.verify(publickey,msg,id,signmsg)

- 生成公私钥函数``genKeyPair``:
  - 返回值
    - 公私钥{privateKey,publicKey},privateKey和publicKey均为16进制字符串
  
 > ``genKeyPair``: sm2.genKeyPair()

- 根据私钥生成公钥``getPublicKeyFromPrivateKey``
  - 入参
    - 私钥  `类型`:16进制字符串
  - 返回值
    - 公钥 `类型`:16进制字符串

> ``getPublicKeyFromPrivateKey``: sm2.getPublicKeyFromPrivateKey(PrivateKey)

- 密钥生成函数``kdf``
  - 入参
    - share    `类型`:Array
    - keyLen   `类型`:Number,密钥长度，字节数量
  - 返回值
    - 共享密钥  `类型`:Array

> ``kdf``: sm2.kdf(share,keyLen)

##### utils

- stringToByteArrayInUtf8: 字符串转换成``utf-8``
- utf8ByteArrayToString: ``utf-8``转换成字符串
- byteArrayToBase64: ascii码数组转换成``base64``
- base64ToByteArray: ``base64``转换成ascii码数组
- byteArrayToHex: ascii码数组转成``16进制串``
- hexToByteArray: ``16进制串``转成ascii码数组

#### 3. 使用示例

```js
   /* SM2调用示例 */

    const sm2 = new SM2Lib.SM2;
    let msgByteArray = SM2Lib.utils.stringToByteArrayInUtf8('Navigate your code with ease. Click on function and method calls to jump to their definitions or references in the same repository.');
    let idByteArray = SM2Lib.utils.stringToByteArrayInUtf8('1234567812345678');

    let cipher = sm2.encrypt('0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13',msgByteArray);
    console.log('密文:',cipher)
    let plainByteArray = sm2.decrypt('3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8',cipher);
    console.log('明文:',SM2Lib.utils.utf8ByteArrayToString(plainByteArray))
    let sign = sm2.sign('041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD','3B03B35C2F26DBC56F6D33677F1B28AF15E45FE9B594A6426BDCAD4A69FF976B',msgByteArray,idByteArray);
    console.log('签名:',sign);

    let ret = sm2.verify('041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD',msgByteArray,idByteArray,sign);
    console.log(ret);

    // 生成公私密钥对
    const keypair = sm2.genKeyPair();
    console.log(keypair);

```

### SM3

#### 1. 下载并引入sm3.js

```
<script src="sm3.js"></script>
```

#### 2. api介绍

sm3.js中包含算法库``SM3``，及工具库``utils``。

##### SM3

- 初始化sm3对象:

```

const sm3 = new SM3Lib.SM3;
```

- hashForUTF8String函数
  - 入参
    - str       `类型`:String
  - 返回值
    - hash值    `类型`:16进制字符串

> ``hashForUTF8String函数``: sm3.hashForUTF8String(str)

```
let result = sm3.hashForUTF8String('This website stores cookies on your computer. These cookies are used to collect information about how you interact with our website and allow us to remember you. We use this information in order to improve and customize your browsing experience and for analytics and metrics about our visitors both on this website and other media. To find out more about the cookies we use, see our Privacy Policy.');
```

##### utils

- stringToByteArrayInUtf8: 字符串转换成``utf-8``
- utf8ByteArrayToString: ``utf-8``转换成字符串
- byteArrayToBase64: ascii码数组转换成``base64``
- base64ToByteArray: ``base64``转换成ascii码数组
- byteArrayToHex: ascii码数组转成``16进制串``
- hexToByteArray: ``16进制串``转成ascii码数组

#### 3. 使用示例

```
const sm3 = new SM3;
let result = sm3.hashForUTF8String('This website stores cookies on your computer. These cookies are used to collect information about how you interact with our website and allow us to remember you. We use this information in order to improve and customize your browsing experience and for analytics and metrics about our visitors both on this website and other media. To find out more about the cookies we use, see our Privacy Policy.');
console.log(result);
```

### SM4

#### 1. 下载并引入sm4.js

```
<script src="sm4.js"></script>
```

#### 2. api介绍

sm4.js中包含算法库``SM4``，及工具库``utils``。

##### SM4

- 初始化配置``config``:

```
let sm4Config = {
    key: sm4keyArray, // 密钥，不可省略  类型:Array
    mode: 'cbc',      // 模式，可选，'cbc' 或 'ecb'
    iv: sm4ivArray,   // CBC模式的初始化向量，需要选取一个随机数，长度为128bit，与key的长度一致, 默认为null 类型:Array
}
let sm4 = new SM4(sm4Config);
```

- 加密函数``encrypt``:
  - 入参
    - sm4plainArray  `类型`:ByteArray
  - 返回值
    - 密文 `类型`:默认16进制字符串，可通过sm4Config中的cipherType字段配置，可选text、hex、base64

> ``encrypt``: sm4.encrypt(sm4plainArray)

- 解密函数``decrypt``:
  - 入参
    - sm4Cipher  `类型`:默认16进制字符串,可通过sm4Config中的cipherType字段配置，可选text、hex、base64
  - 返回值
    - 明文 `类型`:String

> ``decrypt``: sm4.decrypt(sm4Cipher)

##### utils

- stringToByteArrayInUtf8: 字符串转换成``utf-8``
- utf8ByteArrayToString: ``utf-8``转换成字符串
- byteArrayToBase64: ascii码数组转换成``base64``
- base64ToByteArray: ``base64``转换成ascii码数组
- byteArrayToHex: ascii码数组转成``16进制串``
- hexToByteArray: ``16进制串``转成ascii码数组

#### 3. 使用示例

```
    let SM4 = window.SM4Lib.SM4
    let utils = window.SM4Lib.utils
    let sm4keyArray = utils.stringToByteArrayInUtf8('JeF8U9wHFOMfs2Y8');
    let sm4ivArray = utils.stringToByteArrayInUtf8('UISwD9fW6cFh9SNS');

    let sm4Config = {
        // encrypt/decypt main key; cannot be omitted
        key: sm4keyArray,
        // optional; can be 'cbc' or 'ecb'
        mode: 'cbc',
        iv: sm4ivArray, // default is null
    }

    let sm4 = new SM4(sm4Config);
    let sm4plainArray = utils.stringToByteArrayInUtf8('中国国密加解密算法');

    let sm4Cipher = sm4.encrypt(sm4plainArray);
    let sm4Plain = sm4.decrypt(sm4Cipher);
    console.log('sm4Cipher', sm4Cipher)
    console.log('sm4Plain', sm4Plain)
```

# TencentSM javascript 实现

> TencentSM javascript 支持 SM2 密钥对生成、根据私钥生成公钥、SM2 加解密、SM2 签名验签、SM3 哈希算法、SM4 对称加解密(CBC)功能。

## 目录结构

```
javascript-implement
│   README.md                       //帮助文档
│
└───build                           //打包导入文件
│
└───config                          //构建配置文件
│
└───doc                             //其他帮助文档
│
└───example                         //构建打包后的使用示例
│
└───test                            //测试脚本
│
└───dist                            //构建文件输出目录
│
└───src                             //源代码目录
│   │
│   └───sm2                          //sm2实现源码目录
│   │
│   └───sm3                          //sm3实现源码目录
│   │
│   └───sm4                          //sm4实现源码目录
│   │
│   └───utils                        //工具函数目录
```

## 如何构建

1. 构建符合 umd 标准的库(用于web端)

- npm install
  安装依赖，注意需要使用公司内部软件源（npm config set registry <https://mirrors.tencent.com/npm/>）
- npm run build:web
  构建文件输出到 dist 目录下

1. 构建符合 commonjs 标准的库(用于nodejs及小程序端)

- npm install
  安装依赖，注意需要使用公司内部软件源（npm config set registry <https://mirrors.tencent.com/npm/>）
- npm run build:commonjs
  构建文件输出到 dist/commonjs 目录下

## 如何使用

### 使用npm包

1、注册
```npm i @tencent/tencentsm --save```
2、使用示例

``` javascript
const SMLib = require('@tencent/tencentsm');

// sm2使用示例
const { SM2 } = SMLib;
const { utils } = SMLib;
const sm2 = new SM2;
const msgByteArray = utils.stringToByteArrayInUtf8('Navigate your code with ease. Click on function and method calls to jump to their definitions or references in the same repository.');
const idByteArray = utils.stringToByteArrayInUtf8('1234567812345678');
const cipher = sm2.encrypt('0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13', msgByteArray);
const plainByteArray = sm2.decrypt('3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8', cipher);

const sign = sm2.sign('041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD', '3B03B35C2F26DBC56F6D33677F1B28AF15E45FE9B594A6426BDCAD4A69FF976B', msgByteArray, idByteArray);

const ret = sm2.verify('041D9E2952A06C913BAD21CCC358905ADB3A8097DB6F2F87EB5F393284EC2B7208C30B4D9834D0120216D6F1A73164FDA11A87B0A053F63D992BFB0E4FC1C5D9AD', msgByteArray, idByteArray, sign);

// 生成公私钥
const keyPair = sm2.genKeyPair();

// sm3使用示例
const { SM3 } = SMLib;
const sm3 = new SM3;
const begin = new Date().getTime();
// 性能测试
for (let i = 0;i < 1000;i++) {
  const result = sm3.hashForUTF8String('This website stores cookies on your computer. These cookies are used to collect information about how you interact with our website and allow us to remember you. We use this information in order to improve and customize your browsing experience and for analytics and metrics about our visitors both on this website and other media. To find out more about the cookies we use, see our Privacy Policy.');
}
const end = new Date().getTime();
const diff = end - begin;
const performance = 1000.0 / (diff / 1000.0);

// sm4使用示例
const { SM4 } = SMLib;
const sm4keyArray = utils.stringToByteArrayInUtf8('JeF8U9wHFOMfs2Y8');
const sm4ivArray = utils.stringToByteArrayInUtf8('UISwD9fW6cFh9SNS');

const sm4Config = {
  // encrypt/decypt main key; cannot be omitted
  key: sm4keyArray,
  // optional; can be 'cbc' or 'ecb'; default is cbc
  mode: 'cbc',
  iv: sm4ivArray, // default is null
};// padding mode:PKCS7

const sm4 = new SM4(sm4Config);
const sm4plainArray = utils.stringToByteArrayInUtf8('中国国密加解密算法');

const sm4Cipher = sm4.encrypt(sm4plainArray);
const sm4Plain = sm4.decrypt(sm4Cipher);
```

### 使用构建包压缩资源

详情请参考doc/TencentSM Javascript使用指南.md文档

## API介绍

### sm2

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

### sm3

- hashForUTF8String函数
  - 入参
    - str       `类型`:String
  - 返回值
    - hash值    `类型`:16进制字符串

> ``hashForUTF8String函数``: sm3.hashForUTF8String(str)

```
let result = sm3.hashForUTF8String('This website stores cookies on your computer. These cookies are used to collect information about how you interact with our website and allow us to remember you. We use this information in order to improve and customize your browsing experience and for analytics and metrics about our visitors both on this website and other media. To find out more about the cookies we use, see our Privacy Policy.');
```

### sm4

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

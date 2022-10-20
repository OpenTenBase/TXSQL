# <center>腾讯智能移动终端安全密码模块<br>Android接口说明文档</center>

## 产品概述

腾讯智能移动终端安全密码模块（以下简称模块）为腾讯自研的符合国密算法标准的密码模块，具备SM2、SM3、SM4算法功能，以及在移动终端的密钥管理功能。模块在算法性能上进行了大量的工程优化实践，具有优秀的算法性能。在安全性方面模块也进行了大量的优化创新，模块随机数生成器的熵源采集结合了智能移动终端的多传感器特性，后处理模块引入了SM3算法来替代SHA1和MD5算法，从而提升了随机数质量。而在密钥管理方面，模块的多因子联合策略，也解决了长期困扰业界的移动端如何单独安全地保存密钥的问题。

## 接口说明

腾讯智能移动终端安全密码模块可集成于Android移动终端应用。Native到java的接口类型为JNI接口。分为java部分和native部分，java部分包名为com.tenpay.utils，其中密码功能接口文件为SMUtils.java，秘钥管理接口文件包括SMKeyManager.java和SMKeyManUtils.java；native部分以动态链接库so文件形式给出，支持'armeabi', 'armeabi-v7a', 'arm64-v8a'三种ABI类型。以下针对各个接口进行解释说明。

## 密码功能接口
密码功能接口定义于SMUtils.java文件中。其中SM2非对称加密算法初始化接口会返回一个Handler，其它SM2接口会依赖此Handler作为输入参数，使用完后需要调用SM2的free接口销毁Handler。
***
> ###static SMUtils getInstance(Context context)；

##### 说明：

- 该接口获得密码功能接口类SMUtils的单例对象，如需使用密码功能其他接口(如SM2Encrypt)需首先调用该接口以获得单例对象。

##### 返回值:

- 返回密码功能接口类SMUtils的单例对象

##### 示例：
<pre>
SMUtils instance = SMUtils.getInstance(this);
</pre>

***
> ###long    SM2InitCtx();

##### 说明：

- 该接口是SM2算法的初始化接口，返回的handler，其它接口使用。

##### 返回值:

- 返回handler

##### 示例：
<pre>
long sm2Handler = SMUtils.getInstance(this).SM2InitCtx();
if(sm2Handler != 0) {
}
</pre>

***
> ###long    SM2InitCtxWithPubKey(String strPubKey);

##### 说明：

- 该接口输入一个参数strPubKey：公钥，初始化SM2算法，，返回的handler，其它接口使用。
- 调用该接口后，使用该公钥进行的SM2加密以及SM2验签将获得较大的性能提升。
- 调用该接口以后，仍然可以使用其他的公钥进行SM2密码运算。
##### 参数：
- strPubKey：公钥，SM2生成秘钥对接口生成
##### 返回值:

- 返回handler

##### 示例：
<pre>
long newContext = SMUtils.getInstance(this).SM2InitCtxWithPubKey(pubkey);
if(sm2Handler != 0) {
}
</pre>

***
> ###void     SM2FreeCtx(long sm2Handler);

##### 说明：

- 该接口销毁SM2算法的句柄
##### 参数：
- sm2Handler：SM2算法句柄
##### 返回值:
- void
##### 示例：
<pre>
SMUtils.getInstance(this).SM2FreeCtx(sm2Handler);
</pre>

***
> ###Object[]   SM2GenKeyPair(long sm2Handler);

##### 说明：

- 该接口产生SM2的公私钥对。
##### 参数：
- sm2Handler：SM2算法句柄
##### 返回值:

- 返回数组：array[0]-- privatekey str  array[1]-- publickey str

##### 示例：
<pre>
 Object[] keypairs = SMUtils.getInstance(this).SM2GenKeyPair(sm2Handler);
if(keypairs != null) {
}
</pre>

***
> ###byte[]     SM2Encrypt(long sm2Handler, byte[] in, String strPubKey);

##### 说明：

- SM2加密接口，加密后的密文为符合国密标准的ASN1编码的二进制数据。

##### 参数：
- sm2Handler：SM2算法句柄
- in ：要加密的字节码数组
- strPubKey：SM2公钥
##### 返回值:

- 返回加密后的字节码数组，为符合国密标准的ASN1编码的密文数据，如加密失败返回NULL。

##### 示例：
<pre>
byte[] cipherbytes = SMUtils.getInstance(this).SM2Encrypt(sm2Handler, bytesPlain, strPubKey);
if(cipherbytes != null) {
}
</pre>

***
> ###byte[]     SM2Decrypt(long sm2Handler, byte[] in, String strPriKey);

##### 说明：

- 该接口SM2解密接口。

##### 参数：
- sm2Handler：SM2算法句柄
- in ：参数为待解密的密文数据，需符合国密标准的ASN1编码二进制格式。
- strPriKey：SM2私钥
##### 返回值:

- 返回解密后的字节码数组明文数据，如解密失败返回NULL。

##### 示例：
<pre>
bytesPlain = SMUtils.getInstance(this).SM2Decrypt(sm2Handler, cipherbytes, (String) (keypairs[0]));
if(bytesPlain != null) {
}
</pre>

***
> ###byte[]     SM2Sign(long sm2Handler, byte[] msg, byte[] id, String strPubKey, String strPriKey);

##### 说明：

- 该接口SM2签名接口。

##### 参数：
- sm2Handler：SM2算法句柄
- msg ：待签名的消息数据
- id：SM2签名算法中的userID，如果userID传NULL或者“”，内部将使用默认值，尽管国密标准文件未规定默认值，但CFCA以及其他业界商用产品均使用默认值1234567812345678，故此处也以该值为userID的默认取值。
- strPubKey：SM2公钥
- strPriKey：SM2私钥
##### 返回值:

- 返回签名结果，以ASN1编码，如签名失败返回NULL。

##### 示例：
<pre>
byte[] signDatas = SMUtils.getInstance(this).SM2Sign(sm2Handler, bytesPlain, strid.getBytes(), strPubKey, strPriKey);
if(signDatas != null) {
}
</pre>

***
> ###int     SM2Verify(long sm2Handler, byte[] msg, byte[] id, String strPubKey, byte[] sig);

##### 说明：

- 该接口SM2验签接口。

##### 参数：
- sm2Handler：SM2算法句柄
- msg ：签名的消息数据
- id：SM2签名算法中的userID，如果userID传NULL或者“”，内部将使用默认值，尽管国密标准文件未规定默认值，但CFCA以及其他业界商用产品均使用默认值1234567812345678，故此处也以该值为userID的默认取值。
- strPubKey：SM2公钥
- sig：待验签的签名数据
##### 返回值:

- 返回验签结果，0--成功，其它--失败

##### 示例：
<pre>
int result = SMUtils.getInstance(this).SM2Verify(sm2Handler, bytesPlain, strid.getBytes(), (String) (keypairs[1]), signDatas);
if(result == 0) {
}
</pre>

***
> ###byte[]     SM3(byte[] data);

##### 说明：

- SM3哈希计算接口。

##### 参数：
- data：需要做散列运算的数据
##### 返回值:

- 返回SM3的哈希计算结果

##### 示例：
<pre>
byte[] bytesMD2 = SMUtils.getInstance(this).SM3(bytesPlain);
if(bytesMD2 != null) {
}
</pre>

***
> ###byte[]     SM4GenKey();

##### 说明：

- 生成SM4对称密钥的接口

##### 返回值:

- SM4对称密钥，为128bit/16字节的二进制数据。

##### 示例：
<pre>
byte[] sm4keys = SMUtils.getInstance(this).SM4GenKey();
if(sm4keys != null) {
}
</pre>

***
> ###byte[]     SM4CBCEncrypt(byte[] in, byte[] key, byte[] iv);

##### 说明：

- SM4 CBC模式加密接口。使用PKCS#7填充标准。

##### 参数：
- in：待加密的原始数据
- key：SM4对称密钥，128bit/16字节的二进制数据
- iv：初始化向量，128bit/16字节的二进制数据
##### 返回值:

- SM4加密后的密文

##### 示例：
<pre>
byte[] sm4CBCCipher = SMUtils.getInstance(this).SM4CBCEncrypt(bytesPlain, sm4keys, iv);
if(sm4CBCCipher != null) {
}
</pre>

***
> ###byte[]     SM4CBCDecrypt(byte[] in, byte[] key, byte[] iv);

##### 说明：

- SM4 CBC模式解密接口。

##### 参数：
- in：待解密的密文数据
- key：SM4对称密钥，128bit/16字节的二进制数据
- iv：初始化向量，128bit/16字节的二进制数据
##### 返回值:

- 返回解密后的明文数据

##### 示例：
<pre>
byte[] decryptbytes = SMUtils.getInstance(this).SM4CBCDecrypt(sm4CBCCipher, sm4keys, iv);
if(decryptbytes != null) {
}
</pre>

***
> ###byte[]     SM4CBCEncryptNoPadding(byte[] in, byte[] key, byte[] iv);

##### 说明：

- SM4 CBC模式对称秘钥加密接口，无填充。请保证明文为16字节整数倍

##### 参数：
- in：待加密的原始数据
- key：SM4对称密钥，128bit/16字节的二进制数据
- iv：初始化向量，128bit/16字节的二进制数据
##### 返回值:

- SM4加密后的密文

##### 示例：
<pre>
sm4CBCCipher = SMUtils.getInstance(this).SM4CBCEncryptNoPadding(bytesPlain1, sm4keys, iv);
if(sm4CBCCipher != null) {
}
</pre>

***
> ###byte[]     SM4CBCDecryptNoPadding(byte[] in, byte[] key, byte[] iv);

##### 说明：

- SM4 CBC模式对称秘钥解密接口，无填充。

##### 参数：
- in：待解密的密文数据
- key：SM4对称密钥，128bit/16字节的二进制数据
- iv：初始化向量，128bit/16字节的二进制数据
##### 返回值:

- 返回解密后的明文数据

##### 示例：
<pre>
decryptbytes = SMUtils.getInstance(this).SM4CBCDecryptNoPadding(sm4CBCCipher, sm4keys, iv);
if(decryptbytes != null) {
}
</pre>

***
> ### byte[]     SM4ECBEncrypt(byte[] in, byte[] key);

##### 说明：

- SM4 ECB模式对称加解密。加密接口。

##### 参数：
- in：待加密的原始数据
- key：SM4对称密钥，128bit/16字节的二进制数据
##### 返回值:

- 返回加密后的密文

##### 示例：
<pre>
byte[] sm4EBCCipher = SMUtils.getInstance(this).SM4ECBEncrypt(bytesPlain, sm4keys);
if(sm4EBCCipher != null) {
}
</pre>

***
> ###byte[]     SM4ECBDecrypt(byte[] in, byte[] key);

##### 说明：

- SM4 ECB模式对称加解密。解密接口。

##### 参数：
- in：待解密的密文数据
- key：SM4对称密钥，128bit/16字节的二进制数据
##### 返回值:

- 返回解密后的明文数据

##### 示例：
<pre>
decryptbytes = SMUtils.getInstance(this).SM4ECBDecrypt(sm4EBCCipher, sm4keys);
if(decryptbytes != null) {
}
</pre>

***
> ###byte[]     SM4ECBEncryptNoPadding(byte[] in, byte[] key);

##### 说明：

- SM4 ECB模式对称加解密。加密接口，无填充。

##### 参数：
- in：待加密的原始数据
- key：SM4对称密钥，128bit/16字节的二进制数据
##### 返回值:

- 返回加密后的密文

##### 示例：
<pre>
sm4EBCCipher = SMUtils.getInstance(this).SM4ECBEncryptNoPadding(bytesPlain1, sm4keys);
if(sm4EBCCipher != null) {
}
</pre>

***
> ###byte[]     SM4ECBDecryptNoPadding(byte[] in, byte[] key);

##### 说明：

- SM4 ECB模式对称加解密。解密接口，无填充。

##### 参数：
- in：待解密的密文数据
- key：SM4对称密钥，128bit/16字节的二进制数据
##### 返回值:

- 返回解密后的明文数据

##### 示例：
<pre>
decryptbytes = SMUtils.getInstance(this).SM4ECBDecryptNoPadding(sm4EBCCipher, sm4keys);
if(decryptbytes != null) {
}
</pre>

***

## 密钥管理接口
密钥管理接口定义于SMKeyManager.java文件中。支持的功能包括id为基础的key空间，生成及保存对称和非对称秘钥，根据描述符获取对称和非对称秘钥，获得所有描述符列表等。
***
> ###static SMKeyManager getInstance(Context context, String keypath)

##### 说明：

- 该接口获得密钥管理的单例对象，如需使用密钥管理功能其他接口需首先调用该接口以获得单例对象。

##### 参数：
- context：应用上下文
- keypath：指定的保存路径，如果设置为null，则使用默认路径。应用中的key保存在该路径下。
##### 返回值:

- 返回SMKeyManager单例对象

##### 示例：
<pre>
SMKeyManager keymanager = SMKeyManager.getInstance(this, null);
</pre>

***
> ###static  void destroyInstance()；

##### 说明：

- 销毁SMKeyManager单例对象

##### 示例：
<pre>
SMKeyManager.destroyInstance();
</pre>

***
> ###boolean initWithAuthority(String idHash, String pinHash, String userSalt)

##### 说明：

- 如首次使用KMS模块，需先调用该接口以初始化Key空间，首次调用传入合法参数后即可以使用。
     针对传入不同参数的情形，可多次调用，不同参数将会初始化不同的密钥管理空间，如业务
     App需要针对不同用户使用不同的密钥存储空间，可以参数不同来进行分割密钥管理空间。

##### 参数：
- idHash：用户ID散列字符串，例如业务方针对不同用户ID使用不同的密钥管理空间，做到不同用户的安全隔离。
- pinHash：用户密码散列字符串；这里没有强制指定pin的语义，业务方可以根据安全强度需求来自行定义这个参数
- userSalt：可选使用的秘盐，为业务调用方自行定义的扩展因子，例如可以使用业务方服务器下发的固定因子以增加安全级别。
##### 返回值:

- 返回初始化是否成功

##### 示例：
<pre>
boolean result = keymanager.initWithAuthority(Utils.md5("user000"), Utils.md5("pwd000"), null);
if(result) {
}
</pre>

***
> ###boolean delWithAuthority(String idHash, String pinHash, String userSalt)

##### 说明：

- 删除授权对应key空间，该操作将会删除该授权下的所有密钥记录，谨慎操作。

##### 参数：
- idHash：用户ID散列字符串，例如业务方针对不同用户ID使用不同的密钥管理空间，做到不同用户的安全隔离。
- pinHash：用户密码散列字符串；这里没有强制指定pin的语义，业务方可以根据安全强度需求来自行定义这个参数
- userSalt：可选使用的秘盐，为业务调用方自行定义的扩展因子，例如可以使用业务方服务器下发的固定因子以增加安全级别。
##### 返回值:

- 返回删除操作是否成功 

##### 示例：
<pre>
result = keymanager.delWithAuthority(Utils.md5("user000"), Utils.md5("pwd000"), null);
if(result) {
}
</pre>

***
> ###boolean genAsymSM2Key(String desc, boolean forceupdate)

##### 说明：

- 产生SM2随机秘钥对，并以desc标识的密钥描述符持久化存储。如果密钥描述符已存在，当forceupdate参数为YES是则更新，否则将会返回失败。

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
- forceupdate：当密钥描述符存在时是否强制更新
##### 返回值:

- 返回是否调用成功

##### 示例：
<pre>
result = keymanager.genAsymSM2Key("desc001", true);
if(result) {
}
</pre>

***
> ###boolean genSymSM4Key(String desc, boolean forceupdate)

##### 说明：

- 产生SM4随机密钥，并以description标识的密钥描述符持久化存储。如果密钥描述符已存在，当bForceUpdate参数为YES是则更新，否则将会返回失败。

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
- forceupdate：当密钥描述符存在时是否强制更新
##### 返回值:

- 返回是否调用成功

##### 示例：
<pre>
result = keymanager.genSymSM4Key("desc003", true);
if(result) {
}
</pre>

***
> ###String[] getAllAsymKeyDesc()

##### 说明：

- 获取所有的非对称密钥(SM2)的密钥描述符。

##### 返回值:

- 返回所有保存的非对称秘钥(SM2)对应的描述符字符串数组，如果未成功，返回null

##### 示例：
<pre>
String[] descs = keymanager.getAllAsymKeyDesc();
if(descs != null) {
}
</pre>

***
> ###String[] getAllSymKeyDesc()

##### 说明：

- 获取所有的对称密钥(SM4)的密钥描述符。

##### 返回值:

- 返回所有保存的对称秘钥(SM4)对应的描述符字符串数组，如果未成功，返回null

##### 示例：
<pre>
String[] descs = keymanager.getAllSymKeyDesc();
if(descs != null) {
}
</pre>

***
> ###String getAsymPubKey(String desc)

##### 说明：

- 导出非对称秘钥的公钥，以密钥描述符为参数导出公钥，如描述符不存在会返回null。

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
##### 返回值:

- 返回非对称秘钥的公钥,失败时返回null

##### 示例：
<pre>
String resStr = keymanager.getAsymPubKey("desc001");
if(!TextUtils.isEmpty(resStr)) {
}
</pre>

***
> ###String getAsymPriKey(String desc)

##### 说明：

- 导出非对称秘钥的私钥

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
##### 返回值:

- 返回非对称秘钥的私钥,获取失败时返回null

##### 示例：
<pre>
resStr = keymanager.getAsymPriKey("desc001");
if(!TextUtils.isEmpty(resStr)) {
}
</pre>

***
> ###String[] getAsymPriKeypair(String desc)

##### 说明：

- 获取非对称加密秘钥的私钥和公钥

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
##### 返回值:

- 返回数组【0】私钥 【1】公钥，获取失败返回null

##### 示例：
<pre>
String[] keyPair = keymanager.getAsymPriKeypair(desc);
if(keyPair != null) {
}
</pre>

***
> ###String getSymKey(String desc)

##### 说明：

- 获取保存的对称加密秘钥

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
##### 返回值:

- 返回对称加密秘钥,获取失败返回null

##### 示例：
<pre>
String key = keymanager.getSymKey(desc);
if(key != null) {
}
</pre>

***
> ###boolean saveAsymKeypair(String desc, String pubKey, String priKey, boolean forceupdate)

##### 说明：

- 导入(SM2)非对称密钥，以密钥描述符为标识，以后需要导出、更新和删除以密钥描述符进行接口调用。
- 若秘钥描述符已存在，forceupdate=true则覆盖已有信息

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
- pubKey：公钥
- priKey：私钥
- forceupdate：是否覆盖已存在描述符的秘钥信息标识
##### 返回值:

- 返回调用是否成功

##### 示例：
<pre>
boolean lret = keymanager.saveAsymKeypair(desc, pubkey, prikey, true);
if(lret) {
}
</pre>

***
> ###boolean saveSymKey(String desc, String key, boolean forceupdate)

##### 说明：

- 导入(SM4)对称密钥，以密钥描述符为标识，以后需要导出、更新和删除以密钥描述符进行接口调用。
- 若秘钥描述符已存在，forceupdate=true则覆盖已有信息

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
- key：对称秘钥
- forceupdate：是否覆盖已存在描述符的秘钥信息标识
##### 返回值:

- 返回调用是否成功

##### 示例：
<pre>
boolean lret = keymanager.saveSymKey(desc, key, true);
if(lret) {
}
</pre>

***
> ###boolean delDataWithDesc(String desc) 

##### 说明：

- 删除key空间内秘钥描述符对应的秘钥数据。

##### 参数：
- desc：密钥描述符，字符串类型，不同的密钥描述符对应不同的密钥。是秘钥在key空间中的标识。
##### 返回值:

- 返回调用是否成功

##### 示例：
<pre>
boolean lret = keymanager.delDataWithDesc(desc);
if(lret) {
}
</pre>

***
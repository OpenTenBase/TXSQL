/* Copyright 2019, Tencent Technology (Shenzhen) Co Ltd

This file is part of the Tencent SM (Lite Version) Library.

The Tencent SM (Lite Version) Library is free software; you can redistribute it and/or modify
it under the terms of either:

* the GNU Lesser General Public License as published by the Free
Software Foundation; either version 3 of the License, or (at your
option) any later version.

or

* the GNU General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any
later version.

or both in parallel, as here.

The Tencent SM (Lite Version) Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the Tencent SM (Lite Version) Library.  If not,
see https://www.gnu.org/licenses/.  */

#ifndef TENCENTSM_LITE_SOURCE_SM_H_
#define TENCENTSM_LITE_SOURCE_SM_H_
#ifdef OS_ANDROID
#ifdef DEBUG
#include <android/log.h>
#endif
#endif
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#if defined(_WIN32)
#if defined(SMLib_EXPORTS)
#define SMLib_EXPORT __declspec(dllexport)
#else
#define SMLib_EXPORT __declspec(dllimport)
#endif
#else /* defined (_WIN32) */
#define SMLib_EXPORT
#endif

#define TENCENTSM_VERSION ("1.2.1-3")

#define SM3_DIGEST_LENGTH 32
#define SM3_BLOCK_SIZE 64
#define SM3_HMAC_SIZE (SM3_DIGEST_LENGTH)

typedef enum {
  SM_MD_SM3 = 1,
} SM_MD_TYPE;

typedef struct {
  uint32_t digest[8];
  int nblocks;
  unsigned char block[64];
  int num;
} sm3_ctx_t;
typedef struct stHmacSm3Ctx TstHmacSm3Ctx;

typedef struct {
  void *group;
  void *generator;
  void *jcb_generator;
  void *jcb_compute_var;
  void *bn_vars;
  void *ec_vars;
  void *pre_comp_g;
  void *pre_comp_p;
  void *rand_ctx;
  void *pubkey_x;
  void *pubkey_y;
  void *sign_random;
} sm2_ctx_t;

SMLib_EXPORT const char *version(void);

/**
SM2上下文结构体的大小
*/
SMLib_EXPORT int SM2CtxSize(void);

/**
使用SM2获取公私钥或加解密之前，必须调用SM2InitCtx或者SM2InitCtxWithPubKey函数
@param ctx  函数出参 - 上下文
*/
SMLib_EXPORT void SM2InitCtx(sm2_ctx_t *ctx);

/**
 使用SM2获取公私钥或加解密之前，必须调用SM2InitCtx或者SM2InitCtxWithPubKey函数
 如果使用固定公钥加密，可调用SM2InitCtxWithPubKey，将获得较大性能提升
 @param ctx  函数出参 - 上下文
 @param pubkey  函数入参 - 公钥
*/
SMLib_EXPORT void SM2InitCtxWithPubKey(sm2_ctx_t *ctx, const char *pubkey);

/**
使用完SM2算法后，必须调用free函数释放
@param ctx  函数入参 - 上下文
*/
SMLib_EXPORT void SM2FreeCtx(sm2_ctx_t *ctx);

/**
 生成私钥
 @param ctx  函数入参 - 上下文
 @param out  函数出参 -
 私钥，私钥实际上为256bit的大整数，这里输出的为256bit二进制内容Hex后的ASCII编码的可见字符串，长度为64字节，为保证字符串的结束符0，out至少需分配65字节空间。
 @return  0表示成功，其他值为错误码
*/
SMLib_EXPORT int generatePrivateKey(sm2_ctx_t *ctx, char *out);

/**
 根据私钥生成对应公钥，
 @param ctx 函数入参 - 上下文
 @param privateKey 函数入参 -
 私钥，私钥实际上为256bit的大整数，这里输出的为256bit二进制内容Hex后的ASCII编码的可见字符串，长度为64字节
 @param outPubKey 函数出参 - 公钥，公钥格式为04 | X | Y，其中X和Y为256bit大整数，这里输出的为04 | X
 |
 Y的二进制内容Hex后的ASCII编码的可见字符串，长度为130字节，为保证字符串的结束符0，outPubKey至少需分配131字节空间。
 @return  0表示成功，其他值为错误码
*/
SMLib_EXPORT int generatePublicKey(sm2_ctx_t *ctx, const char *privateKey, char *outPubKey);

/**
生成公私钥对
@param ctx 函数入参 - 上下文
@param outPriKey 函数出参 -
私钥，私钥实际上为256bit的大整数，这里输出的为256bit二进制内容Hex后的ASCII编码的可见字符串，长度为64字节，为保证字符串的结束符0，outPubKey至少需分配65字节空间。
@param outPubKey 函数出参 - 公钥，公钥格式为04 | X | Y，其中X和Y为256bit大整数，这里输出的为04 | X |
Y的二进制内容Hex后的ASCII编码的可见字符串，长度为130字节，为保证字符串的结束符0，outPubKey至少需分配131字节空间。
@return  0表示成功，其他值为错误码
*/
SMLib_EXPORT int generateKeyPair(sm2_ctx_t *ctx, char *outPriKey, char *outPubKey);

/**
SM2非对称加解密算法，加密
@param ctx 函数入参 - 上下文
@param in  函数入参 - 待加密消息
@param inlen  函数入参 - 消息长度(字节单位)
@param strPubKey  函数入参 - 公钥
@param pubkeyLen  函数入参 - 公钥长度
@param out  函数出参 - 密文
@param outlen  函数出参 - 密文长度
@return  0表示成功，其他值为错误码
*/
SMLib_EXPORT int SM2Encrypt(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen,
                            const char *strPubKey, size_t pubkeyLen, unsigned char *out,
                            size_t *outlen);

/**
SM2非对称加解密算法，解密
@param ctx  函数入参 - 上下文
@param in  函数入参 - 待解密密文
@param inlen  函数入参 - 密文长度(字节单位)
@param strPriKey  函数入参 - 私钥
@param prikeyLen  函数入参 - 私钥长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度
@return  0表示成功，其他值为错误码
*/
SMLib_EXPORT int SM2Decrypt(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen,
                            const char *strPriKey, size_t prikeyLen, unsigned char *out,
                            size_t *outlen);

/**
SM2签名验签算法，签名
@param ctx 函数入参 - 上下文
@param msg 函数入参 - 待签名消息
@param msglen 函数入参 - 待签名消息长度
@param id 函数入参 - 用户ID(作用是加入到签名hash中，对于传入值无特殊要求)
@param idlen 函数入参 - 用户ID长度
@param strPubKey 函数入参 - 公钥(作用是加入到签名hash中)
@param pubkeyLen 函数入参 - 公钥长度
@param strPriKey 函数入参 - 私钥
@param prikeyLen 函数入参 - 私钥长度
@param sig 函数出参 - 签名结果
@param siglen 函数出参 - 签名结果长度
*/
SMLib_EXPORT int SM2Sign(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen, const char *id,
                         size_t idlen, const char *strPubKey, size_t pubkeyLen,
                         const char *strPriKey, size_t prikeyLen, unsigned char *sig,
                         size_t *siglen);

/**
SM2签名验签算法，验签
@param ctx 函数入参 - 上下文
@param msg 函数入参 - 待验签内容
@param msglen 函数入参 - 待验签内容长度
@param id 函数入参 - 用户ID
@param idlen 函数入参 - 用户ID长度
@param sig 函数入参 - 签名结果
@param siglen 函数入参 - 签名结果长度
@param strPubKey 函数入参 - 公钥
@param pubkeyLen 函数入参 - 公钥长度
@return 0表示成功，其他值为错误码
*/
SMLib_EXPORT int SM2Verify(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen, const char *id,
                           size_t idlen, const unsigned char *sig, size_t siglen,
                           const char *strPubKey, size_t pubkeyLen);

/**
SM3上下文结构体的大小
*/
SMLib_EXPORT int SM3CtxSize(void);

/**
SM3 hash算法，3个接口用法与OpenSSL的MD5算法的接口保持一致。
digest至少需要分配32字节
*/
SMLib_EXPORT void SM3Init(sm3_ctx_t *ctx);
SMLib_EXPORT void SM3Update(sm3_ctx_t *ctx, const unsigned char *data, size_t data_len);
SMLib_EXPORT void SM3Final(sm3_ctx_t *ctx, unsigned char *digest);

/**
SM3 hash算法， 内部依次调用了init update和final三个接口
*/
SMLib_EXPORT void SM3(const unsigned char *data, size_t datalen, unsigned char *digest);

/**
 * @brief 基于sm3算法计算HMAC值 ctx init
 * @param key HMAC用的秘钥
 * @param key_len 秘钥长度
 * @return 0 -- OK
 */
SMLib_EXPORT TstHmacSm3Ctx *SM3_HMAC_Init(const unsigned char *key, size_t key_len);
/**
 * @brief 基于sm3算法计算HMAC值 update数据
 * @param ctx hmac上下文结构指针
 * @param data 做HMAC计算的数据
 * @param data_len 数据长度
 * @return 0 -- OK
 */
SMLib_EXPORT int SM3_HMAC_Update(TstHmacSm3Ctx *ctx, const unsigned char *data, size_t data_len);
/**
 * @brief 基于sm3算法计算HMAC值 最终计算HMAC值
 * @param ctx hmac上下文结构指针
 * @param mac 输出的HMAC字节码
 * @return 0 -- OK
 */
SMLib_EXPORT int SM3_HMAC_Final(TstHmacSm3Ctx *ctx, unsigned char mac[SM3_HMAC_SIZE]);
/**
 * @brief 基于sm3算法计算HMAC值
 * @param data 做HMAC计算的数据
 * @param data_len 数据长度
 * @param key HMAC用的秘钥
 * @param key_len 秘钥长度
 * @param mac 输出的HMAC字节码
 * @return 0 -- OK
 */
SMLib_EXPORT int SM3_HMAC(const unsigned char *data, size_t data_len, const unsigned char *key,
                          size_t key_len, unsigned char mac[SM3_HMAC_SIZE]);

/**
生成16字节128bit的SM4 Key，也可调用该接口生成SM4 CBC模式的初始化向量iv，iv长度和key长度一致
@param outKey  函数出参 - 16字节密钥。
*/
SMLib_EXPORT void generateSM4Key(unsigned char *outKey);

/**
 SM4 ECB模式对称加解密。加密
 @param in  函数入参 - 明文
 @param inlen  函数入参 - 明文长度
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 @param key  函数入参 - 秘钥（128bit）
 */
SMLib_EXPORT void SM4_ECB_Encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                                  size_t *outlen, const unsigned char *key);

/**
 SM4 ECB模式对称加解密。解密
 @param in  函数入参 - 密文
 @param inlen  函数入参 - 密文长度
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 @param key  函数入参 - 秘钥（128bit）
 */
SMLib_EXPORT void SM4_ECB_Decrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                                  size_t *outlen, const unsigned char *key);

/**
 SM4 ECB模式对称加密，无填充。请保证明文为16字节整数倍，否则加密会失败，即出参outlen为0
 @param in  函数入参 - 明文
 @param inlen  函数入参 - 明文长度
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 @param key  函数入参 - 秘钥（128bit）
 */
SMLib_EXPORT void SM4_ECB_Encrypt_NoPadding(const unsigned char *in, size_t inlen,
                                            unsigned char *out, size_t *outlen,
                                            const unsigned char *key);

/**
 SM4 ECB模式对称解密，无填充。请保证密文为16字节整数倍，否则解密会失败，即出参outlen为0
 @param in  函数入参 - 密文
 @param inlen  函数入参 - 密文长度
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 @param key  函数入参 - 秘钥（128bit）
 */
SMLib_EXPORT void SM4_ECB_Decrypt_NoPadding(const unsigned char *in, size_t inlen,
                                            unsigned char *out, size_t *outlen,
                                            const unsigned char *key);

/**
SM4 CBC模式对称加解密。加密，使用PKCS#7填充标准
@param in  函数入参 - 明文
@param inlen  函数入参 - 明文长度
@param out  函数出参 - 密文
@param outlen  函数出参 - 密文长度
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量
*/
SMLib_EXPORT void SM4_CBC_Encrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                                  size_t *outlen, const unsigned char *key,
                                  const unsigned char *iv);

/**
SM4 CBC模式对称加解密。解密，使用PKCS#7填充标准
@param in  函数入参 - 密文
@param inlen  函数入参 - 密文长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量

*/
SMLib_EXPORT void SM4_CBC_Decrypt(const unsigned char *in, size_t inlen, unsigned char *out,
                                  size_t *outlen, const unsigned char *key,
                                  const unsigned char *iv);

/**
SM4 CBC模式对称加解密。加密，无填充。请保证明文为16字节整数倍，否则加密会失败，即出参outlen为0
@param in 函数入参 - 明文
@param inlen 函数入参 - 明文长度
@param out 函数出参 - 密文
@param outlen 函数出参 - 密文长度
@param key 函数入参 - 秘钥（128bit）
@param iv 函数入参 - 初始化向量
*/
SMLib_EXPORT void SM4_CBC_Encrypt_NoPadding(const unsigned char *in, size_t inlen,
                                            unsigned char *out, size_t *outlen,
                                            const unsigned char *key, const unsigned char *iv);

/**
SM4 CBC模式对称解密，无填充。请保证密文为16字节整数倍，否则解密会失败，即出参outlen为0
@param in  函数入参 - 密文
@param inlen  函数入参 - 密文长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量
*/
SMLib_EXPORT void SM4_CBC_Decrypt_NoPadding(const unsigned char *in, size_t inlen,
                                            unsigned char *out, size_t *outlen,
                                            const unsigned char *key, const unsigned char *iv);

/**
 SM4 GCM模式对称加解密。加密，使用PKCS7填充，实际上GCM模式可不填充，非短明文加密推荐使用SM4_GCM_Encrypt_NoPadding替代。
@param in  函数入参 - 明文
@param inlen  函数入参 - 明文长度
@param out  函数出参 - 密文
@param outlen  函数出参 - 密文长度
@param tag  函数出参 - GMAC值，即消息验证码
@param taglen  既作函数入参也作为函数出参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量，GCM模式的向量长度与CBC模式不同，不一定需要使用128bit，旧接口内部默认使用了8字节
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 成功为0，一般加密失败是由参数错误导致
*/
SMLib_EXPORT int SM4_GCM_Encrypt(const unsigned char *in, size_t inlen,
                                 unsigned char *out, size_t *outlen,
                                 unsigned char *tag, size_t *taglen,
                                 const unsigned char *key,
                                 const unsigned char *iv,
                                 const unsigned char *aad, size_t aadlen);

/**
 SM4 GCM模式对称加解密。解密，使用PKCS7填充，实际上GCM模式可不填充。
@param in  函数入参 - 密文
@param inlen  函数入参 - 密文长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度
@param tag  函数入参 - GMAC值，即消息验证码
@param taglen  函数入参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量，GCM模式的向量长度与CBC模式不同，不一定需要使用128bit，旧接口内部默认使用了8字节
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 成功为0，GCM的解密失败主要是tag校验失败
*/
SMLib_EXPORT int SM4_GCM_Decrypt(const unsigned char *in, size_t inlen,
                                 unsigned char *out, size_t *outlen,
                                 const unsigned char *tag, size_t taglen,
                                 const unsigned char *key,
                                 const unsigned char *iv,
                                 const unsigned char *aad, size_t aadlen);

/**
SM4 GCM模式对称加解密。加密，无填充，明文长度无要求。
@param in  函数入参 - 明文
@param inlen  函数入参 - 明文长度
@param out  函数出参 - 密文
@param outlen  函数出参 - 密文长度(GCM NOPADDING模式密文长度与明文长度一致)
@param tag  函数出参 - GMAC值，即消息验证码
@param taglen  既作函数入参也作为函数出参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量，GCM模式的向量长度与CBC模式不同，不一定需要使用128bit，旧接口内部默认使用了8字节
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 成功为0，一般加密失败是由参数错误导致
*/
SMLib_EXPORT int SM4_GCM_Encrypt_NoPadding(const unsigned char *in, size_t inlen,
                                           unsigned char *out, size_t *outlen,
                                           unsigned char *tag, size_t *taglen,
                                           const unsigned char *key,
                                           const unsigned char *iv,
                                           const unsigned char *aad, size_t aadlen);

/**
SM4 GCM模式对称加解密。解密，无填充，密文长度无要求。
@param in  函数入参 - 密文
@param inlen  函数入参 - 密文长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度(GCM NOPADDING模式密文长度与明文长度一致)
@param tag  函数入参 - GMAC值，即消息验证码
@param taglen  函数入参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量，GCM模式的向量长度与CBC模式不同，不一定需要使用128bit，旧接口内部默认使用了8字节
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 返回解密是否失败，GCM的解密失败主要是tag校验失败
*/
SMLib_EXPORT int SM4_GCM_Decrypt_NoPadding(const unsigned char *in, size_t inlen,
                                           unsigned char *out, size_t *outlen,
                                           const unsigned char *tag, size_t taglen,
                                           const unsigned char *key,
                                           const unsigned char *iv,
                                           const unsigned char *aad, size_t aadlen);

/**
以下接口为支持任意长度(>0)iv的GCM接口
按照NIST SP800-38D标准实现GCM部分算法
RFC5647标准iv推荐使用12字节，96bit
*/
/**
 SM4 GCM模式对称加解密。加密，使用PKCS7填充，实际上GCM模式可不填充，非短明文加密推荐使用SM4_GCM_Encrypt_NoPadding替代。
@param in  函数入参 - 明文
@param inlen  函数入参 - 明文长度
@param out  函数出参 - 密文
@param outlen  函数出参 - 密文长度
@param tag  函数出参 - GMAC值，即消息验证码
@param taglen  既作函数入参也作为函数出参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量
@param ivlen 函数入参 - 初始化向量长度
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 成功为0，一般加密失败是由参数错误导致
*/
SMLib_EXPORT int SM4_GCM_Encrypt_NIST_SP800_38D(const unsigned char *in, size_t inlen,
                                 unsigned char *out, size_t *outlen,
                                 unsigned char *tag, size_t *taglen,
                                 const unsigned char *key,
                                 const unsigned char *iv,size_t ivlen,
                                 const unsigned char *aad, size_t aadlen);

/**
 SM4 GCM模式对称加解密。解密，使用PKCS7填充，实际上GCM模式可不填充。
@param in  函数入参 - 密文
@param inlen  函数入参 - 密文长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度
@param tag  函数入参 - GMAC值，即消息验证码
@param taglen  函数入参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量
@param ivlen 函数入参 - 初始化向量长度
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 成功为0，GCM的解密失败主要是tag校验失败
*/
SMLib_EXPORT int SM4_GCM_Decrypt_NIST_SP800_38D(const unsigned char *in, size_t inlen,
                                 unsigned char *out, size_t *outlen,
                                 const unsigned char *tag, size_t taglen,
                                 const unsigned char *key,
                                 const unsigned char *iv,size_t ivlen,
                                 const unsigned char *aad, size_t aadlen);

/**
SM4 GCM模式对称加解密。加密，无填充，明文长度无要求。
@param in  函数入参 - 明文
@param inlen  函数入参 - 明文长度
@param out  函数出参 - 密文
@param outlen  函数出参 - 密文长度(GCM NOPADDING模式密文长度与明文长度一致)
@param tag  函数出参 - GMAC值，即消息验证码
@param taglen  既作函数入参也作为函数出参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量
@param ivlen 函数入参 - 初始化向量长度
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 成功为0，一般加密失败是由参数错误导致
*/
SMLib_EXPORT int SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(const unsigned char *in, size_t inlen,
                                           unsigned char *out, size_t *outlen,
                                           unsigned char *tag, size_t *taglen,
                                           const unsigned char *key,
                                           const unsigned char *iv,size_t ivlen,
                                           const unsigned char *aad, size_t aadlen);

/**
SM4 GCM模式对称加解密。解密，无填充，密文长度无要求。
@param in  函数入参 - 密文
@param inlen  函数入参 - 密文长度
@param out  函数出参 - 明文
@param outlen  函数出参 - 明文长度(GCM NOPADDING模式密文长度与明文长度一致)
@param tag  函数入参 - GMAC值，即消息验证码
@param taglen  函数入参 - GMAC长度，通常取16字节
@param key  函数入参 - 秘钥（128bit）
@param iv  函数入参 - 初始化向量
@param ivlen 函数入参 - 初始化向量长度
@param aad  函数入参 - 附加验证消息
@param aadlen  函数入参 - 附加验证消息长度
@return 返回解密是否失败，GCM的解密失败主要是tag校验失败
*/
SMLib_EXPORT int SM4_GCM_Decrypt_NoPadding_NIST_SP800_38D(const unsigned char *in, size_t inlen,
                                           unsigned char *out, size_t *outlen,
                                           const unsigned char *tag, size_t taglen,
                                           const unsigned char *key,
                                           const unsigned char *iv,size_t ivlen,
                                           const unsigned char *aad, size_t aadlen);

/**
  ---------------------------------------------------------------- 以下为非通用接口
----------------------------------------------------------------
**/

/*
  SM2非对称加密的结果由C1,C2,C3三部分组成。其中C1是生成随机数的计算出的椭圆曲线点,C2是密文数据,C3是SM3的摘要值。
  C1||C3||C2的ASN1编码格式为目前最新标准规范格式，旧版本标准规范格式为C1||C2||C3
 */
typedef enum SM2CipherMode {
  SM2CipherMode_C1C3C2_ASN1,
  SM2CipherMode_C1C3C2,
  SM2CipherMode_C1C2C3_ASN1,
  SM2CipherMode_C1C2C3
} SM2CipherMode;

/**
 SM2非对称加解密算法，加密的兼容接口
 #param1  函数入参 - 上下文
 #param2  函数入参 - 待加密消息
 #param3  函数入参 - 消息长度(字节单位)
 #param4  函数入参 - 公钥
 #param5  函数入参 - 公钥长度
 #param6  函数出参 - 密文
 #param7  函数出参 - 密文长度
 #param8 密文输出格式
 @return  0表示成功，其他值为错误码
 */
SMLib_EXPORT int SM2EncryptWithMode(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen,
                                    const char *strPubKey, size_t pubkeyLen, unsigned char *out,
                                    size_t *outlen, SM2CipherMode mode);

/**
 SM2非对称加解密算法，解密的兼容接口
 #param1  函数入参 - 上下文
 #param2  函数入参 - 待解密密文
 #param3  函数入参 - 密文长度(字节单位)
 #param4  函数入参 - 私钥
 #param5  函数入参 - 私钥长度
 #param6  函数出参 - 明文
 #param7  函数出参 - 明文长度
 #param8  密文格式
 @return  0表示成功，其他值为错误码
 */
SMLib_EXPORT int SM2DecryptWithMode(sm2_ctx_t *ctx, const unsigned char *in, size_t inlen,
                                    const char *strPriKey, size_t prikeyLen, unsigned char *out,
                                    size_t *outlen, SM2CipherMode mode);

/*
  SM2签名结果由R和S分量组成，标准规定需采用ASN1编码，但仍然提供SM2SignMode_RS模式，以便兼容那些没有使用ASN1编码的版本。
 */
typedef enum SM2SignMode { SM2SignMode_RS_ASN1, SM2SignMode_RS } SM2SignMode;

/**
 SM2签名验签算法，签名的兼容接口
 #param1 函数入参 - 上下文
 #param2 函数入参 - 待签名消息
 #param3 函数入参 - 待签名消息长度
 #param4 函数入参 - 用户ID(作用是加入到签名hash中，对于传入值无特殊要求)
 #param5 函数入参 - 用户ID长度
 #param6 函数入参 - 公钥(作用是加入到签名hash中)
 #param7 函数入参 - 公钥长度
 #param8 函数入参 - 私钥
 #param9 函数入参 - 私钥长度
 #param10 函数出参 - 签名结果
 #param11 函数出参 - 签名结果长度
 #param12 签名格式
 */
SMLib_EXPORT int SM2SignWithMode(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen,
                                 const char *id, size_t idlen, const char *strPubKey,
                                 size_t pubkeyLen, const char *strPriKey, size_t prikeyLen,
                                 unsigned char *sig, size_t *siglen, SM2SignMode mode);

/**
 SM2签名验签算法，验签的兼容接口
 #param1 函数入参 - 上下文
 #param2 函数入参 - 待验签内容
 #param3 函数入参 - 待验签内容长度
 #param4 函数入参 - 用户ID
 #param5 函数入参 - 用户ID长度
 #param6 函数入参 - 签名结果
 #param7 函数入参 - 签名结果长度
 #param8 函数入参 - 公钥
 #param9 函数入参 - 公钥长度
 #param10 签名格式
 @return 0表示成功，其他值为错误码
 */
SMLib_EXPORT int SM2VerifyWithMode(sm2_ctx_t *ctx, const unsigned char *msg, size_t msglen,
                                   const char *id, size_t idlen, const unsigned char *sig,
                                   size_t siglen, const char *strPubKey, size_t pubkeyLen,
                                   SM2SignMode mode);

#ifdef __cplusplus
}
#endif
#endif  // TENCENTSM_LITE_SOURCE_SM_H_

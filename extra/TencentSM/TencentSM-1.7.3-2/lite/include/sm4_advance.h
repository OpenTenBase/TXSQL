/*
Copyright 2019, Tencent Technology (Shenzhen) Co Ltd
Description: This file is part of the Tencent SM (Pro Version) Library.
*/

#ifndef HEADER_SM4_ADVANCE_H
#define HEADER_SM4_ADVANCE_H

#ifdef OS_ANDROID
#ifdef DEBUG
#include <android/log.h>
#endif /* DEBUG */
#endif /* OS_ANDROID */

#include <stdint.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
#undef SMLib_EXPORT
#if defined (_WIN32) && !defined (_WIN_STATIC)
#if defined(SMLib_EXPORTS)
#define  SMLib_EXPORT __declspec(dllexport)
#else
#define  SMLib_EXPORT __declspec(dllimport)
#endif
#else /* defined (_WIN32) */
#define SMLib_EXPORT
#endif

#ifndef U64_TYPE_DEFINED
#if (defined(_WIN32) || defined(_WIN64)) && !defined(__MINGW32__)
typedef unsigned __int64 u64;
#elif defined(__arch64__)
typedef unsigned long u64;
#else
typedef unsigned long long u64;
#endif
#define U64_TYPE_DEFINED
#endif

typedef struct {
  void *rk; //扩展出的轮密钥
  unsigned char remain_c[16]; //缓存上一次未处理完的数据
  u64 mlen; //标示数据的的总长度（字节长度），不大于2^36-32
  unsigned int mres; //标示上一次未处理完的数据长度（字节长度），大于等于0，小于16字节
  int no_padding; //1:不填充；0:填充
  unsigned char cipher_buf[16];// 应用于带填充的解密。缓存本次运算结果的最后16字节，作为下一次运算输出结果的前16字节
  int is_one_block_cached; //应用于带填充的解密。1:已缓存1个分组的结果；0: 未缓存1个分组的结果
} tcsm_sm4_ecb_t;

typedef struct {
  void *rk; //扩展出的轮密钥
  unsigned char iv[16];//初始向量，每次运算后更新
  unsigned char remain_c[16]; //缓存上一次未处理完的数据
  u64 mlen; //标示数据的的总长度（字节长度），不大于2^36-32
  unsigned int mres; //标示上一次未处理完的数据长度（字节长度），大于等于0，小于16字节
  int no_padding; //1:不填充；0:填充
  unsigned char cipher_buf[16];// 应用于带填充的解密。缓存本次运算结果的最后16字节，作为下一次运算输出结果的前16字节
  int is_one_block_cached; //应用于带填充的解密。1:已缓存1个分组的结果；0: 未缓存1个分组的结果
} tcsm_sm4_cbc_t;

typedef struct {
  void *rk; //扩展出的轮密钥
  unsigned char iv[16];//初始向量，每次运算后更新
  unsigned char ekcnt[16]; //缓存count的加密结果
  u64 mlen; //标示数据的的总长度（字节长度），不大于2^36-32
  unsigned int mres; //标示上一次未处理完的数据长度（字节长度），大于等于0，小于16字节
} tcsm_sm4_ctr_t;


typedef struct {
  void *rk; //扩展出的轮密钥
  void *gcm_ctx;
} tcsm_sm4_gcm_t;

#define GCM_TAG_MAXLEN 16

/* ---------------------------------------------------------------- 以下为SM4分步计算接口 ---------------------------------------------------------------- */
/**
 SM4 ECB模式对称加解密。加密，初始化。
 @param ctx  函数入参 - ECB模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param no_padding  函数入参 - 1:不填充；0:填充
 */
SMLib_EXPORT int SM4_ECB_Encrypt_Init(tcsm_sm4_ecb_t *ctx, const unsigned char *key, int no_padding);

/**
 SM4 ECB模式对称加解密。加密，分步计算
 @param ctx  函数入参 - ECB模式上下文结构指针
 @param in  函数入参 - 明文
 @param inlen  函数入参 - 明文长度
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
SMLib_EXPORT int SM4_ECB_Encrypt_Update(tcsm_sm4_ecb_t *ctx, const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen);

/**
 SM4 ECB模式对称加解密。加密，最后计算
 @param ctx  函数入参 - ECB模式上下文结构指针
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
SMLib_EXPORT int SM4_ECB_Encrypt_Final(tcsm_sm4_ecb_t *ctx, unsigned char *out, size_t *outlen);

/**
 SM4 ECB模式对称加解密。解密，初始化。
 @param ctx  函数入参 - ECB模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param no_padding  函数入参 - 1:不填充；0:填充
 */
SMLib_EXPORT int SM4_ECB_Decrypt_Init(tcsm_sm4_ecb_t *ctx, const unsigned char *key, int no_padding);

/**
 SM4 ECB模式对称加解密。解密，分步计算
 @param ctx  函数入参 - ECB模式上下文结构指针
 @param in  函数入参 - 密文
 @param inlen  函数入参 - 密文长度
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 */
SMLib_EXPORT int SM4_ECB_Decrypt_Update(tcsm_sm4_ecb_t *ctx, const unsigned char *in, size_t inlen,unsigned char *out, size_t *outlen);

/**
 SM4 ECB模式对称加解密。解密，最后计算
 @param ctx  函数入参 - ECB模式上下文结构指针
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 */
SMLib_EXPORT int SM4_ECB_Decrypt_Final(tcsm_sm4_ecb_t *ctx, unsigned char *out, size_t *outlen);

/**
 SM4 CBC模式对称加解密。加密，初始化。
 @param ctx  函数入参 - CBC模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param iv 函数入参 - 初始化向量
 @param no_padding  函数入参 - 1:不填充；0:填充 
 */
SMLib_EXPORT int SM4_CBC_Encrypt_Init(tcsm_sm4_cbc_t *ctx, const unsigned char *key, const unsigned char *iv, int no_padding);

/**
 SM4 CBC模式对称加解密。加密，分步计算
 @param ctx  函数入参 - CBC模式上下文结构指针
 @param in  函数入参 - 明文
 @param inlen  函数入参 - 明文长度
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
SMLib_EXPORT int SM4_CBC_Encrypt_Update(tcsm_sm4_cbc_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);

/**
 SM4 CBC模式对称加解密。加密，最后计算
 @param ctx  函数入参 - CBC模式上下文结构指针
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
SMLib_EXPORT int SM4_CBC_Encrypt_Final(tcsm_sm4_cbc_t *ctx, unsigned char *out, size_t *outlen);

/**
 SM4 CBC模式对称加解密。解密，初始化。
 @param ctx  函数入参 - CBC模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param iv 函数入参 - 初始化向量
 @param no_padding  函数入参 - 1:不填充；0:填充
 */
SMLib_EXPORT int SM4_CBC_Decrypt_Init(tcsm_sm4_cbc_t *ctx, const unsigned char *key, const unsigned char *iv, int no_padding);

/**
 SM4 CBC模式对称加解密。解密，分步计算
 @param ctx  函数入参 - CBC模式上下文结构指针
 @param in  函数入参 - 密文
 @param inlen  函数入参 - 密文长度
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 */
SMLib_EXPORT int SM4_CBC_Decrypt_Update(tcsm_sm4_cbc_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);

/**
 SM4 CBC模式对称加解密。解密，最后计算
 @param ctx  函数入参 - CBC模式上下文结构指针
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 */
SMLib_EXPORT int SM4_CBC_Decrypt_Final(tcsm_sm4_cbc_t *ctx, unsigned char *out, size_t *outlen);

/**
 SM4 CTR模式对称加解密。加密，初始化。
 @param ctx  函数入参 - CTR模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param iv 函数入参 - 初始化向量
 */
SMLib_EXPORT int SM4_CTR_Encrypt_Init(tcsm_sm4_ctr_t *ctx, const unsigned char *key, const unsigned char *iv);

/**
 SM4 CTR模式对称加解密。加密，分步计算
 @param ctx  函数入参 - CTR模式上下文结构指针
 @param in  函数入参 - 明文
 @param inlen  函数入参 - 明文长度
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
SMLib_EXPORT int SM4_CTR_Encrypt_Update(tcsm_sm4_ctr_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);

/**
 SM4 CTR模式对称加解密。加密，最后计算
 @param ctx  函数入参 - CTR模式上下文结构指针
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
//SMLib_EXPORT int SM4_CTR_Encrypt_Final(tcsm_sm4_ctr_t *ctx, unsigned char *out, size_t *outlen);
SMLib_EXPORT int SM4_CTR_Encrypt_Final(tcsm_sm4_ctr_t *ctx);

/**
 SM4 CTR模式对称加解密。解密，初始化。
 @param ctx  函数入参 - CTR模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param iv 函数入参 - 初始化向量
 */
SMLib_EXPORT int SM4_CTR_Decrypt_Init(tcsm_sm4_ctr_t *ctx, const unsigned char *key, const unsigned char *iv);

/**
 SM4 CTR模式对称加解密。解密，分步计算
 @param ctx  函数入参 - CTR模式上下文结构指针
 @param in  函数入参 - 密文
 @param inlen  函数入参 - 密文长度
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 */
SMLib_EXPORT int SM4_CTR_Decrypt_Update(tcsm_sm4_ctr_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);

/**
 SM4 CTR模式对称加解密。解密，最后计算
 @param ctx  函数入参 - CTR模式上下文结构指针
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
//SMLib_EXPORT int SM4_CTR_Decrypt_Final(tcsm_sm4_ctr_t *ctx, unsigned char *out, size_t *outlen);
SMLib_EXPORT int SM4_CTR_Decrypt_Final(tcsm_sm4_ctr_t *ctx);

/**
 SM4 GCM模式对称加解密。加密，初始化。
 @param ctx  函数入参 - GCM模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param iv 函数入参 - 初始化向量
 @param ivlen 函数入参 - 初始化向量长度
 @param aad  函数入参 - 附加验证消息
 @param aadlen  函数入参 - 附加验证消息长度 
 */
SMLib_EXPORT int SM4_GCM_Encrypt_Init(tcsm_sm4_gcm_t *ctx, const unsigned char *key, const unsigned char *iv, size_t ivlen, const unsigned char *aad, size_t aadlen);

/**
 SM4 GCM模式对称加解密。加密，分步计算
 @param ctx  函数入参 - GCM模式上下文结构指针 
 @param in  函数入参 - 明文
 @param inlen  函数入参 - 明文长度
 @param out  函数出参 - 密文
 @param outlen  函数出参 - 密文长度
 */
SMLib_EXPORT int SM4_GCM_Encrypt_Update(tcsm_sm4_gcm_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);

/**
 SM4 GCM模式对称加解密。加密，最后计算
 @param ctx  函数入参 - GCM模式上下文结构指针
 @param tag  函数出参 - GMAC值，即消息验证码
 @param taglen  函数入参 - GMAC长度，大于0，小于等于16字节
 */
SMLib_EXPORT int SM4_GCM_Encrypt_Final(tcsm_sm4_gcm_t *ctx, unsigned char *tag, size_t taglen);

/**
 SM4 GCM模式对称加解密。解密，初始化。
 @param ctx  函数入参 - GCM模式上下文结构指针
 @param key  函数入参 - 秘钥（128bit）
 @param iv 函数入参 - 初始化向量
 @param ivlen 函数入参 - 初始化向量长度
 @param aad  函数入参 - 附加验证消息
 @param aadlen  函数入参 - 附加验证消息长度
 */
SMLib_EXPORT int SM4_GCM_Decrypt_Init(tcsm_sm4_gcm_t *ctx, const unsigned char *key, const unsigned char *iv, size_t ivlen, const unsigned char *aad, size_t aadlen);

/**
 SM4 GCM模式对称加解密。解密，分步计算
 @param ctx  函数入参 - GCM模式上下文结构指针
 @param in  函数入参 - 密文
 @param inlen  函数入参 - 密文长度
 @param out  函数出参 - 明文
 @param outlen  函数出参 - 明文长度
 */
SMLib_EXPORT int SM4_GCM_Decrypt_Update(tcsm_sm4_gcm_t *ctx, const unsigned char *in, size_t inlen, unsigned char *out, size_t *outlen);

/**
 SM4 GCM模式对称加解密。解密，最后计算
 @param ctx  函数入参 - GCM模式上下文结构指针
 @param tag  函数出参 - GMAC值，即消息验证码
 @param taglen  函数入参 - GMAC长度，大于0，小于等于16字节
 */
SMLib_EXPORT int SM4_GCM_Decrypt_Final(tcsm_sm4_gcm_t *ctx, const unsigned char *tag, size_t taglen);

#ifdef  __cplusplus
}
#endif
#endif

/**
 * jni defined
 * 
 */
#ifndef _SM_JNI_H_
#define _SM_JNI_H_

#ifdef JNI_INTERFACE
#include <stdint.h>
#include <stddef.h>
#include <jni.h>
#include "sm.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************↓↓↓ SM JNI 接口定义↓↓↓***********************************************************/
#define SM2_PRIVATE_KEY_LENGTH  66
#define SM2_PUBLICK_KEY_LENGTH  132
#define SM2_SIGN_LENGTH         164
#define SM4_KEYBYTE_LENGTH      16

JNIEXPORT jstring JNICALL Java_com_tenpay_utils_SMUtils_version(JNIEnv*, jclass);  

//SM2
JNIEXPORT jlong   JNICALL Java_com_tenpay_utils_SMUtils_SM2InitCtx(JNIEnv*, jobject);
JNIEXPORT jlong   JNICALL Java_com_tenpay_utils_SMUtils_SM2InitCtxWithPubKey(JNIEnv*, jobject, jstring strPubKey);
JNIEXPORT void    JNICALL Java_com_tenpay_utils_SMUtils_SM2FreeCtx(JNIEnv*, jobject, jlong sm2Handler);
/**
 * @return array[0] privatekey str
 *         array[1] publickey str
 */
JNIEXPORT jobjectArray    JNICALL Java_com_tenpay_utils_SMUtils_SM2GenKeyPair(JNIEnv*, jobject, jlong sm2Handler);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM2Encrypt(JNIEnv*, jobject, jlong sm2Handler, jbyteArray in, jstring strPubKey);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM2Decrypt(JNIEnv*, jobject, jlong sm2Handler, jbyteArray in, jstring strPriKey);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM2Sign(JNIEnv*, jobject, jlong sm2Handler, jbyteArray msg, 
                          jbyteArray id, jstring strPubKey, jstring strPriKey);
JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMUtils_SM2Verify(JNIEnv*, jobject, jlong sm2Handler, jbyteArray msg, 
                          jbyteArray id, jstring strPubKey, jbyteArray sig);
//SM3
JNIEXPORT jlong   JNICALL Java_com_tenpay_utils_SMUtils_SM3Init(JNIEnv*, jobject);
JNIEXPORT void    JNICALL Java_com_tenpay_utils_SMUtils_SM3Update(JNIEnv*, jobject, jlong sm3Handler, jbyteArray data);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM3Final(JNIEnv*, jobject, jlong sm3Handler);
JNIEXPORT void    JNICALL Java_com_tenpay_utils_SMUtils_SM3Free(JNIEnv* env, jobject jclassObj, jlong sm3Handler);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM3(JNIEnv*, jobject, jbyteArray data);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM3HMAC(JNIEnv*, jobject, jbyteArray data, jbyteArray key);
//SM4
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4GenKey(JNIEnv*, jobject);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4CBCEncrypt(JNIEnv*, jobject, jbyteArray in, jbyteArray key, jbyteArray iv);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4CBCDecrypt(JNIEnv*, jobject, jbyteArray in, jbyteArray key, jbyteArray iv);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4CBCEncryptNoPadding(JNIEnv*, jobject, jbyteArray in, jbyteArray key, jbyteArray iv);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4CBCDecryptNoPadding(JNIEnv*, jobject, jbyteArray in, jbyteArray key, jbyteArray iv);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4ECBEncrypt(JNIEnv*, jobject, jbyteArray in, jbyteArray key);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4ECBDecrypt(JNIEnv*, jobject, jbyteArray in, jbyteArray key);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4ECBEncryptNoPadding(JNIEnv*, jobject, jbyteArray in, jbyteArray key);
JNIEXPORT jbyteArray    JNICALL Java_com_tenpay_utils_SMUtils_SM4ECBDecryptNoPadding(JNIEnv*, jobject, jbyteArray in, jbyteArray key);

/******************************************kms jni interface***********************************************************/
#define SM_DESC_MAX_LEN   256  //带\0的描述符长度最大值
#define KMS_PUBKEY_MAX_LENGTH 200
#define KMS_PRIKEY_MAX_LENGTH 100
#define KMS_SYMKEY_MAX_LENGTH 33  //带\0

JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_checkKmsFile(JNIEnv*, jobject, jobjectArray factors, jstring dir_path);
JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_removeKmsFile(JNIEnv*, jobject, jobjectArray factors, jstring dir_path);

JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_SM4KeyGenWriteKms(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description,jint force_update);
JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_SM2KeyPairGenWriteKms(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description,jint force_update);

/**
 * @return <0 失败   >=0 数量值
 */
JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyDescriptionCount(JNIEnv*, jobject, jobjectArray factors, jstring dir_path);
/**
 * @return null 失败   其它：descriptions string数组
 */
JNIEXPORT jobjectArray    JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path);

/**
 * @return <0 失败   >=0 数量值
 */
JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyPairDescriptionCount(JNIEnv*, jobject, jobjectArray factors, jstring dir_path);
/**
 * @return null 失败   其它：descriptions string数组
 */
JNIEXPORT jobjectArray    JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyPairDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path);

/**
 * @return null 失败   其它：string数组--[0]char *prikey, [1]char *pubkey,
 */
JNIEXPORT jobjectArray    JNICALL Java_com_tenpay_utils_SMKeyManUtils_keyPairWithDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description);
/**
 * @return null 失败   其它：string--char* key
 */
JNIEXPORT jstring    JNICALL Java_com_tenpay_utils_SMKeyManUtils_keyWithDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description);

JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_importKeyWithDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description, jstring key, jint force_update);
JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_importKeyPairWithDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description, jstring pubkey, jstring prikey, jint force_update);

JNIEXPORT jint    JNICALL Java_com_tenpay_utils_SMKeyManUtils_delDescription(JNIEnv*, jobject, jobjectArray factors, jstring dir_path, jstring description);

#ifdef  __cplusplus
} //extern "C"
#endif

#endif //JNI_INTERFACE

#endif

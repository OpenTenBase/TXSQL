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

#ifndef TENCENTSM_LITE_SOURCE_TC_KMS_H_
#define TENCENTSM_LITE_SOURCE_TC_KMS_H_
#ifdef JNI_INTERFACE
#include <jni.h>
#endif
#include <stdio.h>

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

SMLib_EXPORT int tc_check_kms_file(const char *factors[], int number_of_factors,
                                   const char *dir_path);
SMLib_EXPORT int tc_remove_kms_file(const char *factors[], int number_of_factors,
                                    const char *dir_path);

SMLib_EXPORT int tc_generate_sm4_key_write_kms(const char *factors[], int number_of_factors,
                                               const char *dir_path, const char *description,
                                               int force_update);
SMLib_EXPORT int tc_generate_sm2_key_pair_write_kms(const char *factors[], int number_of_factors,
                                                    const char *dir_path, const char *description,
                                                    int force_update);

SMLib_EXPORT int tc_all_key_description_count(const char *factors[], int number_of_factors,
                                              const char *dir_path, int *count);
SMLib_EXPORT int tc_all_key_description(const char *factors[], int number_of_factors,
                                        const char *dir_path, char *description[],
                                        int description_count);

SMLib_EXPORT int tc_all_key_pair_description_count(const char *factors[], int number_of_factors,
                                                   const char *dir_path, int *count);
SMLib_EXPORT int tc_all_key_pair_description(const char *factors[], int number_of_factors,
                                             const char *dir_path, char *description[],
                                             int description_count);

SMLib_EXPORT int tc_key_pair_with_description(const char *factors[], int number_of_factors,
                                              const char *dir_path, const char *description,
                                              char *pubkey, char *prikey);
SMLib_EXPORT int tc_key_with_description(const char *factors[], int number_of_factors,
                                         const char *dir_path, const char *description, char *key);

SMLib_EXPORT int tc_import_key_with_description(const char *factors[], int number_of_factors,
                                                const char *dir_path, const char *description,
                                                const char *key, int force_update);
SMLib_EXPORT int tc_import_key_pair_with_description(const char *factors[], int number_of_factors,
                                                     const char *dir_path, const char *description,
                                                     const char *pubkey, const char *prikey,
                                                     int force_update);

SMLib_EXPORT int tc_delete_description(const char *factors[], int number_of_factors,
                                       const char *dir_path, const char *description);
/******************************************↓↓↓JNI
 * 接口定义↓↓↓***********************************************************/
#ifdef JNI_INTERFACE
#define SM_DESC_MAX_LEN 256  //带\0的描述符长度最大值
#define KMS_PUBKEY_MAX_LENGTH 200
#define KMS_PRIKEY_MAX_LENGTH 100
#define KMS_SYMKEY_MAX_LENGTH 33  //带\0

JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_checkKmsFile(JNIEnv *, jobject,
                                                                        jobjectArray factors,
                                                                        jstring dir_path);
JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_removeKmsFile(JNIEnv *, jobject,
                                                                         jobjectArray factors,
                                                                         jstring dir_path);

JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_SM4KeyGenWriteKms(JNIEnv *, jobject,
                                                                             jobjectArray factors,
                                                                             jstring dir_path,
                                                                             jstring description,
                                                                             jint force_update);
JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_SM2KeyPairGenWriteKms(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path, jstring description,
    jint force_update);

/**
 * @return <0 失败   >=0 数量值
 */
JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyDescriptionCount(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path);
/**
 * @return null 失败   其它：descriptions string数组
 */
JNIEXPORT jobjectArray JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyDescription(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path);

/**
 * @return <0 失败   >=0 数量值
 */
JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyPairDescriptionCount(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path);
/**
 * @return null 失败   其它：descriptions string数组
 */
JNIEXPORT jobjectArray JNICALL Java_com_tenpay_utils_SMKeyManUtils_allKeyPairDescription(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path);

/**
 * @return null 失败   其它：string数组--[0]char *prikey, [1]char *pubkey,
 */
JNIEXPORT jobjectArray JNICALL Java_com_tenpay_utils_SMKeyManUtils_keyPairWithDescription(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path, jstring description);
/**
 * @return null 失败   其它：string--char* key
 */
JNIEXPORT jstring JNICALL Java_com_tenpay_utils_SMKeyManUtils_keyWithDescription(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path, jstring description);

JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_importKeyWithDescription(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path, jstring description, jstring key,
    jint force_update);
JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_importKeyPairWithDescription(
    JNIEnv *, jobject, jobjectArray factors, jstring dir_path, jstring description, jstring pubkey,
    jstring prikey, jint force_update);

JNIEXPORT jint JNICALL Java_com_tenpay_utils_SMKeyManUtils_delDescription(JNIEnv *, jobject,
                                                                          jobjectArray factors,
                                                                          jstring dir_path,
                                                                          jstring description);
void byte2str(unsigned char *bytes, int len, char *strbuf);
#endif  // JNI_INTERFACE

#ifdef __cplusplus
}  // extern "C"
#endif

#endif // TENCENTSM_LITE_SOURCE_TC_KMS_H_

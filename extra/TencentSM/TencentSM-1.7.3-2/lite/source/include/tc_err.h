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

#ifndef __TC_ERR_H__
#define __TC_ERR_H__

//error code:OK
#define ERR_TENCENTSM_OK            0
//通用错误 -10001~-10999
#define ERR_ILLEGAL_ARGUMENT  -10001 //专用于用户接口输入的参数问题
#define ERR_TC_MALLOC         -10002 //malloc失败
#define ERR_REMOVE_FILE       -10003 //删除文件失败
#define ERR_TIMETRANSFER      -10004
#define ERR_MODULE_STATUSNG   -10900 //模块状态错误
//ASN1错误 -11001~-11999
#define ERR_ASN1_FORMAT_ERROR -11001 //ASN1格式错误
#define ERR_ASN1_DECODE_OBJ   -11002 //ASN1对象解码失败
#define ERR_ASN1_CREATE_ELEMENT  -11003
#define ERR_PEM_FORMAT_ERR   -11004 //pem格式不支持或错误 
/*****************************************/
/* Errors returned by libtasn1 functions */
/*****************************************/
// #define ASN1_SUCCESS			ERR_TENCENTSM_OK
// #define ASN1_FILE_NOT_FOUND		-11999
// #define ASN1_ELEMENT_NOT_FOUND		2   //from libtasn1
// #define ASN1_IDENTIFIER_NOT_FOUND	-11997
// #define ASN1_DER_ERROR			-11996
// #define ASN1_VALUE_NOT_FOUND		-11995
// #define ASN1_GENERIC_ERROR		-11994
// #define ASN1_VALUE_NOT_VALID		-11993
// #define ASN1_TAG_ERROR			-11992
// #define ASN1_TAG_IMPLICIT		-11991
// #define ASN1_ERROR_TYPE_ANY		-11990
// #define ASN1_SYNTAX_ERROR		-11989
// #define ASN1_MEM_ERROR			-11988
// #define ASN1_MEM_ALLOC_ERROR		-11987
// #define ASN1_DER_OVERFLOW		-11986
// #define ASN1_NAME_TOO_LONG		-119895
// #define ASN1_ARRAY_ERROR		-11984
// #define ASN1_ELEMENT_NOT_EMPTY		-11983
// #define ASN1_TIME_ENCODING_ERROR	-11982
// #define ASN1_RECURSION			-11981
#define ERR_ASN1_NORMAL         -11980
//CSR错误 -12001~-12999
#define ERR_CSR_ENCODE_ERROR  -12003
#define ERR_CSR_PEM_FORMAT_ERROR  -12004
#define ERR_CSR_PARSE_ERROR  -12005
#define ERR_CSR_VERIFY  -12006
//证书错误
#define ERR_CERT_IS_NOT_SM2 -12010
#define ERR_CERT_OID_NOT_FOUND -12011
#define ERR_CERT_IMPORT_ERR -12012
#define ERR_CERT_FILE_ERR -12013
#define ERR_CERT_EMPTY -12014
#define ERR_CERT_REMOVE_ERR -12015
#define ERR_CERT_EXIST -12016
#define ERR_CERT_VERIFY_FAILED -12017
#define ERR_CERT_NOT_EXIST -12018
#define ERR_CERT_PEM_FORMAT_ERROR  -12019
#define ERR_CERT_GEN_ERR  -12020
#define ERR_CERT_ADD_EXT  -12021
#define ERR_CERT_PARSE_ERR  -12022
#define ERR_CERT_CA_ERR    -12023
#define ERR_CERT_GEN_AUTHKEY  -12024
#define ERR_CERT_GEN_SUBJKEY  -12025
#define ERR_CERT_GEN_KEYUSE   -12026
#define ERR_CERT_SUBJ_MODIFY  -12027
#define ERR_CERT_SET_PKCSNAME -12028
#define ERR_CERT_CREATE_DIRSTR -12029
#define ERR_CERT_GETITEM_ERR   -12030
#define ERR_CERT_GETSERIALNUM  -12031
#define ERR_CERT_GETDN_ITEM    -12032
#define ERR_CERT_READVALUE     -12033
#define ERR_CERT_CERTVERIFY    -12034
#define ERR_CERT_CERT_NODIR    -12035

//ECC compute error -13001~-13999
#define ERR_ECC_NID_NO_PARAMS     -13001
#define ERR_ECC_POINTS_MUL        -13010
#define ERR_ECC_POINT_NOTINCURVE  -13011
//BN compute error -14001~-14999
#define ERR_BN_INVERT         -14001
#define ERR_BN_SECTION        -14002

//SM3 错误 -20001~29999
#define ERR_SM3_KDF_ARGUMENT  -20001
#define ERR_SM3_DIGEST_ARGUMENT  -20002
#define ERR_SM3_HMAC_ARGUMENT  -20003
#define ERR_SM3_HMAC_ERR       -20004
//KMS error -30001~-39999
#define KMS_STATUS_SUCCESS                      ERR_TENCENTSM_OK
#define KMS_STATUS_UNAUTH                       -30001
#define KMS_STATUS_AUTHED                       -30002
#define KMS_STATUS_DESCRIPTION_EXISTED          -30003
#define KMS_STATUS_DESCRIPTION_NOT_EXISTED      -30004
#define KMS_STATUS_DESCRIPTION_ARGUMENT_ERROR   -30005
#define KMS_STATUS_PATH_NOT_EXIST               -30006
#define KMS_STATUS_PATH_EXISTED                 -30007
#define KMS_STATUS_UNKNOWN                      -39999

//SM4错误 -40001~-49999
#define ERR_SM4_PKCS7_PADDING_VERIFY_FAILED -40001
#define ERR_SM4_GCM_TAG_VERIFY_FAILED       -40002
#define ERR_SM4_GCM_ILLEGAL_AADLEN          -40003
#define ERR_SM4_GCM_ILLEGAL_MSGLEN          -40004
#define ERR_SM4_GCM_ILLEGAL_TAGLEN          -40005
#define ERR_SM4_ECB_ILLEGAL_MSGLEN          -40006

//SM2错误 -50001~-59999
//SM2 exchange errors -50001~-50999
#define ERR_SM2_EXCHANGE_POINT_NOT_ON_CURVE -50001
#define ERR_SM2_EXCHANGE_tA -50002
#define ERR_SM2_EXCHANGE_U  -50003

#define ERR_SM2_DECODE       -51001
#define ERR_SM2_DEC_TBN_IS_Z -51002
#define ERR_SM2_VERIFY       -51003

#define ERR_UNKNOWN -90001

#endif /* TC_ERR_H */

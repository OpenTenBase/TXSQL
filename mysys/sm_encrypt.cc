/* Copyright (c) 2002, 2021, Tencent and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2.0,
 as published by the Free Software Foundation.

 This program is also distributed with certain software (including
 but not limited to OpenSSL) that is licensed under separate terms,
 as designated in a particular file or component or in included license
 documentation.  The authors of MySQL hereby grant you an additional
 permission to link the program and your derivative works with the
 separately licensed software that they have included with MySQL.

 Without limiting anything contained in the foregoing, this file,
 which is part of C Driver for MySQL (Connector/C), is also subject to the
 Universal FOSS Exception, version 1.0, a copy of which can be found at
 http://oss.oracle.com/licenses/universal-foss-exception.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License, version 2.0, for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sm_encrypt.h"

/* If bad data discovered during decoding */

void Hex2Str(const unsigned char *sSrc, unsigned char *sDest, int nSrcLen) {
  int i;
  char szTmp[3];

  for (i = 0; i < nSrcLen; i++) {
    sprintf(szTmp, "%02X", sSrc[i]);
    memcpy(&sDest[i * 2], szTmp, 2);
  }
  return;
}

void print_bytes(const unsigned char *beg, const unsigned char *end) {
	const unsigned char *pbeg = beg, *pend = end;
    while (pbeg != pend) fprintf(stderr,"%02x", (unsigned)*pbeg++);
    fprintf(stderr,"\n");
}

int Sm3Hmac(unsigned char *data, int dataLen, unsigned char *hmac, int *hmacLen,
            unsigned char *hmacKey, int keyLen) {
  if (data == NULL || dataLen <= 0) {
    return MY_SM_BAD_DATA;
  }
  if (hmac == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (hmacLen == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (hmacKey == NULL || keyLen < 0) {
    return MY_SM_BAD_DATA;
  }

  const EVP_MD *md = EVP_sm3();
  unsigned int len;
  HMAC_CTX *hmac_ctx = HMAC_CTX_new();
  HMAC_Init_ex(hmac_ctx, hmacKey, keyLen, md, NULL);
  HMAC_Update(hmac_ctx, data, dataLen);
  HMAC_Final(hmac_ctx, hmac, &len);
  *hmacLen = len;
  HMAC_CTX_free(hmac_ctx);

  return 0;
}

int Sm3Digest(unsigned char *data, int dataLen, unsigned char *digest,
              int *digestLen) {
  if (data == NULL || dataLen <= 0) {
    return MY_SM_BAD_DATA;
  }
  if (digest == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (digestLen == NULL) {
    return MY_SM_BAD_DATA;
  }

  const EVP_MD *md = EVP_sm3();
  EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
  EVP_DigestInit(md_ctx, md);
  EVP_DigestUpdate(md_ctx, data, dataLen);
  EVP_DigestFinal_ex(md_ctx, digest, (unsigned int *)digestLen);
  EVP_MD_CTX_free(md_ctx);

  return 0;
}

int Sm4CbcEncrypt(unsigned char *source, int sourceLength,
                  unsigned char *cipherText, int *cipherLength,
                  unsigned char *key, unsigned char *iv, bool padding) {
  if (source == NULL || sourceLength <= 0) {
    return MY_SM_BAD_DATA;
  }
  if (cipherText == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (cipherLength == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (key == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (iv == NULL) {
    return MY_SM_BAD_DATA;
  }

  int tmplen = 0, clen = 0;
  const EVP_CIPHER *cipher = NULL;
  cipher = EVP_sm4_cbc();

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_EncryptInit(ctx, cipher, key, iv);
  if (!EVP_CIPHER_CTX_set_padding(ctx, padding)) {
    return MY_SM_BAD_DATA;
  }
  EVP_EncryptUpdate(ctx, cipherText, &clen, source, sourceLength);
  EVP_EncryptFinal_ex(ctx, cipherText + clen, &tmplen);
  clen += tmplen;
  EVP_CIPHER_CTX_free(ctx);
  *cipherLength = clen;

  return 0;
}

int Sm4CbcDecrypt(unsigned char *source, int sourceLength,
                  unsigned char *plainText, int *plainTextLength,
                  unsigned char *key, unsigned char *iv, bool padding) {
  if (source == NULL || sourceLength <= 0) {
    return MY_SM_BAD_DATA;
  }
  if (plainText == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (plainTextLength == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (key == NULL) {
    return MY_SM_BAD_DATA;
  }
  if (iv == NULL) {
    return MY_SM_BAD_DATA;
  }

  int tmplen = 0, plen = 0;
  const EVP_CIPHER *cipher = NULL;
  cipher = EVP_sm4_cbc();

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  EVP_DecryptInit(ctx, cipher, key, iv);
  if (!EVP_CIPHER_CTX_set_padding(ctx, padding)) {
    return MY_SM_BAD_DATA;
  }
  EVP_DecryptUpdate(ctx, plainText, &plen, source, sourceLength);
  EVP_DecryptFinal_ex(ctx, plainText + plen, &tmplen);
  plen += tmplen;
  EVP_CIPHER_CTX_free(ctx);
  *plainTextLength = plen;

  return 0;
}


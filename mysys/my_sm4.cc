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

/**
  @file mysys/my_sm4.cc
*/

#include "my_sm4.h"
#include <sm.h>
#include <stdio.h>
// #include "sm_encrypt.h"

/* SM4 encryption must have iv, give a default one if not provide */
static unsigned char my_sm4_default_iv[16] = {
    0x99, 0xaa, 0x3e, 0x68, 0xed, 0x81, 0x73, 0xa0,
    0xee, 0xd0, 0x66, 0x84, 0xee, 0xd0, 0x66, 0x84};

int my_sm4_encrypt(unsigned char *source, int source_length,
                   unsigned char *cipher_text, int *cipher_length,
                   unsigned char *key, unsigned char *iv, bool padding) {
  if (nullptr == iv) {
    iv = my_sm4_default_iv;
  }

  if (source == NULL || source_length <= 0) {
    return (-1);
  }

  if (cipher_text == NULL) {
    return (-1);
  }

  if (cipher_length == NULL) {
    return (-1);
  }

  if (key == NULL) {
    return (-1);
  }

  if (iv == NULL) {
    return (-1);
  }

  int ret = 0;
  size_t length_tmp = 0;
  if (padding) {
    ret = SM4_CBC_Encrypt(source, static_cast<size_t>(source_length),
                          cipher_text, &length_tmp, key, iv);
  } else {
    ret = SM4_CBC_Encrypt_NoPadding(source, static_cast<size_t>(source_length),
                                    cipher_text, &length_tmp, key, iv);
  }
  if (ret == 0) *cipher_length = length_tmp;
  return ret;
}

int my_sm4_decrypt(unsigned char *source, int source_length,
                   unsigned char *plain_text, int *plaintext_length,
                   unsigned char *key, unsigned char *iv, bool padding) {
  if (nullptr == iv) {
    iv = my_sm4_default_iv;
  }

  if (source == NULL || source_length <= 0) {
    return (-1);
  }

  if (plain_text == NULL) {
    return (-1);
  }

  if (plaintext_length == NULL) {
    return (-1);
  }

  if (key == NULL) {
    return (-1);
  }

  if (iv == NULL) {
    return (-1);
  }

  int ret = 0;
  size_t length_tmp = 0;
  if (padding) {
    ret = SM4_CBC_Decrypt(source, static_cast<size_t>(source_length),
                          plain_text, &length_tmp, key, iv);
  } else {
    ret = SM4_CBC_Decrypt_NoPadding(source, static_cast<size_t>(source_length),
                                    plain_text, &length_tmp, key, iv);
  }
  if (ret == 0) *plaintext_length = length_tmp; 
  return ret;
}

int my_sm4_get_size(int source_length) {
  return  (MY_SM4_CBC_BLOCK_SIZE * (source_length / MY_SM4_CBC_BLOCK_SIZE) +
          MY_SM4_CBC_BLOCK_SIZE);
}

int my_sm3_digest(unsigned char *data, int data_len,
                  unsigned char *digest, int *digest_len) {
  if (data == NULL || data_len <= 0) {
    printf("Sm3Hmac data is NULL");
    return (-1);
  }

  if (digest == NULL) {
    printf("digest is NULL");
    return (-1);
  }

  if (digest_len == NULL) {
    printf("digestLen is NULL");
    return (-1);
  }

  int ret = SM3(data, static_cast<size_t>(data_len), digest);
  if (ret == 0) *digest_len = SM3_HMAC_SIZE;

  return ret;
}

int my_sm3_hmac(unsigned char *data, int data_len,
                unsigned char *hmac, int *hmac_len,
                unsigned char *hmac_key, int key_len) {
  if (data == NULL || data_len <= 0) {
    printf("Sm3Hmac data is NULL");
    return (-1);
  }

  if (hmac == NULL) {
    printf("hmac is NULL");
    return (-1);
  }

  if (hmac_len == NULL) {
    printf("hmac_len is NULL");
    return (-1);
  }

  if (hmac_key == NULL || key_len < 0) {
    printf("hmacKey is NULL");
    return (-1);
  }

  int ret = SM3_HMAC(data, data_len, hmac_key, key_len, hmac);
  if (ret == 0) *hmac_len = SM3_HMAC_SIZE;

  return ret;
}

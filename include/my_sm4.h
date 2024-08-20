#ifndef MY_SM4_INCLUDED
#define MY_SM4_INCLUDED

/* Copyright (c) 2000, 2021, Tencent and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2.0,
 as published by the Free Software Foundation.

 This program is also distributed with certain software (including
 but not limited to OpenSSL) that is licensed under separate terms,
 as designated in a particular file or component or in included license
 documentation.  The authors of MySQL hereby grant you an additional
 permission to link the program and your derivative works with the
 separately licensed software that they have included with MySQL.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License, version 2.0, for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

/**
  @file include/my_sm4.h
  Wrapper to give simple interface for MySQL to SM4 national secret standard encryption.
*/

/**
  Encrypt a buffer using SM4
 
  @param [in] source           Pointer to data for encryption
  @param [in] source_length    Size of encryption data
  @param [out] cipher_text     Buffer to place encrypted data (must be large enough)
  @param [out] cipher_length   Length of the encrypt data
  @param [in] key              Key to be used for encryption
  @param [in] iv               initialization vector if needed. Otherwise NULL
  @param [in] padding          if padding needed
  @return                      0 is ok, otherwise error 
*/

int my_sm4_encrypt(unsigned char *source, int source_length,
                   unsigned char *cipher_text, int *cipher_length,
                   unsigned char *key, unsigned char *iv, bool padding);

/**
  Decrypt an SM4 encrypted buffer

  @param source         Pointer to data for decryption
  @param source_length  size of encrypted data
  @param plain_text     buffer to place decrypted data (must be large enough)
  @param plaintext_length     Length of the plaintext.
  @param key            Key to be used for decryption
  @param iv             initialization vector if needed. Otherwise NULL
  @return 0 is ok, Otherwise error.
*/

int my_sm4_decrypt(unsigned char *source, int source_length,
                   unsigned char *plain_text, int *plaintext_length,
                   unsigned char *key, unsigned char *iv, bool padding);

#endif

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
#include "sm_encrypt.h"

/* SM4 encryption must have iv, give a default one if not provide */
static unsigned char my_sm4_default_iv[16] =
         { 0x99, 0xaa, 0x3e, 0x68, 0xed, 0x81, 0x73, 0xa0,
           0xee, 0xd0, 0x66, 0x84, 0xee, 0xd0, 0x66, 0x84 };

int my_sm4_encrypt(unsigned char *source, int source_length,
                   unsigned char *cipher_text, int *cipher_length,
                   unsigned char *key, unsigned char *iv, bool padding) {
  if (nullptr == iv) {
    iv = my_sm4_default_iv;
  }
  return Sm4CbcEncrypt(source, source_length,
                       cipher_text, cipher_length, key, iv, padding); 
}

int my_sm4_decrypt(unsigned char *source, int source_length,
                   unsigned char *plain_text, int *plaintext_length,
                   unsigned char *key, unsigned char *iv, bool padding) {
  if (nullptr == iv) {
    iv = my_sm4_default_iv;
  }
  return Sm4CbcDecrypt(source, source_length,
                       plain_text, plaintext_length, key, iv, padding);
}

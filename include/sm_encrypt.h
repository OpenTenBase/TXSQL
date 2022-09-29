#ifndef SM_ENCRYPT_INCLUDED
#define SM_ENCRYPT_INCLUDED

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

#include <openssl/err.h>
#include "openssl/evp.h"
#include "openssl/rand.h"
#include "openssl/ossl_typ.h"
#include "openssl/rsa.h"
#include "openssl/ec.h"
#include "openssl/hmac.h"
#include "openssl/x509.h"
#include "string.h"

#define MY_SM_BAD_DATA -1

void print_bytes(const unsigned char *beg, const unsigned char *end);

void Hex2Str( const unsigned char *sSrc,  unsigned char *sDest, int nSrcLen );

int Sm3Digest(unsigned char *data,int dataLen,unsigned char *digest,int *digestLen);

int Sm3Hmac(unsigned char *data,int dataLen,unsigned char *hmac,int *hmacLen,
				unsigned char *hmacKey,int keyLen);

int Sm4CbcEncrypt(unsigned char *source,int sourceLength,
				unsigned char *cipherText,int *cipherLength,
				unsigned char *key,unsigned char *iv, bool padding = true);

int Sm4CbcDecrypt(unsigned char *source,int sourceLength,
				unsigned char *plainText,int *plainTextLength,
				unsigned char *key,unsigned char *iv, bool padding = true);
#endif /* SM_ENCRYPT_INCLUDED */


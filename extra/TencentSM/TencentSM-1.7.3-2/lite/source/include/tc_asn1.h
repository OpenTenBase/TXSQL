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


#ifndef TC_ANS1_H
#define TC_ANS1_H

#include "../include/tc_sm2.h"

void tc_asn1_encode_integer(const unsigned char* in,int inlen,unsigned char* out,int* outlen);
void tc_asn1_encode_sequence(const unsigned char* in,int inlen,unsigned char* out,int* outlen);
void tc_asn1_encode_octet_string(const unsigned char* in,int inlen,unsigned char* out,int* outlen);

int tc_asn1_decode_object(const unsigned char* in,int inlen,int* offset,int* outlen);

int tc_asn1_encode_sm2_cipher_c1c3c2(const sm2_cipher* cipher,unsigned char* out,size_t* outlen);
int tc_asn1_decode_sm2_cipher_c1c3c2(const unsigned char* in,int inlen,unsigned char* cipher);

int tc_asn1_encode_sm2_cipher_c1c2c3(const sm2_cipher* cipher,unsigned char* out,size_t* outlen);
int tc_asn1_decode_sm2_cipher_c1c2c3(const unsigned char* in,int inlen,unsigned char* cipher);

#endif /* TC_ANS1_H */

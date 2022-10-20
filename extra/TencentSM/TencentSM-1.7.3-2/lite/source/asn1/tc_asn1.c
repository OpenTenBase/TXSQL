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

#include "../include/tc_asn1.h"
#include "../include/tc_str.h"
#include "../include/tc_err.h"
#include <string.h>

void tc_asn1_encode(unsigned char tag,const unsigned char* in,int inlen,unsigned char* out,int* outlen)
{
    if (tag == 0x02) {
      /* encode integer */
      int _inlen = inlen;
      const unsigned char* _in = in;
      for (int i = 0; i < _inlen; i++) {
        if (_in[i] == 0x00) {
          inlen = inlen - 1;
          in = in + 1;
        }else{
          break;
        }
      }
    }
  
    *outlen = 0;
    int offset = 0;
    
    memset(out, tag, 1);offset += 1;
    
    if (tag == 0x02 && (in[0] & 0x80) == 0x80) {
        /* high bit 1 */
        inlen++;
    }
    
    if (inlen > 0x7f) {
        
        char L_length = 0x01;
        
        if (inlen > 0xff) {
            L_length ++;
        }
        if (inlen > 0xffff) {
            L_length ++;
        }
        if (inlen > 0xffffff) {
            L_length ++;
        }
        
        memset(out + offset, 0x80 | L_length, 1);offset += 1;

        if (L_length >= 4) {
            char remainder = inlen/(0xffffff + 1);
            memset(out + offset, remainder, 1);offset += 1;
        }
        if (L_length >= 3) {
            char remainder = inlen/(0xffff + 1);
            memset(out + offset, remainder, 1);offset += 1;
        }
        if (L_length >= 2) {
            char remainder = inlen/(0xff + 1);
            memset(out + offset, remainder, 1);offset += 1;
        }
        if (L_length >= 1) {
            char remainder = inlen%(0xff + 1);
            memset(out + offset, remainder, 1);offset += 1;
        }
        
    }else{
        memset(out + offset, inlen, 1);offset += 1;
    }
    
    if (tag == 0x02 && (in[0] & 0x80) == 0x80) {
        /* high bit 1 */
        memset(out + offset, 0x00, 1);offset += 1;
        memcpy(out + offset, in, inlen - 1);offset += (inlen - 1);
    }else{
        memcpy(out + offset, in, inlen);offset += inlen;
    }
    *outlen = offset;
}

void tc_asn1_encode_integer(const unsigned char* in,int inlen,unsigned char* out,int* outlen)
{
    tc_asn1_encode(0x02, in, inlen, out,outlen);
}

void tc_asn1_encode_sequence(const unsigned char* in,int inlen,unsigned char* out,int* outlen)
{
    tc_asn1_encode(0x30, in, inlen, out,outlen);
}

void tc_asn1_encode_octet_string(const unsigned char* in,int inlen,unsigned char* out,int* outlen)
{
    tc_asn1_encode(0x04, in, inlen, out,outlen);
}

int tc_asn1_decode_object(const unsigned char* in,int inlen,int* offset,int* outlen)
{
    int cursor = 0;
    switch (in[cursor]) {
        case 0x30:
        case 0x02:
        case 0x04:
            cursor++;
            if ((in[cursor] & 0x80) == 0x80) {
                char L_length = in[cursor] & 0x7f;cursor++;
                unsigned long length = 0;
                if (L_length >= 4) {
                    length += in[cursor] * (0xffffff + 1);cursor++;
                }
                if (L_length >= 3) {
                    length += in[cursor] * (0xffff + 1);cursor++;
                }
                if (L_length >= 2) {
                    length += in[cursor] * (0xff + 1);cursor++;
                }
                if (L_length >= 1) {
                    length += in[cursor] * (0x00 + 1);cursor++;
                }
                *offset = cursor;
                *outlen = (int)length;
            }else{
                int length = in[cursor];cursor++;
                *offset = cursor;
                *outlen = (int)length;
            }
            return ERR_TENCENTSM_OK;
        default:
            *offset = 0;
            *outlen = 0;
            return ERR_ASN1_DECODE_OBJ;
    }
}

int tc_asn1_encode_sm2_cipher_c1c3c2(const sm2_cipher* cipher,unsigned char* out,size_t* outlen)
{
  unsigned char x_encode[40];
  int x_encode_len = 0;

  tc_asn1_encode_integer(cipher->XCoordinate, TC_ECCref_MAX_LEN, x_encode, &x_encode_len);
  
  unsigned char y_encode[40];
  int y_encode_len = 0;
  tc_asn1_encode_integer(cipher->YCoordinate, TC_ECCref_MAX_LEN, y_encode, &y_encode_len);
  
  unsigned char hash_encode[40];
  int hash_encode_len = 0;
  tc_asn1_encode_octet_string(cipher->HASH, SM3_DIGEST_SIZE, hash_encode, &hash_encode_len);
  
  
  unsigned char* cipher_encode = (unsigned char*)tcsm_tc_secure_malloc(cipher->CipherLen + 8);
  int cipher_encode_len = 0;
  tc_asn1_encode_octet_string(cipher->Cipher, (int)cipher->CipherLen, cipher_encode, &cipher_encode_len);
  
  
  unsigned char* seq_encode = (unsigned char*)tcsm_tc_secure_malloc(cipher->CipherLen + 8 + 120);
  memcpy(seq_encode, x_encode, x_encode_len);
  memcpy(seq_encode + x_encode_len , y_encode, y_encode_len);
  memcpy(seq_encode + x_encode_len + y_encode_len , hash_encode, hash_encode_len);
  memcpy(seq_encode + x_encode_len + y_encode_len + hash_encode_len , cipher_encode, cipher_encode_len);
  
  int encode_len = 0;
  tc_asn1_encode_sequence(seq_encode, x_encode_len + y_encode_len + hash_encode_len + cipher_encode_len, out, &encode_len);
  *outlen = encode_len;
  
  tcsm_tc_secure_free(cipher_encode);
  tcsm_tc_secure_free(seq_encode);
  
  return 0;
}

int tc_asn1_encode_sm2_cipher_c1c2c3(const sm2_cipher* cipher,unsigned char* out,size_t* outlen)
{
  unsigned char x_encode[40];
  int x_encode_len = 0;

  tc_asn1_encode_integer(cipher->XCoordinate, TC_ECCref_MAX_LEN, x_encode, &x_encode_len);
  
  unsigned char y_encode[40];
  int y_encode_len = 0;
  tc_asn1_encode_integer(cipher->YCoordinate, TC_ECCref_MAX_LEN, y_encode, &y_encode_len);
    
  
  unsigned char* cipher_encode = (unsigned char*)tcsm_tc_secure_malloc(cipher->CipherLen + 8);
  int cipher_encode_len = 0;
  tc_asn1_encode_octet_string(cipher->Cipher, (int)cipher->CipherLen, cipher_encode, &cipher_encode_len);
  
  unsigned char hash_encode[40];
  int hash_encode_len = 0;
  tc_asn1_encode_octet_string(cipher->HASH, SM3_DIGEST_SIZE, hash_encode, &hash_encode_len);
  
  
  unsigned char* seq_encode = (unsigned char*)tcsm_tc_secure_malloc(cipher->CipherLen + 8 + 120);
  memcpy(seq_encode, x_encode, x_encode_len);
  memcpy(seq_encode + x_encode_len , y_encode, y_encode_len);
  memcpy(seq_encode + x_encode_len + y_encode_len , cipher_encode, cipher_encode_len);
  memcpy(seq_encode + x_encode_len + y_encode_len + cipher_encode_len, hash_encode, hash_encode_len);
  
  int encode_len = 0;
  tc_asn1_encode_sequence(seq_encode, x_encode_len + y_encode_len + hash_encode_len + cipher_encode_len, out, &encode_len);
  *outlen = encode_len;
  
  tcsm_tc_secure_free(cipher_encode);
  tcsm_tc_secure_free(seq_encode);
  
  return 0;
}

int tc_asn1_decode_sm2_cipher_c1c3c2(const unsigned char* in,int inlen,unsigned char* cipher)
{
  if (in[0] != 0x30) {
    return ERR_ASN1_FORMAT_ERROR;
  }
  
  int seq_offset = 0;
  int seq_outlen = 0;
  
  int ret = tc_asn1_decode_object(in, (int)inlen, &seq_offset, &seq_outlen);
  
  if (ret != ERR_TENCENTSM_OK) {
    LOGV("sm2 decrypt argument format error. \n", ret);
    return ret;
  }else{
    
    int x_offset = 0;
    int x_outlen = 0;
    
    const unsigned char* ptr = in + seq_offset;
    
    if (ptr[0] != 0x02) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen, &x_offset, &x_outlen);
    
    if (ret != ERR_TENCENTSM_OK || x_outlen > 33 || x_outlen <= 0) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ptr += x_offset;
    if (x_outlen == 33) {
      memcpy(cipher, ptr + 1, 32);
    }else if(x_outlen == 32){
      memcpy(cipher, ptr, 32);
    }else{
      int z = 32 - x_outlen;
      memset(cipher, 0x00, z);
      memcpy(cipher + z, ptr, x_outlen);
    }
    
    ptr += x_outlen;
    
    int y_offset = 0;
    int y_outlen = 0;
    
    if (ptr[0] != 0x02) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen - x_offset - x_outlen, &y_offset, &y_outlen);
    
    if (ret != ERR_TENCENTSM_OK || y_outlen > 33) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ptr += y_offset;
    
    if (y_outlen == 33) {
      memcpy(cipher + 32, ptr + 1, 32);
    }else if(y_outlen == 32){
      memcpy(cipher + 32, ptr, 32);
    }else{
      int z = 32 - y_outlen;
      memset(cipher + 32, 0x00, z);
      memcpy(cipher + 32 + z, ptr, y_outlen);
    }
    
    ptr += y_outlen;
    
    int hash_offset = 0;
    int hash_outlen = 0;
    
    if (ptr[0] != 0x04) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen - x_offset - x_outlen - y_offset - y_outlen, &hash_offset, &hash_outlen);
    
    if (ret != ERR_TENCENTSM_OK || hash_outlen != 32) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ptr += hash_offset;
    
    memcpy(cipher + 64, ptr, 32);
    
    ptr += hash_outlen;
    
    int cipher_offset = 0;
    int cipher_outlen = 0;
    
    if (ptr[0] != 0x04) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen - x_offset - x_outlen - y_offset - y_outlen - hash_offset - hash_outlen, &cipher_offset, &cipher_outlen);
    
    if (ret != ERR_TENCENTSM_OK) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ret;
    }
    
    ptr += cipher_offset;
    
    memcpy(cipher + 96, (char*)&cipher_outlen, sizeof(unsigned int));
    memcpy(cipher + 96 + sizeof(unsigned int), ptr, cipher_outlen);
  }
  return ERR_TENCENTSM_OK;
}

int tc_asn1_decode_sm2_cipher_c1c2c3(const unsigned char* in,int inlen,unsigned char* cipher)
{
  if (in[0] != 0x30) {
    return ERR_ASN1_FORMAT_ERROR;
  }
  
  int seq_offset = 0;
  int seq_outlen = 0;
  
  int ret = tc_asn1_decode_object(in, (int)inlen, &seq_offset, &seq_outlen);
  
  if (ret != ERR_TENCENTSM_OK) {
    LOGV("sm2 decrypt argument format error. \n", ret);
    return ret;
  }else{
    
    int x_offset = 0;
    int x_outlen = 0;
    
    const unsigned char* ptr = in + seq_offset;
    
    if (ptr[0] != 0x02) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen, &x_offset, &x_outlen);
    
    if (ret != ERR_TENCENTSM_OK || x_outlen > 33 || x_outlen <= 0) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ptr += x_offset;
    if (x_outlen == 33) {
      memcpy(cipher, ptr + 1, 32);
    }else if(x_outlen == 32){
      memcpy(cipher, ptr, 32);
    }else{
      int z = 32 - x_outlen;
      memset(cipher, 0x00, z);
      memcpy(cipher + z, ptr, x_outlen);
    }
    
    ptr += x_outlen;
    
    int y_offset = 0;
    int y_outlen = 0;
    
    if (ptr[0] != 0x02) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen - x_offset - x_outlen, &y_offset, &y_outlen);
    
    if (ret != ERR_TENCENTSM_OK || y_outlen > 33) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ptr += y_offset;
    
    if (y_outlen == 33) {
      memcpy(cipher + 32, ptr + 1, 32);
    }else if(y_outlen == 32){
      memcpy(cipher + 32, ptr, 32);
    }else{
      int z = 32 - y_outlen;
      memset(cipher + 32, 0x00, z);
      memcpy(cipher + 32 + z, ptr, y_outlen);
    }
    
    ptr += y_outlen;

    //---- decode cipher text
    int cipher_offset = 0;
    int cipher_outlen = 0;
    
    if (ptr[0] != 0x04) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen - x_offset - x_outlen - y_offset - y_outlen, &cipher_offset, &cipher_outlen);
    
    if (ret != ERR_TENCENTSM_OK) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ret;
    }
    
    ptr += cipher_offset;
    
    memcpy(cipher + 96, (char*)&cipher_outlen, sizeof(unsigned int));
    memcpy(cipher + 96 + sizeof(unsigned int), ptr, cipher_outlen);
    
    ptr += cipher_outlen;
    
    //---- decode hash value
    int hash_offset = 0;
    int hash_outlen = 0;
    
    if (ptr[0] != 0x04) {
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ret = tc_asn1_decode_object(ptr, seq_outlen - x_offset - x_outlen - y_offset - y_outlen - cipher_offset - cipher_outlen, &hash_offset, &hash_outlen);
    
    if (ret != ERR_TENCENTSM_OK || hash_outlen != 32) {
      LOGV("sm2 decrypt argument sig format error. \n", ret);
      return ERR_ASN1_FORMAT_ERROR;
    }
    
    ptr += hash_offset;
    
    memcpy(cipher + 64, ptr, 32);
    
    
  }
  return ERR_TENCENTSM_OK;
}

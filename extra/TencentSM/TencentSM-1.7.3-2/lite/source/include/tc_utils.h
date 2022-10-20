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

#ifndef __UTILS_H__
#define __UTILS_H__

#include "tc.h"
#include <stdio.h>

#ifdef CPU_BIGENDIAN

#define cpu_to_be16(v) (v)
#define cpu_to_be32(v) (v)
#define be16_to_cpu(v) (v)
#define be32_to_cpu(v) (v)

#else

#define cpu_to_le16(v) (v)
#define cpu_to_le32(v) (v)
#define le16_to_cpu(v) (v)
#define le32_to_cpu(v) (v)

#define cpu_to_be16(v) (((v)<< 8) | ((v)>>8))
#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))
#define be16_to_cpu(v) cpu_to_be16(v)
#define be32_to_cpu(v) cpu_to_be32(v)

#endif

void tcsm_private_key_get_str(char *str, tc_bn_t prik);
void tcsm_private_key_set_str(const char *str, tc_bn_t prik);
void tcsm_public_key_get_str(char *str, tc_ec_t pubk);
void tcsm_public_key_set_str(const char *str, tc_ec_t pubk);

long tcsm_get_file_size(FILE * file_handle);

void tcsm_csr_convert_der_2_pem(const unsigned char* der,size_t derlen,char* pem,size_t * pemlen);
void tcsm_crt_convert_der_2_pem(const unsigned char* der,size_t derlen,char* pem,size_t * pemlen);
void tcsm_csr_convert_pem_2_der(const char* pem,unsigned char* der,size_t * derlen);
void tcsm_crt_convert_pem_2_der(const char* pem,unsigned char* der,size_t * derlen);

int getTMFromTime(time_t* time, struct tm* pTM);
int tc_pem_2_der(const char* pem,unsigned char* der,size_t * derlen,const char* begin_str,const char* end_str);
void tc_der_2_pem(const unsigned char* der,size_t derlen,char* pem,size_t * pemlen,const char* begin_str,const char* end_str);

unsigned long get_tick_count();
#endif

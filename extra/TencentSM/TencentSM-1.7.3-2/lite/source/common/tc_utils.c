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

// #include "../tc.h"
#include "../include/tc_str.h"
#include "../include/tc_utils.h"
#include "../include/b64.h"

#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define SM2_COORDINATE_LEN 64
#define POINT_UNCOMPRESSED "04"

void tcsm_private_key_get_str(char *str, tc_bn_t prik)
{
  tcsm_tc_bn_get_str(str,prik);
  
  int length = (int)strlen(str);
  
  if (length != SM2_COORDINATE_LEN) {
    
    char tmp[SM2_COORDINATE_LEN + 1] = {0};
    memset(tmp, '0', SM2_COORDINATE_LEN - length);
    memcpy(tmp + SM2_COORDINATE_LEN - length, str, length);
    memcpy(str, tmp, SM2_COORDINATE_LEN + 1);
  }
}

void tcsm_private_key_set_str(const char *str, tc_bn_t prik)
{
  char c_pri[SM2_COORDINATE_LEN + 1] = {0};
  memcpy(c_pri, str, SM2_COORDINATE_LEN);
  tcsm_tc_bn_set_str(prik, c_pri, 16);
}

void tcsm_public_key_get_str(char *str, tc_ec_t pubk)
{
  const unsigned char flag[] = POINT_UNCOMPRESSED;
  memcpy(str, flag, 2);
  
  char x[SM2_COORDINATE_LEN + 1] = {0};
  char y[SM2_COORDINATE_LEN + 1] = {0};
  
  tcsm_tc_ec_get_str(x, y, pubk);
  
  int x_len = (int)strlen(x);
  int y_len = (int)strlen(y);
  
  if (x_len != 64) {
    char t_x[SM2_COORDINATE_LEN + 1] = {0};
    memset(t_x, '0', SM2_COORDINATE_LEN - x_len);
    memcpy(t_x + SM2_COORDINATE_LEN - x_len, x, x_len);
    memcpy(x, t_x, SM2_COORDINATE_LEN);
  }
  if (y_len != 64) {
    char t_y[SM2_COORDINATE_LEN + 1] = {0};
    memset(t_y, '0', SM2_COORDINATE_LEN - y_len);
    memcpy(t_y + SM2_COORDINATE_LEN - y_len, y, y_len);
    memcpy(y, t_y, SM2_COORDINATE_LEN);
  }
  
  memcpy(str + 2, x, strlen(x));
  memcpy(str + 2 + strlen(x), y, strlen(y));
  memset(str + 2 + strlen(x) + strlen(y), 0, 1);
}

void tcsm_public_key_set_str(const char *str, tc_ec_t pubk)
{
  char x[SM2_COORDINATE_LEN + 1] = {0};
  char y[SM2_COORDINATE_LEN + 1] = {0};
  
  memcpy(x, str + 2, SM2_COORDINATE_LEN);
  memcpy(y, str + 2 + strlen(x), SM2_COORDINATE_LEN);
  
  tcsm_tc_ec_set_str(pubk, x, 16, y, 16);
}

long tcsm_get_file_size(FILE * file_handle)
{
    long current_read_position = (long)ftell( file_handle );
    long file_size;
    fseek( file_handle,0,SEEK_END );
    file_size = ftell( file_handle );
    fseek( file_handle,current_read_position,SEEK_SET );
    return file_size;
}

#define X509_PEM_MAXLEN 8192
void tc_der_2_pem(const unsigned char* der,size_t derlen,char* pem,size_t * pemlen,const char* begin_str,const char* end_str)
{
   char* b64 = tcsm_b64_encode(der, derlen);
   char* ptmp = b64;
   int b64len = strlen(b64);
   int line = b64len/64;
   int iPEM_LEN = b64len + line + strlen(begin_str) + strlen(end_str) + 8;
   char *csr_pem = tcsm_tc_secure_malloc(iPEM_LEN);
   
   const char* begin  = begin_str;

   memset(csr_pem, 0x00, iPEM_LEN);
   strncpy(csr_pem, begin, iPEM_LEN);
   strncat(csr_pem, "\n", iPEM_LEN);
   
   for (int i = 0; i < line; i++) {
     memcpy(csr_pem + strlen(csr_pem), b64, 64);
     strncat(csr_pem, "\n", iPEM_LEN);
     b64 = b64 + 64;
   }
   //left b64 char line
    line = b64len%64;
    if(line != 0) {
        memcpy(csr_pem + strlen(csr_pem), b64, line);
        strncat(csr_pem, "\n", iPEM_LEN);
    }
   tcsm_tc_free(ptmp);
   const char* end  = end_str;
   
   strncat(csr_pem, end, iPEM_LEN);
  
   size_t length = strlen(csr_pem);
  
   strncpy(pem, csr_pem,length);
   pem[length] = 0;
   *pemlen = length;
   tcsm_tc_secure_free(csr_pem);
}

void tcsm_csr_convert_der_2_pem(const unsigned char* der,size_t derlen,char* pem,size_t * pemlen)
{
  tc_der_2_pem(der, derlen, pem, pemlen, "-----BEGIN CERTIFICATE REQUEST-----", "-----END CERTIFICATE REQUEST-----");
}

void tcsm_crt_convert_der_2_pem(const unsigned char* der,size_t derlen,char* pem,size_t * pemlen)
{
  tc_der_2_pem(der, derlen, pem, pemlen, "-----BEGIN CERTIFICATE-----", "-----END CERTIFICATE-----");
}

int tc_pem_2_der(const char* pem,unsigned char* der,size_t * derlen,const char* begin_str,const char* end_str)
{
  char dest[X509_PEM_MAXLEN] = {0};
  char *begin, *end;
  
  begin = strstr(pem, begin_str);
  end = strstr(pem, end_str);
  
  if (begin == NULL || end == NULL || begin > end) {

    LOGV("CSR PEM Format error,not found key info.\n");
    *derlen = 0;
    return ERR_ASN1_NORMAL;

  }else{
    begin += strlen(begin_str);
    memcpy(dest, begin, end - begin);
  }
  
  size_t declen = 0;
  
  char* read_p = dest;
  char* write_p = dest;
  char temp = 0;
  do{
    temp = *(read_p++);
    if(temp != '\n' && temp != '\r' && temp != ' ')
        *(write_p++) = temp;
  }while(temp);
  
  unsigned char* dec = tcsm_b64_decode_ex(dest, strlen(dest), &declen);
  
  if (dec == NULL) {
    *derlen = 0;
    return ERR_ASN1_NORMAL;
  }
  
  memcpy(der, dec, declen);
  *derlen = declen;
  tcsm_tc_free(dec);
  return ERR_TENCENTSM_OK;
}

void tcsm_csr_convert_pem_2_der(const char* pem,unsigned char* der,size_t * derlen)
{
  tc_pem_2_der(pem, der, derlen, "-----BEGIN CERTIFICATE REQUEST-----", "-----END CERTIFICATE REQUEST-----");
}

void tcsm_crt_convert_pem_2_der(const char* pem,unsigned char* der,size_t * derlen)
{
  tc_pem_2_der(pem, der, derlen, "-----BEGIN CERTIFICATE-----", "-----END CERTIFICATE-----");
}


//----------------------------time----------------------------------
int getTMFromTime(time_t* time, struct tm* pTM) {
    if(pTM == NULL || time == NULL) {
        return ERR_TIMETRANSFER;
    }

// #ifdef _WIN32
//     if (gmtime_s(result, timer))
//         goto ERR;
//     ts = result;
// #else
//     if (gmtime_r(time, pTM) == NULL)
//         goto ERR;
// #endif
  
    struct tm* ptmp = gmtime(time);
    if(ptmp == NULL) {
        return ERR_TIMETRANSFER;
    }
    memcpy(pTM, ptmp, sizeof(struct tm));
  
    return ERR_TENCENTSM_OK;
}

unsigned long get_tick_count()
{
    struct timeval  timev;
    gettimeofday(&timev, NULL);
    return (timev.tv_sec * 1000 + timev.tv_usec / 1000);
}

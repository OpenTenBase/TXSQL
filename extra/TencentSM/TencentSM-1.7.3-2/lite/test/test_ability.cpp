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
#include "test_global.hpp"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <iostream>
#include <pthread.h>

#include "sm.h"

#include "test_sm4_perf.h"
#include "test_sm2_perf.h"
#include "test_sm3_perf.h"


using namespace::std;

void printf_hex(const unsigned char *buffer, size_t len,const char* title)
{
    char outbuf[10240] = {0};
    size_t outbuflen = 10240;
  
    const static char hexdig[] = "0123456789ABCDEF";
    char *q = NULL;
    const unsigned char *p = NULL;
    int i = 0;
    
    if (outbuflen < len * 2 + 1)
    {
        return;
    }
    
    q = outbuf;
    for (i = 0, p = buffer; i < len; i++, p++) {
        *q++ = hexdig[(*p >> 4) & 0xf];
        *q++ = hexdig[*p & 0xf];
    }
    *q = 0;
  
  printf("%s:%s\n",title,outbuf);
}

void random_content(size_t len, unsigned char *out)
{
  
  for (size_t i = 0; i < len; ++i)
  {
    srand((unsigned int)time(NULL) + (unsigned int)i);
    out[i] = rand()%256;
  }
}

unsigned long get_tick_count()
{
  struct timespec ts;
  
  clock_gettime(CLOCK_MONOTONIC, &ts);
  
  return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

const static char* METHODS_NAME[] 
    = {"sm2_generate_keypair_perf","sm2_encrypt_decrypt_perf","sm2_sign_verify_perf","test_sm2_multithread",
      "sm4_generate_key_perf","sm4_cbc_encrypt_decrypt_perf","sm4_cbc_nopadding_encrypt_decrypt_perf",
      "sm4_ecb_encrypt_decrypt_perf","sm4_ecb_nopadding_encrypt_decrypt_perf",
      "sm4_ctr_nopadding_encrypt_decrypt_perf",
      "sm4_gcm_encrypt_decrypt_perf","sm4_gcm_nopadding_encrypt_decrypt_perf",
      "sm4_gcm_nist_encrypt_decrypt_perf","sm4_gcm_nopadding_nist_encrypt_decrypt_perf","test_sm4_multithread",
      "test_sm3_multithread", "sm3_md_perf","sm3_hmac_perf",
      "sm4_ecb_steps_encrypt_decrypt_perf","sm4_ecb_nopadding_steps_encrypt_decrypt_perf","sm4_cbc_steps_encrypt_decrypt_perf","sm4_cbc_nopadding_steps_encrypt_decrypt_perf",
      "sm4_ctr_nopadding_steps_encrypt_decrypt_perf","sm4_gcm_nopadding_steps_encrypt_decrypt_perf",
    };
const static TEST_ABILITY_T TEST_METHODS[] 
    = {test_sm2_generate_keypair,test_sm2_encrypt_decrypt, test_sm2_sign_verify, test_sm2_multithread,
      test_sm4_generate_key,test_sm4_cbc_encrypt_decrypt,test_sm4_cbc_nopadding_encrypt_decrypt,
      test_sm4_ecb_encrypt_decrypt,test_sm4_ecb_nopadding_encrypt_decrypt,
      test_sm4_ctr_nopadding_encrypt_decrypt,
      test_sm4_gcm_encrypt_decrypt,test_sm4_gcm_nopadding_encrypt_decrypt,
      test_sm4_gcm_nist_sp800_38d_encrypt_decrypt,test_sm4_gcm_nopadding_nist_sp800_38d_encrypt_decrypt,test_sm4_multithread,
      test_sm3_multithread,test_sm3_md,test_sm3_hmac,
      test_sm4_ecb_steps_encrypt_decrypt,test_sm4_ecb_nopadding_steps_encrypt_decrypt,test_sm4_cbc_steps_encrypt_decrypt,test_sm4_cbc_nopadding_steps_encrypt_decrypt,
      test_sm4_ctr_nopadding_steps_encrypt_decrypt,test_sm4_gcm_nopadding_nist_sp800_38d_steps_encrypt_decrypt,
    };
const static int TEST_TIMES[] = {50000,30000,30000,30000,
  30000,500000,500000,
  500000,500000,
  500000,
  500000,500000,
  500000,500000,500000,
  500000,500000,500000,
  500000,500000,500000,500000,
  500000,500000,
};

const static int METHODS_LEN = sizeof(METHODS_NAME)/sizeof(char*);

void test_ability(const char* name) {
  TEST_ABILITY_T funp = NULL;
  int index = 0;
  if(name == NULL) {
      cout << "[Performance]: method " << name << " is null!\n";
      return;
  } else if(strcmp(name, "all") == 0) {
    for(int i = 0; i < METHODS_LEN; i++) {
        funp = TEST_METHODS[i];
        if(funp(TEST_TIMES[i],256) == RET_OK) {

            cout << "Performance:" << METHODS_NAME[i] << " is OK,Call " << TEST_TIMES[i] << " times \n";
        } else {
            cout << "Performance:" << METHODS_NAME[i] << " is error!\n";
        }
    }
    return;
  }
  for(int i = 0; i < METHODS_LEN; i++) {
      if(strcmp(METHODS_NAME[i], name) == 0) {
        funp = TEST_METHODS[i];
        index = i;
      }
  }

  if(funp != NULL) {
      if(funp(TEST_TIMES[index],256) == RET_OK) {

          cout << "Performance:" << name << " is OK,Call " << TEST_TIMES[index] << " times \n";
      } else {
          cout << "Performance:" << name << " is error!\n";
      }
  } else {
      cout << "[Performance]: method test_" << name << " not found!\n";
  }
}

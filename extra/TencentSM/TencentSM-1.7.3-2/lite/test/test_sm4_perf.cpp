/*
 
Copyright 2020, Tencent Technology (Shenzhen) Co Ltd
This file is part of the Tencent SM (Lite Version) Library.
 
*/

#include "test_global.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "../include/sm.h"
#include "../include/sm4_advance.h"

extern void random_content(size_t len, unsigned char *out);

int test_sm4_generate_key(int times,size_t data_length)
{
  int ret = RET_OK;
  
  unsigned long begin = 0,end = 0;
  double total_time = 0;
  double perf = 0;
  
  unsigned char sm4key[16] = {0};
  
  begin = get_tick_count();
  for(int i = 0; i < times; i++) {
      generateSM4Key(sm4key);
  }
  end = get_tick_count();
  total_time = end - begin;
  
  perf = (float)times/(total_time/1000.0);
  printf("SM4 generate key perf:[tps :%lu]\n",(long)perf);
  
  return ret;
}

int test_sm4_ecb_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    SM4_ECB_Encrypt(plain, plainlen, cipher, &cipherlen, SM4_KEY);
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    SM4_ECB_Decrypt(cipher, cipherlen, outplain, &outplainlen, SM4_KEY);
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-ECB en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-ECB de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_cbc_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    SM4_CBC_Encrypt(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    SM4_CBC_Decrypt(cipher, cipherlen, outplain, &outplainlen, SM4_KEY,SM4_IV);
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-CBC en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-CBC de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_gcm_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    ret = SM4_GCM_Encrypt(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    ret = SM4_GCM_Decrypt(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-GCM en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-GCM de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_gcm_nist_sp800_38d_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    ret = SM4_GCM_Encrypt_NIST_SP800_38D(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    ret = SM4_GCM_Decrypt_NIST_SP800_38D(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-GCM(NIST) en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-GCM(NIST) de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_ecb_nopadding_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    SM4_ECB_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY);
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    SM4_ECB_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, SM4_KEY);
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-ECB nopadding en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-ECB nopadding de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_cbc_nopadding_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    SM4_CBC_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    SM4_CBC_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, SM4_KEY,SM4_IV);
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-CBC nopadding en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-CBC nopadding de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_ctr_nopadding_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    SM4_CTR_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    SM4_CTR_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, SM4_KEY,SM4_IV);
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-CTR nopadding en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-CTR nopadding de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_gcm_nopadding_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    ret = SM4_GCM_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    ret = SM4_GCM_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-GCM nopadding en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-GCM nopadding de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_gcm_nopadding_nist_sp800_38d_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  unsigned long encrypt_total_time = 0;
  unsigned long decrypt_total_time = 0;

  for(int i = 0; i < times; i++) {
    
    unsigned long encrypt_begin = get_tick_count();
    
    ret = SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, 12,SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
    encrypt_total_time += encrypt_end_or_decrypt_begin - encrypt_begin;
    
    ret = SM4_GCM_Decrypt_NoPadding_NIST_SP800_38D(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
    if (ret != 0) {
      break;
    }
    
    unsigned long decrypt_end = get_tick_count();
    decrypt_total_time += decrypt_end - encrypt_end_or_decrypt_begin;
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      break;
    }
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-GCM(NIST) nopadding en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-GCM(NIST) nopadding de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

struct sm4_perf_test_args {
  int times;
  size_t data_length;
};

void* thread_test_sm4_cbc_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_CBC_Encrypt(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_cbc_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_CBC_Encrypt(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_CBC_Decrypt(cipher, cipherlen, outplain, &outplainlen, SM4_KEY,SM4_IV);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 CBC Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 CBC Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_cbc_nopadding_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_CBC_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_cbc_nopadding_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_CBC_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_CBC_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, SM4_KEY,SM4_IV);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 CBC Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 CBC Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_ctr_nopadding_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_CTR_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_ctr_nopadding_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_CTR_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY,SM4_IV);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_CTR_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, SM4_KEY,SM4_IV);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 CTR Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 CTR Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_ecb_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_ECB_Encrypt(plain, plainlen, cipher, &cipherlen, SM4_KEY);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_ecb_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_ECB_Encrypt(plain, plainlen, cipher, &cipherlen, SM4_KEY);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_ECB_Decrypt(cipher, cipherlen, outplain, &outplainlen, SM4_KEY);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 ECB Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 ECB Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_ecb_nopadding_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_ECB_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_ecb_nopadding_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_ECB_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, SM4_KEY);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_ECB_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, SM4_KEY);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 ECB Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 ECB Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_GCM_Encrypt(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_GCM_Encrypt(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_GCM_Decrypt(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_nopadding_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_GCM_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_nopadding_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_GCM_Encrypt_NoPadding(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_GCM_Decrypt_NoPadding(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_nist_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_GCM_Encrypt_NIST_SP800_38D(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_nist_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_GCM_Encrypt_NIST_SP800_38D(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_GCM_Decrypt_NIST_SP800_38D(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_nopadding_nist_encrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, 12,SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

void* thread_test_sm4_gcm_nopadding_nist_decrypt(void *ptr) {
  
  sm4_perf_test_args args = *((sm4_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t plainlen = args.data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char tag[16] = {0};
  size_t taglen = 16;

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  
  
  ret = SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(plain, plainlen, cipher, &cipherlen, tag, &taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
  if(ret != 0) {
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    ret = SM4_GCM_Decrypt_NoPadding_NIST_SP800_38D(cipher, cipherlen, outplain, &outplainlen, tag, taglen, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV,12, SM4_GCM_TEST_AAD, 16);
    if(ret != 0) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
    
    if(outplainlen != plainlen || memcmp(outplain, plain, plainlen)) {
      ret = RET_ERR;
      printf("SM4 GCM Decrypt fatal error!!!\n");
      break;
    }
  }
  
END:
  return NULL;
}

int test_sm4_multithread(int times,size_t data_length)
{
  int core_num = (int)sysconf(_SC_NPROCESSORS_ONLN);
  pthread_t tid[MAX_TID];
  
  sm4_perf_test_args args;
  args.times = times;
  args.data_length = data_length;
  
  /* 多线程满核性能测试：ECB加密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_ecb_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  unsigned long begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  unsigned long end = get_tick_count();
  
  double encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-ECB en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：ECB解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_ecb_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-ECB de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：ECB NOPADDING加密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_ecb_nopadding_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-ECB nopadding en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：ECB NOPADDING 解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_ecb_nopadding_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-ECB nopadding de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
/* 多线程满核性能测试：CBC加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_cbc_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-CBC en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：CBC解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_cbc_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-CBC de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：CBC NOPADDING加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_cbc_nopadding_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-CBC nopadding en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：CBC NOPADDING解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_cbc_nopadding_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-CBC nopadding de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：CTR NOPADDING加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_ctr_nopadding_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-CTR nopadding en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：CTR NOPADDING解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_ctr_nopadding_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-CTR nopadding de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  

  /* 多线程满核性能测试：GCM NOPADDING加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_nopadding_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM nopadding en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM NOPADDING解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_nopadding_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM nopadding de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM NIST加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_nist_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM(NIST) en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM NIST解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_nist_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM(NIST) de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM NIST NOPADDING加密 */
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_nopadding_nist_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM(NIST) nopadding en multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  /* 多线程满核性能测试：GCM NIST NOPADDING解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm4_gcm_nopadding_nist_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm4_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM4-GCM(NIST) nopadding de multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
  return 0;
}

//SM4分步计算
int test_sm4_ecb_steps_encrypt_decrypt(int times,size_t data_length){
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);

  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);

  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;

  tcsm_sm4_ecb_t ctx_en;
  tcsm_sm4_ecb_t ctx_de;
  unsigned char *p_outb = NULL;
  size_t tmp_cipherlen = 0;
    
  unsigned long encrypt_begin = get_tick_count();
  SM4_ECB_Encrypt_Init(&ctx_en, SM4_KEY, 0);
  p_outb = cipher;
  for(int i = 0; i < times; i++)
    SM4_ECB_Encrypt_Update(&ctx_en, plain, plainlen, p_outb, &tmp_cipherlen);
  p_outb += tmp_cipherlen;
  SM4_ECB_Encrypt_Final(&ctx_en, p_outb, &cipherlen);
  cipherlen += tmp_cipherlen;
  unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
  unsigned long encrypt_total_time = encrypt_end_or_decrypt_begin - encrypt_begin;

  for(int i = 0; i < times; i++) {
    SM4_ECB_Decrypt_Init(&ctx_de, SM4_KEY, 0);
    p_outb = outplain;
    SM4_ECB_Decrypt_Update(&ctx_de, cipher, cipherlen, p_outb, &tmp_cipherlen);
    p_outb += tmp_cipherlen;
    SM4_ECB_Decrypt_Final(&ctx_de, p_outb, &outplainlen);
    outplainlen += tmp_cipherlen;
  }
  unsigned long decrypt_end = get_tick_count();
  unsigned long decrypt_total_time = decrypt_end - encrypt_end_or_decrypt_begin;

  if(outplainlen != plainlen || memcmp(outplain, plain, plainlen))
    ret = RET_ERR;

  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-ECB STEPS en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-ECB STEPS de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_cbc_steps_encrypt_decrypt(int times,size_t data_length){
  int ret = RET_OK;
  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);
  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  tcsm_sm4_cbc_t ctx_en;
  tcsm_sm4_cbc_t ctx_de;
  unsigned char *p_outb = NULL;
  size_t tmp_cipherlen = 0;
  unsigned long encrypt_begin = get_tick_count();
  for(int i = 0; i < times; i++) {
    SM4_CBC_Encrypt_Init(&ctx_en, SM4_KEY, SM4_IV, 0);
    p_outb = cipher;
    SM4_CBC_Encrypt_Update(&ctx_en, plain, plainlen, p_outb, &tmp_cipherlen);
    p_outb += tmp_cipherlen;
    SM4_CBC_Encrypt_Final(&ctx_en, p_outb, &cipherlen);
    cipherlen += tmp_cipherlen;
  }
  unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
  unsigned long encrypt_total_time = encrypt_end_or_decrypt_begin - encrypt_begin;

  for(int i = 0; i < times; i++) {
    SM4_CBC_Decrypt_Init(&ctx_de, SM4_KEY, SM4_IV, 0);
    p_outb = outplain;
    SM4_CBC_Decrypt_Update(&ctx_de, cipher, cipherlen, p_outb, &tmp_cipherlen);
    p_outb += tmp_cipherlen;
    SM4_CBC_Decrypt_Final(&ctx_de, p_outb, &outplainlen);
    outplainlen += tmp_cipherlen;
  }
  unsigned long decrypt_end = get_tick_count();
  unsigned long decrypt_total_time = decrypt_end - encrypt_end_or_decrypt_begin;
  if(outplainlen != plainlen || memcmp(outplain, plain, plainlen))
      ret = RET_ERR;
    
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);
    
  printf("SM4-CBC STEPS en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
    
  printf("SM4-CBC STEPS de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_ecb_nopadding_steps_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);
  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  tcsm_sm4_ecb_t ctx_en;
  tcsm_sm4_ecb_t ctx_de;
  unsigned char *p_outb = NULL;
  size_t tmp_cipherlen = 0;
      
  unsigned long encrypt_begin = get_tick_count();
  SM4_ECB_Encrypt_Init(&ctx_en, SM4_KEY, 1);
  p_outb = cipher;
  for(int i = 0; i < times; i++)
    SM4_ECB_Encrypt_Update(&ctx_en, plain, plainlen, p_outb, &tmp_cipherlen);
  p_outb += tmp_cipherlen;
  SM4_ECB_Encrypt_Final(&ctx_en, p_outb, &cipherlen);
  cipherlen += tmp_cipherlen;
  unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
  unsigned long encrypt_total_time = encrypt_end_or_decrypt_begin - encrypt_begin;

  SM4_ECB_Decrypt_Init(&ctx_de, SM4_KEY, 1);
  p_outb = outplain;
  for(int i = 0; i < times; i++)
    SM4_ECB_Decrypt_Update(&ctx_de, cipher, cipherlen, p_outb, &tmp_cipherlen);
  p_outb += tmp_cipherlen;
  SM4_ECB_Decrypt_Final(&ctx_de, p_outb, &outplainlen);
  outplainlen += tmp_cipherlen;
  unsigned long decrypt_end = get_tick_count();
  unsigned long decrypt_total_time = decrypt_end - encrypt_end_or_decrypt_begin;
  if(outplainlen != plainlen || memcmp(outplain, plain, plainlen))
    ret = RET_ERR;

  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);

  printf("SM4-ECB nopadding STEPS en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
    
  printf("SM4-ECB nopadding STEPS de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

int test_sm4_cbc_nopadding_steps_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);
  size_t cipherlen = plainlen + 16;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  tcsm_sm4_cbc_t ctx_en;
  tcsm_sm4_cbc_t ctx_de;
  unsigned char *p_outb = NULL;
  size_t tmp_cipherlen = 0;
  unsigned long encrypt_begin = get_tick_count();
  for(int i = 0; i < times; i++) {
    SM4_CBC_Encrypt_Init(&ctx_en, SM4_KEY, SM4_IV, 1);
    p_outb = cipher;
    SM4_CBC_Encrypt_Update(&ctx_en, plain, plainlen, p_outb, &tmp_cipherlen);
    p_outb += tmp_cipherlen;
    SM4_CBC_Encrypt_Final(&ctx_en, p_outb, &cipherlen);
    cipherlen += tmp_cipherlen;
  }
  unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
  unsigned long encrypt_total_time = encrypt_end_or_decrypt_begin - encrypt_begin;
  
  for(int i = 0; i < times; i++) {
    SM4_CBC_Decrypt_Init(&ctx_de, SM4_KEY, SM4_IV, 1);
    p_outb = outplain;
    SM4_CBC_Decrypt_Update(&ctx_de, cipher, cipherlen, p_outb, &tmp_cipherlen);
    p_outb += tmp_cipherlen;
    SM4_CBC_Decrypt_Final(&ctx_de, p_outb, &outplainlen);
    outplainlen += tmp_cipherlen;
  }
  unsigned long decrypt_end = get_tick_count();
  unsigned long decrypt_total_time = decrypt_end - encrypt_end_or_decrypt_begin;
  if(outplainlen != plainlen || memcmp(outplain, plain, plainlen))
      ret = RET_ERR;
  
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);
  
  printf("SM4-CBC nopadding STEPS en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-CBC nopadding STEPS de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
 
  return ret;
}

int test_sm4_ctr_nopadding_steps_encrypt_decrypt(int times,size_t data_length){
  
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);
  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  tcsm_sm4_ctr_t ctx_en;
  tcsm_sm4_ctr_t ctx_de;

  unsigned long encrypt_begin = get_tick_count();
  for(int i = 0; i < times; i++) {
    SM4_CTR_Encrypt_Init(&ctx_en, SM4_KEY, SM4_IV);
    SM4_CTR_Encrypt_Update(&ctx_en, plain, plainlen, cipher, &cipherlen);
    SM4_CTR_Encrypt_Final(&ctx_en);
  }
  unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
  unsigned long encrypt_total_time = encrypt_end_or_decrypt_begin - encrypt_begin;
  
  for(int i = 0; i < times; i++) {
    SM4_CTR_Decrypt_Init(&ctx_de, SM4_KEY, SM4_IV);
    SM4_CTR_Decrypt_Update(&ctx_de, cipher, cipherlen, outplain, &outplainlen);
    SM4_CTR_Decrypt_Final(&ctx_de);
  }
  unsigned long decrypt_end = get_tick_count();
  unsigned long decrypt_total_time = decrypt_end - encrypt_end_or_decrypt_begin;
  if(outplainlen != plainlen || memcmp(outplain, plain, plainlen))
    ret = RET_ERR;
  
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);
  
  printf("SM4-CTR nopadding STEPS en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-CTR nopadding STEPS de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);
 
  return ret;
}

int test_sm4_gcm_nopadding_nist_sp800_38d_steps_encrypt_decrypt(int times,size_t data_length){
   
  int ret = RET_OK;

  size_t plainlen = data_length;
  unsigned char plain[plainlen];
  random_content(plainlen, plain);
  size_t cipherlen = plainlen;
  unsigned char cipher[cipherlen];
  memset(cipher, 0x00, cipherlen);
  unsigned char outplain[cipherlen];
  memset(outplain, 0x00, cipherlen);
  size_t outplainlen = cipherlen;
  unsigned char tag[16] = {0};
  size_t taglen = 16;
  unsigned char buf[16] = {0};
  unsigned char ivbuf[16] = {0};
  tcsm_sm4_gcm_t ctx_en;
  tcsm_sm4_gcm_t ctx_de;

  unsigned long encrypt_begin = get_tick_count();
  for(int i = 0; i < times; i++) {
    SM4_GCM_Encrypt_Init(&ctx_en, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, 16, SM4_GCM_TEST_AAD, 16);
    SM4_GCM_Encrypt_Update(&ctx_en, plain, plainlen, cipher, &cipherlen);
    SM4_GCM_Encrypt_Final(&ctx_en, tag, taglen);
  }

  unsigned long encrypt_end_or_decrypt_begin = get_tick_count();
  unsigned long encrypt_total_time = encrypt_end_or_decrypt_begin - encrypt_begin;

  for(int i = 0; i < times; i++) {
    SM4_GCM_Decrypt_Init(&ctx_de, SM4_GCM_TEST_KEY, SM4_GCM_TEST_IV, 16, SM4_GCM_TEST_AAD, 16);
    SM4_GCM_Decrypt_Update(&ctx_de, cipher, cipherlen, outplain, &outplainlen);
    ret = SM4_GCM_Decrypt_Final(&ctx_de, tag, taglen);
  }

  unsigned long decrypt_end = get_tick_count();
  unsigned long decrypt_total_time = decrypt_end - encrypt_end_or_decrypt_begin;
  if((0 != ret) || (outplainlen != plainlen) || memcmp(outplain, plain, plainlen)) {
    ret = RET_ERR;
  }
  double encrypt_count_per_second = (float)times/(encrypt_total_time/1000.0);
  double decrypt_count_per_second = (float)times/(decrypt_total_time/1000.0);
  
  printf("SM4-GCM nopadding STEPS en perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024/1024);
  
  printf("SM4-GCM nopadding STEPS de perf:[block size:%lu][tps :%lu][%lf MBps]\n",
  data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024/1024);

  return ret;
}

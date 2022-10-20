/*
 
Copyright 2020, Tencent Technology (Shenzhen) Co Ltd
This file is part of the Tencent SM (Lite Version) Library.
 
*/

#include "test_sm3_perf.h"
#include "test_global.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "../include/sm.h"

extern void random_content(size_t len, unsigned char *out);

int test_sm3_md(int times,size_t data_length)
{
  /* 正确性验证 */
  int ret = RET_OK;
  unsigned char md1[SM3_DIGEST_LENGTH] = {0};
  unsigned char md2[SM3_DIGEST_LENGTH] = {0};
  char md_str[SM3_DIGEST_LENGTH*2+1] = {0};
  
  sm3_ctx_t ctx;
  SM3Init(&ctx);
  SM3Update(&ctx, (const unsigned char*)SM3_MSG, strlen(SM3_MSG));
  SM3Final(&ctx, md1);
  
  SM3((const unsigned char*)SM3_MSG, strlen(SM3_MSG), md2);
  
  if (memcmp(md1, md2, SM3_DIGEST_LENGTH)) {
    ret = RET_ERR;
    return ret;
  }
  
  tc_bin2hex((const unsigned char*)md1, SM3_DIGEST_LENGTH, md_str, SM3_DIGEST_LENGTH*2+1);
  if(memcmp(SM3_MD_DATA, md_str, SM3_DIGEST_LENGTH*2)) {
    ret = RET_ERR;
    return ret;
  }
  
  /* 性能验证 */

  unsigned char* msg = (unsigned char*)malloc(data_length);
  random_content(data_length, msg);

  unsigned long begin = get_tick_count();
  
  unsigned char digest[SM3_DIGEST_LENGTH] = {0};

  for (int i = 0; i < times; i++) {
    /* method 1 */
    SM3(msg, data_length, digest);
    /* method 2 */
    SM3Init(&ctx);
    SM3Update(&ctx, (const unsigned char*)msg, data_length);
    SM3Final(&ctx, digest);
  }
  
  unsigned long end = get_tick_count();
  unsigned long cost = end - begin;
  
  double count_per_second = (float)2*times/(cost/1000.0);
  
  printf("SM3 hash perf:[block size:%lu][tps :%lu][%lf Mbps]\n",
  data_length,(long)count_per_second,data_length*count_per_second/1024/1024);
  
  free(msg);
END:
  return ret;
}

int test_sm3_hmac(int times,size_t data_length)
{
  /* 正确性验证 */
  
  int ret = RET_OK;
  unsigned char mac1[SM3_HMAC_SIZE] = {0};
  unsigned char mac2[SM3_HMAC_SIZE] = {0};
  char mac_str[SM3_HMAC_SIZE*2+1] = {0};

  SM3_HMAC((const unsigned char*)SM3_MSG, strlen(SM3_MSG), (const unsigned char*)SM3_HMAC_KEY, strlen(SM3_HMAC_KEY), mac1);
  
  TstHmacSm3Ctx* sm3hmacctx = SM3_HMAC_Init((const unsigned char*)SM3_HMAC_KEY,  strlen(SM3_HMAC_KEY));
  if(sm3hmacctx == NULL) {
      ret = RET_ERR;
      return ret;
  }
  
  if(SM3_HMAC_Update(sm3hmacctx, (const unsigned char*)SM3_MSG, strlen(SM3_MSG)) != 0) {
      ret = RET_ERR;
       return ret;
  }
  if(SM3_HMAC_Final(sm3hmacctx, mac2) != 0) {
      ret = RET_ERR;
       return ret;
  }
  
  if (memcmp(mac1, mac2, SM3_HMAC_SIZE)) {
    ret = RET_ERR;
    return ret;
  }
  
  tc_bin2hex((const unsigned char*)mac1, SM3_DIGEST_LENGTH, mac_str, SM3_DIGEST_LENGTH*2+1);
  if(memcmp(SM3_HMAC_RET, mac_str, SM3_HMAC_SIZE*2)) {
    ret = RET_ERR;
    return ret;
  }
  
  /* 性能验证 */

  unsigned char* msg = (unsigned char*)malloc(data_length);
  random_content(data_length, msg);

  unsigned long begin = get_tick_count();
  
  unsigned char mac[SM3_HMAC_SIZE] = {0};

  for (int i = 0; i < times; i++) {
    /* method 1 */
    SM3_HMAC((const unsigned char*)msg, data_length, (const unsigned char*)SM3_HMAC_KEY, strlen(SM3_HMAC_KEY), mac);
    
    /* method 2 */
    TstHmacSm3Ctx* sm3hmacctx = SM3_HMAC_Init((const unsigned char*)SM3_HMAC_KEY,  strlen(SM3_HMAC_KEY));
    SM3_HMAC_Update(sm3hmacctx, (const unsigned char*)msg, data_length);
    SM3_HMAC_Final(sm3hmacctx, mac);
  }
  
  unsigned long end = get_tick_count();
  unsigned long cost = end - begin;
  
  double count_per_second = (float)2*times/(cost/1000.0);
  
  printf("SM3 hmac perf:[block size:%lu][tps :%lu][%lf Mbps]\n",
  data_length,(long)count_per_second,data_length*count_per_second/1024/1024);
  
  free(msg);
  
END:
  return ret;
}

struct sm3_perf_test_args {
  int times;
  size_t data_length;
};

void* thread_test_sm3_md(void *ptr) {
  
  sm3_perf_test_args args = *((sm3_perf_test_args*)ptr);

  int ret = RET_OK;

  size_t msglen = args.data_length;
  unsigned char msg[msglen];
  random_content(msglen, msg);


  unsigned char digest[SM3_DIGEST_LENGTH] = {0};
  
  for (int i = 0; i < args.times; i++)
  {
    /* hash */
    ret = SM3(msg, msglen, digest);
    if(ret != 0) {
      ret = RET_ERR;
      break;
    }
  }
  
END:
  return NULL;
}

int test_sm3_multithread(int times,size_t data_length)
{
  int core_num = (int)sysconf(_SC_NPROCESSORS_ONLN);
  pthread_t tid[MAX_TID];
  
  sm3_perf_test_args args;
  args.times = times;
  args.data_length = data_length;
  
  /* 多线程满核性能测试：SM3 MD */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm3_md, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm3_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  unsigned long begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm3_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  unsigned long end = get_tick_count();
  
  double hash_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM3 hash multi-thread perf:[block size:%lu][tps :%lu][%.02lf MBps]\n",
         data_length,(long)hash_count_per_second,data_length* hash_count_per_second/1024/1024);

  return 0;
}

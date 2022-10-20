/*
 
Copyright 2020, Tencent Technology (Shenzhen) Co Ltd
This file is part of the Tencent SM (Lite Version) Library.
 
*/

#include "test_sm2_perf.h"

#include "test_global.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include "../include/sm.h"

extern void random_content(size_t len, unsigned char *out);

int test_sm2_generate_keypair_v1(int times,size_t data_length)
{
  int ret = RET_OK;
  unsigned long begin = 0,end = 0;
  double total_time = 0;
  double perf = 0;
  
  char private_key[65] = {0};
  char public_key[131] = {0};
  
  sm2_ctx_t ctx;
  SM2InitCtx(&ctx);

  for(int i = 0; i < times; i++) {
    
    begin = get_tick_count();
    
    if(generateKeyPair(&ctx, private_key, public_key) != 0) {
        ret = RET_ERR;
        goto END;
    }
    
    end = get_tick_count();
    total_time += (end - begin);
    
    /* 密钥正确性和一致性校验 */
    if (strlen(private_key) != 64 || strlen(public_key) != 130) {
      printf("test sm2 generate key pair error,key length is not correct.\n");
      goto END;
    }
    
    unsigned char *plain = new unsigned char[data_length];
    random_content(data_length, plain);
    
    unsigned char *cipher = new unsigned char[data_length + 200];
    size_t cipher_outlen = data_length + 200;
    
    ret = SM2Encrypt(&ctx, plain, data_length, public_key, 130, cipher, &cipher_outlen);
    if(ret != 0) {
      delete[] plain;
      delete[] cipher;
      ret = RET_ERR;
      goto END;;
    }
    
    unsigned char *plain_decrypt = new unsigned char[data_length];
    size_t plain_outlen = 0;
    ret = SM2Decrypt(&ctx, cipher, cipher_outlen, private_key, 64, plain_decrypt, &plain_outlen);
    if(ret != 0) {
      delete[] plain;
      delete[] cipher;
      delete[] plain_decrypt;
      ret = RET_ERR;
      goto END;;
    }
    
    if (memcmp(plain, plain_decrypt, data_length) != 0)
    {
      printf("test sm2 generate key pair error,key pair is not match.\n");
      delete[] plain;
      delete[] cipher;
      delete[] plain_decrypt;
      ret = RET_ERR;
      goto END;
    }
    
    delete[] plain;
    delete[] cipher;
    delete[] plain_decrypt;
  }
  
  perf = (float)times/(total_time/1000.0);
  printf("SM2 generate key(v1) pair perf:[tps :%lu]\n",(long)perf);
END:
  SM2FreeCtx(&ctx);
  return ret;
}

int test_sm2_generate_keypair_v2(int times,size_t data_length)
{
  int ret = RET_OK;
  unsigned long begin = 0,end = 0;
  double total_time = 0;
  double perf = 0;
  
  char private_key[65] = {0};
  char public_key[131] = {0};
  
  sm2_ctx_t ctx;
  SM2InitCtx(&ctx);

  for(int i = 0; i < times; i++) {
    
    begin = get_tick_count();
    
    if(generatePrivateKey(&ctx, private_key) != 0) {
        ret = RET_ERR;
        goto END;
    }
    
    if(generatePublicKey(&ctx, private_key, public_key) != 0) {
        ret = RET_ERR;
        goto END;
    }
   
    end = get_tick_count();
    total_time += (end - begin);
    
    /* 密钥正确性和一致性校验 */
    if (strlen(private_key) != 64 || strlen(public_key) != 130) {
      printf("test sm2 generate key pair error,key length is not correct.\n");
      goto END;
    }
    
    unsigned char *plain = new unsigned char[data_length];
    random_content(data_length, plain);
    
    unsigned char *cipher = new unsigned char[data_length + 200];
    size_t cipher_outlen = data_length + 200;
    
    ret = SM2Encrypt(&ctx, plain, data_length, public_key, 130, cipher, &cipher_outlen);
    if(ret != 0) {
      delete[] plain;
      delete[] cipher;
      ret = RET_ERR;
      goto END;;
    }
    
    unsigned char *plain_decrypt = new unsigned char[data_length];
    size_t plain_outlen = 0;
    ret = SM2Decrypt(&ctx, cipher, cipher_outlen, private_key, 64, plain_decrypt, &plain_outlen);
    if(ret != 0) {
      delete[] plain;
      delete[] cipher;
      delete[] plain_decrypt;
      ret = RET_ERR;
      goto END;;
    }
    
    if (memcmp(plain, plain_decrypt, data_length) != 0)
    {
      printf("test sm2 generate key pair error,key pair is not match.\n");
      delete[] plain;
      delete[] cipher;
      delete[] plain_decrypt;
      ret = RET_ERR;
      goto END;
    }
    
    delete[] plain;
    delete[] cipher;
    delete[] plain_decrypt;
  }
  
  perf = (float)times/(total_time/1000.0);
  printf("SM2 generate key(v2) pair perf:[tps :%lu]\n",(long)perf);
END:
  SM2FreeCtx(&ctx);
  return ret;
}

int test_sm2_generate_keypair(int times,size_t data_length)
{
  int ret = 0;
  ret = test_sm2_generate_keypair_v1(times, data_length);
  if (ret != RET_OK) {
    return ret;
  }

  ret = test_sm2_generate_keypair_v2(times, data_length);
  if (ret != RET_OK) {
    return ret;
  }
  
  return RET_OK;
}

int test_sm2_encrypt_decrypt_inner(int times,size_t data_length,int precompute)
{
  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  /* 0. 初始化 */
  sm2_ctx_t ctx;
  
  if (precompute != 0) {
    SM2InitCtxWithPubKey(&ctx, pub);
  }else{
    SM2InitCtx(&ctx);
  }
  
  unsigned long random_content_time = 0;
  unsigned long encrypt_time = 0;
  unsigned long decrypt_time = 0;
  
  for (int i = 0; i < times; i++)
  {
    /* 1. 生成测试的随机明文 */
    unsigned long timestamp_random_begin = get_tick_count();
    
    unsigned char *plain = new unsigned char[data_length];
    random_content(data_length, plain);
    
    unsigned char *cipher = new unsigned char[data_length + 200];
    size_t cipher_outlen = data_length + 200;
    
    unsigned long timestamp_random_end = get_tick_count();
    
    random_content_time += timestamp_random_end - timestamp_random_begin;
    
    /* 2. 加密 */
    ret = SM2Encrypt(&ctx, plain, data_length, pub, 130, cipher, &cipher_outlen);
    if(ret != 0) {
      delete[] plain;
      delete[] cipher;
      ret = RET_ERR;
      break;
    }
    unsigned long timestamp_encrypt_end = get_tick_count();
    encrypt_time += timestamp_encrypt_end - timestamp_random_end;
    
    /* 3. 解密 */
    unsigned char *plain_decrypt = new unsigned char[data_length];
    size_t plain_outlen = 0;
    ret = SM2Decrypt(&ctx, cipher, cipher_outlen, pri, 64, plain_decrypt, &plain_outlen);
    if(ret != 0) {
      delete[] plain;
      delete[] cipher;
      delete[] plain_decrypt;
      ret = RET_ERR;
      break;
    }
    
    unsigned long timestamp_decrypt_end = get_tick_count();
    decrypt_time += timestamp_decrypt_end - timestamp_encrypt_end;
    
    /* 3. 校验 */
    if (memcmp(plain, plain_decrypt, data_length) != 0)
    {
      printf("test sm2 encrypt decrypt failed.\n");
      ret = RET_ERR;
    }

    delete[] plain;
    delete[] cipher;
    delete[] plain_decrypt;
    if(ret != RET_OK) {
      ret = RET_ERR;
      break;
    }
  }
  
  double encrypt_count_per_second = (float)times/(encrypt_time/1000.0);
  printf("SM2 en perf:[precompute:%d][block size:%lu][tps :%lu][%.02lf Kbps]\n",
         precompute,data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024);
  
  double decrypt_count_per_second = (float)times/(decrypt_time/1000.0);
  printf("SM2 de perf:[precompute:%d][block size:%lu][tps :%lu][%.02lf Kbps]\n",
         precompute,data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024);
END:
  SM2FreeCtx(&ctx);
  return ret;;
  
}

int test_sm2_encrypt_decrypt(int times,size_t data_length)
{
  int ret = 0;
  ret = test_sm2_encrypt_decrypt_inner(times, data_length, 0);
  if (ret != 0) {
    return ret;
  }
  
  ret = test_sm2_encrypt_decrypt_inner(times, data_length, 1);
  if (ret != 0) {
    return ret;
  }
  
  return ret;
}

int test_sm2_sign_verify_inner(int times,size_t data_length,int precompute)
{
  sm2_ctx_t ctx;
  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  if (precompute != 0) {
    SM2InitCtxWithPubKey(&ctx, pub);
  }else{
    SM2InitCtx(&ctx);
  }
  
  unsigned long random_content_time = 0;
  unsigned long sign_time = 0;
  unsigned long verify_time = 0;
  
  for (int i = 0; i < times; ++i)
  {
    unsigned long timestamp_random_begin = get_tick_count();
    
    size_t random_len = data_length;
    
    unsigned char *msg = new unsigned char[random_len];
    random_content(random_len, msg);
    
    size_t id_len = 16;
    const char *id_str = "1234567812345678";
    
    unsigned char *sig = new unsigned char[80];
    memset(sig, 0, 80);
    size_t outlen = 80;
    
    unsigned long timestamp_random_end = get_tick_count();
    random_content_time += timestamp_random_end - timestamp_random_begin;
    
    ret = SM2Sign(&ctx, msg, random_len, (char *)id_str, id_len, pub, 130, pri, 64, sig, &outlen);
    if (ret != 0)
    {
      printf("test sm2 sign failed.\n");
      delete[] msg;
      delete[] sig;
      ret = RET_ERR;
      break;
    }
    
    unsigned long timestamp_sign_end = get_tick_count();
    sign_time += timestamp_sign_end - timestamp_random_end;
    
    ret = SM2Verify(&ctx, msg, random_len, id_str, id_len, sig, outlen, pub, 130);
    if (ret != 0)
    {
      printf("test sm2 verify failed.\n");
      delete[] msg;
      delete[] sig;
      ret = RET_ERR;
      break;
    }
    
    unsigned long timestamp_verify_end = get_tick_count();
    verify_time += timestamp_verify_end - timestamp_sign_end;

    delete[] msg;
    delete[] sig;
  }
  
  double sign_count_per_second = (float)times/(sign_time/1000.0);
  printf("SM2 sign perf:[precompute:%d][block size:%lu][tps :%lu]\n",
         precompute,data_length,(long)sign_count_per_second);
  
  double verify_count_per_second = (float)times/(verify_time/1000.0);
  printf("SM2 verify perf:[precompute:%d][block size:%lu][tps :%lu]\n",
         precompute,data_length,(long)verify_count_per_second);
  
  SM2FreeCtx(&ctx);
  return ret;
}

int test_sm2_sign_verify(int times,size_t data_length)
{
  int ret = 0;
  ret = test_sm2_sign_verify_inner(times, data_length, 0);
  if (ret != 0) {
    return ret;
  }
  
  ret = test_sm2_sign_verify_inner(times, data_length, 1);
  if (ret != 0) {
    return ret;
  }
  return ret;
}

struct sm2_perf_test_args {
  int times;
  size_t data_length;
};

void* thread_test_sm2_encrypt_precompute(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;

  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  
  /* 初始化 */
  sm2_ctx_t ctx;
  
  SM2InitCtxWithPubKey(&ctx, pub);
  
  unsigned char *plain = new unsigned char[data_length];
  random_content(data_length, plain);
  
  unsigned char *cipher = new unsigned char[data_length + 200];
  size_t cipher_outlen = data_length + 200;
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM2Encrypt(&ctx, plain, data_length, pub, SM2_PUBKEY_LEN, cipher, &cipher_outlen);
    if(ret != 0) {
      printf("test sm2 encrypt(precompute) failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] plain;
  delete[] cipher;
  SM2FreeCtx(&ctx);
  return NULL;
}

void* thread_test_sm2_encrypt(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;

  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  
  /* 初始化 */
  sm2_ctx_t ctx;
  
  SM2InitCtx(&ctx);
  
  unsigned char *plain = new unsigned char[data_length];
  random_content(data_length, plain);
  
  unsigned char *cipher = new unsigned char[data_length + 200];
  size_t cipher_outlen = data_length + 200;
  
  for (int i = 0; i < args.times; i++)
  {
    /*  加密 */
    ret = SM2Encrypt(&ctx, plain, data_length, pub, SM2_PUBKEY_LEN, cipher, &cipher_outlen);
    if(ret != 0) {
      printf("test sm2 encrypt failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] plain;
  delete[] cipher;
  SM2FreeCtx(&ctx);
  return NULL;
}


void* thread_test_sm2_decrypt(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;

  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  /* 初始化 */
  sm2_ctx_t ctx;
  
  SM2InitCtxWithPubKey(&ctx, pub);
  
  unsigned char *plain = new unsigned char[data_length];
  random_content(data_length, plain);
  
  unsigned char *cipher = new unsigned char[data_length + 200];
  size_t cipher_outlen = data_length + 200;
  
  unsigned char *plain_decrypt = new unsigned char[data_length];
  
  /*  加密 */
  ret = SM2Encrypt(&ctx, plain, data_length, pub, SM2_PUBKEY_LEN, cipher, &cipher_outlen);
  if(ret != 0) {
    printf("test sm2 decrypt(s1) failed.\n");
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; i++)
  {
    size_t plain_outlen = 0;
    ret = SM2Decrypt(&ctx, cipher, cipher_outlen, pri, SM2_PRIKEY_LEN, plain_decrypt, &plain_outlen);
    if(ret != 0) {
      printf("test sm2 decrypt(s2) failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] plain;
  delete[] cipher;
  delete[] plain_decrypt;
  SM2FreeCtx(&ctx);
  return NULL;
}

void* thread_test_sm2_sign(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;
  
  sm2_ctx_t ctx;
  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  SM2InitCtx(&ctx);
  
  unsigned char *msg = new unsigned char[data_length];
  random_content(data_length, msg);
  
  size_t id_len = 16;
  const char *id_str = "1234567812345678";
  
  unsigned char *sig = new unsigned char[80];
  memset(sig, 0, 80);
  
  for (int i = 0; i < args.times; ++i)
  {
    size_t outlen = 80;
    
    ret = SM2Sign(&ctx, msg, data_length, (char *)id_str, id_len, pub, 130, pri, 64, sig, &outlen);
    if (ret != 0)
    {
      printf("test sm2 sign failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] msg;
  delete[] sig;
  SM2FreeCtx(&ctx);
  
  return NULL;
}

void* thread_test_sm2_sign_precompute(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;
  
  sm2_ctx_t ctx;
  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  SM2InitCtxWithPubKey(&ctx, pub);
  
  unsigned char *msg = new unsigned char[data_length];
  random_content(data_length, msg);
  
  size_t id_len = 16;
  const char *id_str = "1234567812345678";
  
  unsigned char *sig = new unsigned char[80];
  memset(sig, 0, 80);
  
  for (int i = 0; i < args.times; ++i)
  {
    size_t outlen = 80;
    
    ret = SM2Sign(&ctx, msg, data_length, (char *)id_str, id_len, pub, 130, pri, 64, sig, &outlen);
    if (ret != 0)
    {
      printf("test sm2 sign(precompute) failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] msg;
  delete[] sig;
  SM2FreeCtx(&ctx);
  
  return NULL;
}

void* thread_test_sm2_verify(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;
  
  sm2_ctx_t ctx;
  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  SM2InitCtx(&ctx);
  
  unsigned char *msg = new unsigned char[data_length];
  random_content(data_length, msg);
  
  size_t id_len = 16;
  const char *id_str = "1234567812345678";
  
  unsigned char *sig = new unsigned char[80];
  memset(sig, 0, 80);
  
  size_t outlen = 80;
  
  ret = SM2Sign(&ctx, msg, data_length, (char *)id_str, id_len, pub, 130, pri, 64, sig, &outlen);
  if (ret != 0)
  {
    printf("test sm2 sign failed.\n");
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; ++i)
  {
    ret = SM2Verify(&ctx, msg, data_length, id_str, id_len, sig, outlen, pub, 130);
    if (ret != 0)
    {
      printf("test sm2 verify failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] msg;
  delete[] sig;
  SM2FreeCtx(&ctx);
  
  return NULL;
}

void* thread_test_sm2_verify_precompute(void *ptr) {
  
  sm2_perf_test_args args = *((sm2_perf_test_args*)ptr);
  int data_length = (int)args.data_length;
  
  sm2_ctx_t ctx;
  int ret = RET_OK;
  
  const char *pub = "0409F9DF311E5421A150DD7D161E4BC5C672179FAD1833FC076BB08FF356F35020CCEA490CE26775A52DC6EA718CC1AA600AED05FBF35E084A6632F6072DA9AD13";
  const char *pri = "3945208F7B2144B13F36E38AC6D39F95889393692860B51A42FB81EF4DF7C5B8";
  
  SM2InitCtxWithPubKey(&ctx, pub);
  
  unsigned char *msg = new unsigned char[data_length];
  random_content(data_length, msg);
  
  size_t id_len = 16;
  const char *id_str = "1234567812345678";
  
  unsigned char *sig = new unsigned char[80];
  memset(sig, 0, 80);
  
  size_t outlen = 80;
  
  ret = SM2Sign(&ctx, msg, data_length, (char *)id_str, id_len, pub, 130, pri, 64, sig, &outlen);
  if (ret != 0)
  {
    printf("test sm2 sign failed.\n");
    ret = RET_ERR;
    goto END;
  }
  
  for (int i = 0; i < args.times; ++i)
  {
    ret = SM2Verify(&ctx, msg, data_length, id_str, id_len, sig, outlen, pub, 130);
    if (ret != 0)
    {
      printf("test sm2 verify failed.\n");
      ret = RET_ERR;
      break;
    }
  }
  
END:
  delete[] msg;
  delete[] sig;
  SM2FreeCtx(&ctx);
  
  return NULL;
}


int test_sm2_multithread(int times,size_t data_length)
{
  int core_num = (int)sysconf(_SC_NPROCESSORS_ONLN);
  
  sm2_perf_test_args args;
  args.times = times;
  args.data_length = data_length;
  
/* 多线程满核性能测试：公钥预计算加密 */
  pthread_t tid[MAX_TID];
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_encrypt_precompute, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  unsigned long begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  unsigned long end = get_tick_count();
  
  double encrypt_precompute_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 en multi-thread perf:[precompute:%d][block size:%lu][tps:%lu][%.02lf Kbps]\n",
  1,data_length,(long)encrypt_precompute_count_per_second,data_length*encrypt_precompute_count_per_second/1024);
  
/* 多线程满核性能测试：通用加密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_encrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double encrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 en multi-thread perf:[precompute:%d][block size:%lu][tps:%lu][%.02lf Kbps]\n",
  0,data_length,(long)encrypt_count_per_second,data_length*encrypt_count_per_second/1024);
  
/* 多线程满核性能测试：解密 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_decrypt, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double decrypt_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 de multi-thread perf:[precompute:%d][block size:%lu][tps:%lu][%.02lf Kbps]\n",
  1,data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024);
  printf("SM2 de multi-thread perf:[precompute:%d][block size:%lu][tps:%lu][%.02lf Kbps]\n",
  0,data_length,(long)decrypt_count_per_second,data_length*decrypt_count_per_second/1024);
  
  
  /* 多线程满核性能测试：公钥预计算签名 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_sign_precompute, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double sign_precompute_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 sign multi-thread Performance:[precompute:%d][block size:%lu][tps :%lu]\n",
  1,data_length,(long)sign_precompute_count_per_second);
  
  /* 多线程满核性能测试：通用签名 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_sign, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double sign_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 sign multi-thread Performance:[precompute:%d][block size:%lu][tps :%lu]\n",
  0,data_length,(long)sign_count_per_second);
  
  
  /* 多线程满核性能测试：公钥预计算验签 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_verify_precompute, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double verify_precompute_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 verify multi-thread Performance:[precompute:%d][block size:%lu][tps :%lu]\n",
  1,data_length,(long)verify_precompute_count_per_second);
  
  /* 多线程满核性能测试：通用验签 */
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_create(&tid[i], NULL, thread_test_sm2_verify, (void*)&args);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with creating thread...");
      return RET_ERR;
    }
  }
  begin = get_tick_count();
  
  for (int i = 0; i < core_num; i++) {
    int ret = pthread_join(tid[i], NULL);
    if (ret != RET_OK) {
      printf("test_sm2_multithread failed with join thread...");
      return RET_ERR;
    }
  }
  
  end = get_tick_count();
  
  double verify_count_per_second = (float)times*core_num/((end - begin)/1000.0);
  
  printf("SM2 verify multi-thread Performance:[precompute:%d][block size:%lu][tps :%lu]\n",
  0,data_length,(long)verify_count_per_second);
  
  return 0;
}

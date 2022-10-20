/*
 
Copyright 2020, Tencent Technology (Shenzhen) Co Ltd
This file is part of the Tencent SM (Lite Version) Library.
 
*/

#ifndef test_sm4_perf_h
#define test_sm4_perf_h

int test_sm4_generate_key(int times,size_t data_length);
int test_sm4_ecb_encrypt_decrypt(int times,size_t data_length);
int test_sm4_cbc_encrypt_decrypt(int times,size_t data_length);
int test_sm4_gcm_encrypt_decrypt(int times,size_t data_length);
int test_sm4_gcm_nist_sp800_38d_encrypt_decrypt(int times,size_t data_length);

int test_sm4_ecb_nopadding_encrypt_decrypt(int times,size_t data_length);
int test_sm4_cbc_nopadding_encrypt_decrypt(int times,size_t data_length);
int test_sm4_ctr_nopadding_encrypt_decrypt(int times,size_t data_length);
int test_sm4_gcm_nopadding_encrypt_decrypt(int times,size_t data_length);
int test_sm4_gcm_nopadding_nist_sp800_38d_encrypt_decrypt(int times,size_t data_length);

int test_sm4_multithread(int times,size_t data_length);

//sm4分步计算
int test_sm4_ecb_steps_encrypt_decrypt(int times,size_t data_length);
int test_sm4_cbc_steps_encrypt_decrypt(int times,size_t data_length);

int test_sm4_ecb_nopadding_steps_encrypt_decrypt(int times,size_t data_length);
int test_sm4_cbc_nopadding_steps_encrypt_decrypt(int times,size_t data_length);
int test_sm4_ctr_nopadding_steps_encrypt_decrypt(int times,size_t data_length);
int test_sm4_gcm_nopadding_nist_sp800_38d_steps_encrypt_decrypt(int times,size_t data_length);
#endif /* test_sm4_perf_h */

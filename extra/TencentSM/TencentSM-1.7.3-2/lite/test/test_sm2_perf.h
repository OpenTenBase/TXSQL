/*
 
Copyright 2020, Tencent Technology (Shenzhen) Co Ltd
This file is part of the Tencent SM (Lite Version) Library.
 
*/

#ifndef test_sm2_perf_h
#define test_sm2_perf_h

#include <stdio.h>

int test_sm2_generate_keypair(int times,size_t data_length);
int test_sm2_encrypt_decrypt(int times,size_t data_length);
int test_sm2_sign_verify(int times,size_t data_length);

int test_sm2_multithread(int times,size_t data_length);

#endif /* test_sm2_perf_h */

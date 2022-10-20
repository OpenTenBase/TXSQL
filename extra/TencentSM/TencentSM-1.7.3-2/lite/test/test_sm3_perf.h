/*
 
Copyright 2020, Tencent Technology (Shenzhen) Co Ltd
This file is part of the Tencent SM (Lite Version) Library.
 
*/

#ifndef test_sm3_perf_h
#define test_sm3_perf_h

#include <stdio.h>

int test_sm3_md(int times,size_t data_length);
int test_sm3_hmac(int times,size_t data_length);
int test_sm3_multithread(int times,size_t data_length);

#endif /* test_sm3_perf_h */

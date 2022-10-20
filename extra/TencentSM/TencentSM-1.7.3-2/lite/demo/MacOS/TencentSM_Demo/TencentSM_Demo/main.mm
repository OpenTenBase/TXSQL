//
//  main.m
//  TencentSM_Demo
//
//  Created by medivhwu on 2019/9/2.
//  Copyright © 2019 TSM. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "test_global.hpp"
#include "string.h"
#include "sm.h"

const char* INTERFACE_NAME[]
= {
  "Version","SM2CtxSize","SM2InitCtx_Free","SM2InitCtxWithPubKey",
  "generatePrivateKey","generatePublicKey","generateKeyPair","SM2Decrypt",
  "SM2Encrypt","SM2Verify","SM2Sign",
  "SM3CtxSize","SM3Init","SM3UpdateFinal","SM3","SM3_hmac_steps","SM3_hmac","SM3KDF",
  "generateSM4Key","SM4_CBC_Encrypt","SM4_CBC_Decrypt","SM4_CBC_Encrypt_NoPadding",
  "SM4_CBC_Decrypt_NoPadding","SM4_ECB_Encrypt","SM4_ECB_Decrypt","SM4_ECB_Encrypt_NoPadding",
  "SM4_ECB_Decrypt_NoPadding","SM4_GCM_Encrypt","SM4_GCM_Decrypt","SM4_GCM_Encrypt_NoPadding",
  "SM4_GCM_Decrypt_NoPadding","SM4_GCM_Encrypt_Decrypt_NoPadding_NIST_SP800_38D","SM4_GCM_Encrypt_Decrypt_NIST_SP800_38D",
  "SM4_CBC_Encrypt_Steps","SM4_CBC_Decrypt_Steps","SM4_CBC_Encrypt_NoPadding_Steps","SM4_CBC_Decrypt_NoPadding_Steps",
 "SM4_ECB_Encrypt_Steps","SM4_ECB_Decrypt_Steps","SM4_ECB_Encrypt_NoPadding_Steps","SM4_ECB_Decrypt_NoPadding_Steps",
  "SM4_CTR_Encrypt_NoPadding_Steps","SM4_CTR_Decrypt_NoPadding_Steps",
    "SM4_GCM_Encrypt_NoPadding_Steps","SM4_GCM_Decrypt_NoPadding_Steps",
  "SM2DecryptWithMode",
  "SM2EncryptWithMode","SM2VerifyWithMode","SM2SignWithMode"
};

const static char* ABILITY_NAME[]
= {"sm2_generate_keypair_perf","sm2_encrypt_decrypt_perf","sm2_sign_verify_perf","test_sm2_multithread",
  "sm4_generate_key_perf","sm4_cbc_encrypt_decrypt_perf","sm4_cbc_nopadding_encrypt_decrypt_perf",
  "sm4_ecb_encrypt_decrypt_perf","sm4_ecb_nopadding_encrypt_decrypt_perf",
  "sm4_gcm_encrypt_decrypt_perf","sm4_gcm_nopadding_encrypt_decrypt_perf", 
  "sm4_gcm_nist_encrypt_decrypt_perf","sm4_gcm_nopadding_nist_encrypt_decrypt_perf",
  "sm4_ecb_steps_encrypt_decrypt_perf","sm4_ecb_nopadding_steps_encrypt_decrypt_perf",
  "sm4_cbc_steps_encrypt_decrypt_perf","sm4_cbc_nopadding_steps_encrypt_decrypt_perf",
  "sm4_ctr_nopadding_steps_encrypt_decrypt_perf","sm4_gcm_nopadding_steps_encrypt_decrypt_perf",
  "test_sm4_multithread",
 "test_sm3_multithread", "sm3_md_perf","sm3_hmac_perf"
};

const static int INTERFACE_LEN = sizeof(INTERFACE_NAME)/sizeof(char*);
const static int ABILITY_LEN = sizeof(ABILITY_NAME)/sizeof(char*);

int main(int argc, const char * argv[]) {
  @autoreleasepool {
    
    for (int i = 0; i < INTERFACE_LEN; i++) {
      testInterface((const char*)INTERFACE_NAME[i]);
    }
    
    for (int i = 0; i < ABILITY_LEN; i++) {
      test_ability((const char*)ABILITY_NAME[i]);
    }
  }
  return 0;
}

/**
 * 本文件仅定义全局性定义
 */
#ifndef _TC_GLOBAL_H_
#define _TC_GLOBAL_H_
#include <stdio.h>

#define SM3_BLOCK_SIZE    64
#define SM2_PUBKEY_STR_SIZE    130
#define DEFAULT_SM2_SIG_ID "1234567812345678"
  
typedef enum {
  SM_MD_SM3 = 1,
} SM_MD_TYPE;

#ifdef __ANDROID__
#include <android/log.h>
#endif
#include "tc_err.h"

#ifdef __x86_64__
#ifndef _WIN32 //x86_64 optimise，does not support windows
#ifdef __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR
      // iOS Simulator
  #elif TARGET_OS_IPHONE
      // iOS device
  #elif TARGET_OS_MAC
      // MacOS
      #define _X86_64_OPT_ASM_ECC
  #else
  #   error "Unknown Apple platform"
  #endif
#else
  #define _X86_64_OPT_ASM_ECC
#endif
#endif
#endif

#ifdef __aarch64__ //arm64
#ifdef __linux
#else
//does not support other system
#endif
#endif

#if defined(_X86_64_OPT_ASM_ECC)
#define _OPT_ASM_ECC
#endif


# if defined(__alpha) || defined(__sparcv9) || defined(__mips)
#  define MD32_REG_T long
# else
#  define MD32_REG_T int
# endif



#ifdef __ANDROID__
    #undef _OPT_ASM_ECC
#endif

#ifdef _WIN32
    #undef _SM3_BAESD_CSPRNG
    #undef _OPT_ASM_ECC
#else
  #define _SM3_BAESD_CSPRNG
#endif

#define _MEMORY_ERASE_PROTECTION /* 内存擦除保护,开启内存擦除保护，将最大限度在不安全的内存环境中保护敏感安全参数，但会导致性能的略微下降 */

#ifdef _DEBUG
#define FIXED_RANDOM_NUM
#endif

// #define _TSM_LOGFILE
#ifdef _TSM_LOGFILE
extern FILE* gflog;
#endif
//定义log打印函数
#ifdef __ANDROID__
    #ifdef _DEBUG
    #define LOGV(...) __android_log_print(ANDROID_LOG_INFO, "JNITag", __VA_ARGS__)
    #define SM_LOGA(...) __android_log_print(ANDROID_LOG_INFO, "JNITag", __VA_ARGS__)
    #endif
#else
    #ifdef _WIN32
        #ifdef _DEBUG
            #define LOGV(...) printf(__VA_ARGS__)
        #endif
        #define SM_LOGA(...) printf(__VA_ARGS__)
    #else
        #ifdef __linux
            #ifdef _DEBUG
                #ifdef _TSM_LOGFILE
                    // #define LOGV(...) { if(gflog != NULL) { fprintf(gflog, __VA_ARGS__); fflush(gflog); } }
                #else
                    #define LOGV(...) printf(__VA_ARGS__)
                #endif
            #endif
            #define SM_LOGA(...) printf(__VA_ARGS__)
        #endif
    #endif
#endif //OS_ANDROID
#ifdef LOGV
#define PRINT_BN(var_bn)    \
    do {    \
        if(var_bn != NULL) {    \
            if(var_bn->val != NULL) {   \
                char tmpstr[33];    \
                memset(tmpstr, 0x00, 33);   \
                tcsm_tc_bn_get_str(tmpstr, var_bn);    \
                LOGV("%s:%d: var_bn->val =  %s", __FILE__, __LINE__, tmpstr);    \
            } else {   \
                LOGV("%s:%d: var_bn->val == NULL", __FILE__, __LINE__); \
            }   \
        } else {  \
            LOGV("%s:%d: var_bn == NULL", __FILE__, __LINE__); \
        }   \
    } while(0)
#else
    #define LOGV(...)
    #define PRINT_BN(var_bn)
#endif
#ifndef SM_LOGA
    #define SM_LOGA(...)
#endif
// void byte2str(unsigned char *bytes, int len, char* strbuf);

#endif // _TC_GLOBAL_H_

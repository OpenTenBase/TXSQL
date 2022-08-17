#ifndef CDB_ENTERPRISE_ENCRYPTION_H
#define CDB_ENTERPRISE_ENCRYPTION_H

#include <mysql/plugin.h>
#include <mysql/psi/mysql_mutex.h>

extern ulong rsa_bits_threshold;
extern ulong dh_bits_threshold;
extern ulong dsa_bits_threshold;
extern mysql_mutex_t dh_params_lock, dsa_params_lock, rsa_key_lock, dh_key_lock,
    dsa_key_lock;

#define ALGORITHM_LEN 5
#define KEY_HEADTAIL_LEN 32
#define KEY_FILENAME_LEN 64
#define PRIVATE_KEY_TYPE 0
#define PUBLIC_KEY_TYPE 1
#define OPENSSL_SAFE_MINIMUM 1024

#ifdef WIN32
#define PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
#define PLUGIN_EXPORT extern "C"
#endif

#endif

#ifdef __cplusplus


extern "C" {
#endif
#ifndef SM3_INCLUDED
#define SM3_INCLUDED

#define SM3_HASH_SIZE 32
#define SM3_SCRAMBLE_LENGTH  32
#define SM3_SCRAMBLED_PASSWORD_CHAR_LENGTH (SM3_SCRAMBLE_LENGTH*2+1)

#ifndef UNUSED
#define UNUSED(v) ((void)(v))
#endif

#include "my_inttypes.h"

void txsql_make_scrambled_password(char *to, const unsigned char *password);

void my_make_scrambled_password_sm3(char *to, const unsigned char *password, size_t pass_len);

bool check_scramble_sm3(unsigned char *scramble_arg,
	unsigned char *message,
	unsigned char *hash_stage2);

void get_salt_from_sm3_password(uint8 *hash_stage2, const char *password);

void scramble_sm3(char *to, const unsigned char *message, const char *password);

void sm3(unsigned char *input, int ilen, unsigned char output[32]);

#endif
#ifdef __cplusplus
}
#endif

/*===================================================================
*
*
*
* Copyright (C) 2010, 2013 wenghaixing.
*
* Created on 2016-08-17 by wenghaixing (wenghaixing@unionpay.com)
*
* -------------------------------------------------------------------
*
* Description
* this is txsql.gm sm3 encrypt source code
* see that www.oscca.gov.cn/News/201012/News_1199.htm
* download the sm3 pdf.
* -------------------------------------------------------------------
*
* Change Log
*
*====================================================================*/

#include "password.h"
#include "mysql.h"
#include "sm3.h"
#include "my_inttypes.h"
#include <string.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/hmac.h>


/** =======================================================
*  Then we will define the func used in mysql auth
*  =======================================================*/

#define SM3_PVERSION41_CHAR '*'

/** =======================================================
*  We use openssl's SM3 function
*  =======================================================*/
void sm3(unsigned char *input, int ilen,
	unsigned char output[32])
{
  EVP_MD_CTX *mdctx = NULL;
  const EVP_MD *md;
  unsigned int md_len;
  OpenSSL_add_all_digests();
  
  md = EVP_get_digestbyname("SM3");
  if (!md) {
    /* error , return */
    return;
  }

  mdctx = EVP_MD_CTX_create();
  if (!mdctx) {
    /* error , return */
    return;
  }

  EVP_MD_CTX_init(mdctx);
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, input, ilen);
  EVP_DigestFinal_ex(mdctx, output, &md_len);

  /* release context */
  EVP_MD_CTX_destroy(mdctx);

  return;
}


/** =======================================================
*  output = HMAC-SM#( hmac key, input buffer )
*  =======================================================*/
void sm3_hmac(unsigned char *key, int keylen,
	unsigned char *input, int ilen,
	unsigned char output[32])
{
  EVP_MD_CTX *mdctx = NULL;
  const EVP_MD *md;
  unsigned int md_len;
  OpenSSL_add_all_digests();
  
  md = EVP_get_digestbyname("SM3");
  if (!md) {
    /* error , return */
    return;
  }

  mdctx = EVP_MD_CTX_create();
  if (!mdctx) {
    /* error , return */
    return;
  }

  HMAC(md, (const unsigned char*)(key), keylen,
       (const unsigned char*)(input), ilen,
       output, &md_len);
  
  return;
}

/** =======================================================
*  Simple Crypt Function
*  =======================================================*/

static void my_crypt_sm3(char *to, const uchar *s1, const uchar *s2, uint len)
{
  const uint8 *s1_end = s1 + len;
  while (s1 < s1_end)
    *to++ = *s1++ ^ *s2++;
}

/** =======================================================
*  Convert into a lower case char
*  =======================================================*/
static inline uint8 sm3_char_val(uint8 X)
{
  return (uint)(X >= '0' && X <= '9' ? X - '0' :
         X >= 'A' && X <= 'Z' ? X - 'A' + 10 : X - 'a' + 10);
}

/** =======================================================
*  HEX2OCTET
*  =======================================================*/

static void sm3_hex2octet(uint8 *to, const char *str, uint len)
{
  const char *str_end = str + len;
  while (str < str_end) {
    register char tmp = sm3_char_val(*str++);
    *to++ = (tmp << 4) | sm3_char_val(*str++);
  }
}

static const char sm3_dig_vec_upper[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char sm3_dig_vec_lower[] = "0123456789abcdefghijklmnopqrstuvwxyz";

/** =======================================================
*  OCTET2HEX
*  =======================================================*/
static char *sm3_octet2hex(char *to, const char *str, uint len)
{
  UNUSED(sm3_dig_vec_lower);

  const char *str_end = str + len;
  for (; str != str_end; ++str)
  {
    *to++ = sm3_dig_vec_upper[((uchar)*str) >> 4];
    *to++ = sm3_dig_vec_upper[((uchar)*str) & 0x0F];
  }
  *to = '\0';
  return to;
}

/** =======================================================
*  Caculate 2 stage sm3 hash
*  =======================================================*/
static
void compute_two_stage_sm3_hash(const unsigned char *password,
size_t pass_len,
unsigned char *hash_stage1,
unsigned char *hash_stage2)
{
  /* Stage 1: hash password */
  sm3((unsigned char *)password, pass_len, hash_stage1);


  /* Stage 2 : hash first stage's output. */
  sm3(hash_stage1, SM3_HASH_SIZE, hash_stage2);
}

/** =======================================================
*  Use scramble to encrypt password with sm3
*  We think it's better to use hmac with scramble string
*  =======================================================*/
void scramble_sm3_from_clear_text(char *to, const unsigned char *message, const char *password)
{
  unsigned char hash_stage1[SM3_HASH_SIZE];
  unsigned char hash_stage2[SM3_HASH_SIZE];

  /* Two stage SM3 hash of the password. */
  compute_two_stage_sm3_hash((unsigned char *)password, strlen(password),
  hash_stage1, hash_stage2);

  sm3_hmac((unsigned char*)message, SCRAMBLE_LENGTH, hash_stage2, SM3_HASH_SIZE, (unsigned char *)to);

  my_crypt_sm3(to, (const uchar *)to, hash_stage1, SM3_SCRAMBLE_LENGTH);
}

void scramble_sm3(char *to, const unsigned char *message, const char *password)
{
  scramble_sm3_from_clear_text(to, message, password);
}

bool check_scramble_sm3(unsigned char *scramble_arg,
	unsigned char *message,
	unsigned char *hash_stage2)
{
  unsigned char buf[SM3_HASH_SIZE];
  unsigned char hash_stage2_reassured[SM3_HASH_SIZE];
  /* create key to encrypt scramble */

  memset(buf, 0x00, SM3_HASH_SIZE);

  sm3_hmac(message, SCRAMBLE_LENGTH, hash_stage2, SM3_HASH_SIZE, buf);

  /* encrypt scramble */
  my_crypt_sm3((char *)buf, buf, scramble_arg, SM3_SCRAMBLE_LENGTH);

  /* now buf supposedly contains hash_stage1: so we can get hash_stage2 */
  sm3(buf, SM3_HASH_SIZE, hash_stage2_reassured);

  return (memcmp(hash_stage2, hash_stage2_reassured, SM3_HASH_SIZE)) ? true : false;
}

/*
Convert scrambled password from asciiz hex string to binary form.

SYNOPSIS
get_salt_from_password()
res       OUT buf to hold password. Must be at least SM3_HASH_SIZE
bytes long.
password  IN  4.1.1 version value of user.password
*/

void get_salt_from_sm3_password(uint8 *hash_stage2, const char *password)
{
  sm3_hex2octet(hash_stage2, password + 1 /* skip '*' */, SM3_HASH_SIZE * 2);
}

/*
Convert scrambled password from binary form to asciiz hex string.
SYNOPSIS
make_password_from_salt()
to    OUT store resulting string here, 2*SHA1_HASH_SIZE+2 bytes
salt  IN  password in salt format
*/

void make_sm3_password_from_salt(char *to, const uint8 *hash_stage2)
{
  *to++ = SM3_PVERSION41_CHAR;
  sm3_octet2hex(to, (const char*)hash_stage2, SM3_HASH_SIZE);
}


void my_make_scrambled_password_sm3(char *to, const unsigned char *password, size_t pass_len)
{
  unsigned char hash_stage2[SM3_HASH_SIZE];
  compute_two_stage_sm3_hash(password, pass_len,
                            (unsigned char*)to, hash_stage2);
  *to++ = SM3_PVERSION41_CHAR;
  sm3_octet2hex(to, (const char*)hash_stage2, SM3_HASH_SIZE);
}


void txsql_make_scrambled_password(char *to, const unsigned char *password)
{
  my_make_scrambled_password_sm3(to, password, strlen((const char *)password));
}

#include <openssl/err.h>
#include "openssl/evp.h"
#include "openssl/rand.h"
#include "openssl/ossl_typ.h"
#include "openssl/rsa.h"
#include "openssl/ec.h"
#include "openssl/hmac.h"
#include "openssl/x509.h"
#include "string.h"

void print_bytes(const unsigned char *beg, const unsigned char *end);

void Hex2Str( const unsigned char *sSrc,  unsigned char *sDest, int nSrcLen );

int Sm3Digest(unsigned char *data,int dataLen,unsigned char *digest,int *digestLen);

int Sm3Hmac(unsigned char *data,int dataLen,unsigned char *hmac,int *hmacLen,
				unsigned char *hmacKey,int keyLen);

int Sm4CbcEncrypt(unsigned char *source,int sourceLength,
				unsigned char *cipherText,int *cipherLength,
				unsigned char *key,unsigned char *iv, bool padding = true);

int Sm4CbcDecrypt(unsigned char *source,int sourceLength,
				unsigned char *plainText,int *plainTextLength,
				unsigned char *key,unsigned char *iv, bool padding = true);

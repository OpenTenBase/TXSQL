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
int Sm2GetKey(unsigned char *pubKey,unsigned char *priKey);

EVP_PKEY *Getpkey(int curve_id, uint8_t *pubKey, uint8_t *priKey, int flag);
int Sm2Encrypt(unsigned char *pubKey, unsigned char *source, int sourceLength,
				unsigned char *cipherText, int *cipherLength);


int Sm2Decrypt(unsigned char *priKey, unsigned char *source, int sourceLength,
				unsigned char *plainText, int *plainTextLength); 

int Sm3Digest(unsigned char *data,int dataLen,unsigned char *digest,int *digestLen);

int Sm3Hmac(unsigned char *data,int dataLen,unsigned char *hmac,int *hmacLen,
				unsigned char *hmacKey,int keyLen);
int Sm2Sign(unsigned char *pubKey,unsigned char *priKey,unsigned char *msg,int msgLen,
		unsigned char *sig,int *sigLen);

int Sm2Verify(unsigned char *pubKey,unsigned char *sig,int sigLen,unsigned char *msg,int msgLen);

int Sm4CbcEncrypt(unsigned char *source,int sourceLength,
				unsigned char *cipherText,int *cipherLength,
				unsigned char *key,unsigned char *iv, bool padding = true);

int Sm4CbcDecrypt(unsigned char *source,int sourceLength,
				unsigned char *plainText,int *plainTextLength,
				unsigned char *key,unsigned char *iv, bool padding = true);


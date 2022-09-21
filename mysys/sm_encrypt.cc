#include "sm_encrypt.h"

void Hex2Str( const unsigned char *sSrc,unsigned char *sDest, int nSrcLen )
{
    int  i;
    char szTmp[3];

    for( i = 0; i < nSrcLen; i++ )
    {
        sprintf( szTmp, "%02X", sSrc[i] );
        memcpy( &sDest[i * 2], szTmp, 2 );
    }
    return ;
}

int Sm3Hmac(unsigned char *data,int dataLen,unsigned char *hmac,int *hmacLen,
				unsigned char *hmacKey,int keyLen)
{
	
	if ( data == NULL || dataLen <= 0 )
	{
		printf("Sm3Hmac data is NULL");
		return ( -1 );
	}
	if ( hmac == NULL )
	{
		printf("hmac is NULL");
		return ( -1 );
	}
	if ( hmacLen == NULL )
	{
		printf("hmacLen is NULL");
		return ( -1 );
	}
	if ( hmacKey == NULL || keyLen < 0 )
	{
		printf("hmacKey is NULL");
		return ( -1 );
	}
	
	const EVP_MD *md = EVP_sm3();
	unsigned int len;
	HMAC_CTX *hmac_ctx = HMAC_CTX_new();
	HMAC_Init_ex(hmac_ctx, hmacKey,keyLen,md, NULL);
	HMAC_Update(hmac_ctx, data, dataLen);
	HMAC_Final(hmac_ctx, hmac, &len);
	*hmacLen = len;
	HMAC_CTX_free(hmac_ctx);

	return ( 0 );
}

void print_bytes(const unsigned char *beg, const unsigned char *end) {
	const unsigned char *pbeg = beg, *pend = end;
    while (pbeg != pend) fprintf(stderr,"%02x", (unsigned)*pbeg++);
    fprintf(stderr,"\n");
}

int Sm3Digest(unsigned char *data,int dataLen,unsigned char *digest,int *digestLen)
{
	
	if ( data == NULL || dataLen <= 0 )
	{
		printf("Sm3Hmac data is NULL");
		return ( -1 );
	}
	if ( digest == NULL )
	{
		printf("digest is NULL");
		return ( -1 );
	}
	if ( digestLen == NULL )
	{
		printf("digestLen is NULL");
		return ( -1 );
	}
	
	const EVP_MD *md = EVP_sm3();
	EVP_MD_CTX *md_ctx = EVP_MD_CTX_new();
	EVP_DigestInit(md_ctx, md);
	EVP_DigestUpdate(md_ctx, data, dataLen);
	EVP_DigestFinal_ex(md_ctx, digest, (unsigned int*)digestLen);
	EVP_MD_CTX_free(md_ctx);

	return ( 0 );
}

int Sm4CbcEncrypt(unsigned char *source,int sourceLength,
				unsigned char *cipherText,int *cipherLength,
				unsigned char *key,unsigned char *iv,bool padding)
{

	if ( source == NULL || sourceLength <= 0 )
	{
		printf("source is NULL");

		return ( -1 );
	}
	if ( cipherText == NULL )
	{
		printf("cipherText is NULL");
		return ( -1 );
	}
	if ( cipherLength == NULL )
	{
		printf("cipherLength is NULL");
		return ( -1 );
	}
	if ( key == NULL )
	{
		printf("key is NULL");
		return ( -1 );
	}
	if ( iv == NULL )
	{
		printf("iv is NULL");
		return ( -1 );
	}

	int tmplen = 0, clen = 0;
	const EVP_CIPHER *cipher = NULL;
	cipher = EVP_sm4_cbc();

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_EncryptInit(ctx, cipher, key, iv);
	if (!EVP_CIPHER_CTX_set_padding(ctx, padding))
	{
		printf("set no padding failed\n");
		return ( -1 );
	}
	EVP_EncryptUpdate(ctx, cipherText, &clen, source, sourceLength);
	EVP_EncryptFinal_ex(ctx, cipherText + clen, &tmplen);
	clen += tmplen;
	EVP_CIPHER_CTX_free(ctx);
	*cipherLength = clen;

	return ( 0 );
}

int Sm4CbcDecrypt(unsigned char *source,int sourceLength,
				unsigned char *plainText,int *plainTextLength,
				unsigned char *key,unsigned char *iv,bool padding)
{

	if ( source == NULL || sourceLength <= 0 )
	{
		printf("source is NULL");

		return ( -1 );
	}
	if ( plainText == NULL )
	{
		printf("plainText is NULL");
		return ( -1 );
	}
	if ( plainTextLength == NULL )
	{
		printf("plainTextLength is NULL");
		return ( -1 );
	}
	if ( key == NULL )
	{
		printf("key is NULL");
		return ( -1 );
	}
	if ( iv == NULL )
	{
		printf("iv is NULL");
		return ( -1 );
	}

	int tmplen = 0, plen = 0;
	const EVP_CIPHER *cipher = NULL;
	cipher = EVP_sm4_cbc();

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit(ctx, cipher, key, iv);
        if (!EVP_CIPHER_CTX_set_padding(ctx, padding))
        {
          printf("set no padding failed\n");
          return ( -1 );
        }
	EVP_DecryptUpdate(ctx, plainText, &plen, source, sourceLength);
	EVP_DecryptFinal_ex(ctx, plainText + plen, &tmplen);
	plen += tmplen;
	EVP_CIPHER_CTX_free(ctx);
	*plainTextLength = plen;

	return ( 0 );
}

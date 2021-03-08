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

int Sm2GetKey(unsigned char *pubKey,unsigned char *priKey)
{
	int rv = -1;
	int buflen = 0;
	const EC_GROUP* group = NULL;
	BIGNUM * privkey = NULL;
	EC_POINT * pubkey = NULL;
	unsigned char vch[32] = { 0 };
	unsigned char pubbuf[128] = { 0 };
	EC_KEY *ec_key = NULL;
	int pubkeyLen;

	if (!pubKey || !priKey )
	{
		return -1;
	}
	RAND_bytes(vch, 32);

	privkey = BN_new();
	ec_key = EC_KEY_new_by_curve_name(NID_sm2);
	group = EC_KEY_get0_group(ec_key);
	pubkey = EC_POINT_new(group);

	if (privkey == NULL || NULL == pubkey || ec_key == NULL)
	{
		goto end;
	}

	if (BN_bin2bn(vch, 32, privkey))
	{
		if (!EC_POINT_mul(group, pubkey, privkey, NULL, NULL, NULL))
		{
			goto end;
		}
	}

	BN_bn2binpad(privkey, priKey, 32);

	buflen = EC_POINT_point2oct(group, pubkey, EC_KEY_get_conv_form(ec_key), pubbuf, sizeof(pubbuf), NULL);
	if (buflen >0)
	{
		pubkeyLen = buflen - 1;
		memcpy(pubKey, pubbuf + 1, pubkeyLen);
	}
	else
	{
		goto end;
	}
	
	rv = 0;
	end:
		if (pubkey) EC_POINT_free(pubkey);
		if (privkey) BN_free(privkey);
		if (ec_key) EC_KEY_free(ec_key);

	 return rv;
}

EVP_PKEY *Getpkey(int curve_id, uint8_t *pubKey, uint8_t *priKey, int flag)
{
	BIGNUM *r = BN_new();
	BIGNUM *x = BN_new();
	BIGNUM *y = BN_new();  

	EC_KEY *eckey = EC_KEY_new_by_curve_name(curve_id);
	EVP_PKEY *pkey = EVP_PKEY_new();
	EVP_PKEY_set1_EC_KEY(pkey, eckey);

	if(flag&2) 
	{
		x = BN_bin2bn(pubKey, 32, x);
		y = BN_bin2bn(pubKey+32, 32, y);
		EC_POINT *point = EC_POINT_new(EC_KEY_get0_group(EVP_PKEY_get0_EC_KEY(pkey)));
		if(point == NULL)
		{
			//printf("genkey error\n");
			return NULL;
		}
		if(!EC_POINT_set_affine_coordinates_GFp(EC_KEY_get0_group(EVP_PKEY_get0_EC_KEY(pkey)), point, x, y, NULL))
		{
			//printf("genkey error 1\n");
			return NULL;
		}
	   if(! EC_KEY_set_public_key(EVP_PKEY_get0_EC_KEY(pkey), point))
	   {
		   //printf("genkey error 2\n");
		   return NULL;
	   }
	} 
	if(flag&1) 
	{
		r = BN_bin2bn(priKey, 32, r);
		EC_KEY_set_private_key(EVP_PKEY_get0_EC_KEY(pkey), r);
	}
	
	BN_free(r);
	BN_free(x);
	BN_free(y);
	EC_KEY_free(eckey);
	
	return pkey;
}

int Sm2Encrypt(unsigned char *pubKey, unsigned char *source, int sourceLength,
				unsigned char *cipherText, int *cipherLength)
{
	
	if ( pubKey == NULL )
	{
		printf("Sm2Encrypt pubKey is NULL");
		return ( -1 );
	}
	if ( source == NULL || sourceLength <= 0 )
	{
		printf("Sm2Encrypt source is NULL");
		
		return ( -1 );
	}
	if ( cipherText == NULL )
	{
		printf("Sm2Encrypt cipherText is NULL");
		return ( -1 );
	}
	if ( cipherLength == NULL )
	{
		printf("Sm2Encrypt cipherLength is NULL");
		return ( -1 );
	}
	
	
	size_t clen = sourceLength + 200;
	EVP_PKEY *pkey = NULL;
	EVP_PKEY_CTX *pkctx = NULL;

	if (!(pkey = Getpkey(NID_sm2, pubKey, NULL, 2))) 
	{
		printf("Getpkey err");
		return ( -1 );
	}
	
	if ((pkctx = EVP_PKEY_CTX_new_pkey_id(pkey, NID_sm2, NULL)) == NULL)
	{
		EVP_PKEY_free(pkey);
		printf("EVP_PKEY_CTX_new err");
		return ( -1 );
	}
	if (!EVP_PKEY_encrypt_init(pkctx)) 
	{
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(pkctx);
		printf("EVP_PKEY_CTX_new err");
		return ( -1 );
	}

	if (!EVP_PKEY_encrypt(pkctx, cipherText, &clen, source, sourceLength)) 
	{
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(pkctx);
		printf("EVP_PKEY_encrypt err");
		return ( -1 );
	}
	*cipherLength = clen;

	EVP_PKEY_free(pkey);
	EVP_PKEY_CTX_free(pkctx);
	
	return ( 0 );
}

int Sm2Decrypt(unsigned char *priKey, unsigned char *source, int sourceLength,
				unsigned char *plainText, int *plainTextLength) 
{
	
	if ( priKey == NULL )
	{
		printf("Sm2Decrypt priKey is NULL");
		return ( -1 );
	}
	if ( source == NULL || sourceLength <= 0 )
	{
		printf("Sm2Decrypt source is NULL");
		
		return ( -1 );
	}
	if ( plainText == NULL )
	{
		printf("Sm2Decrypt plainText is NULL");
		return ( -1 );
	}
	if ( plainTextLength == NULL )
	{
		printf("Sm2Decrypt plainTextLength is NULL");
		return ( -1 );
	}
	
	
	size_t plen = sourceLength + 200;
	EVP_PKEY *pkey = NULL;
	EVP_PKEY_CTX *pkctx = NULL;

	if (!(pkey = Getpkey(NID_sm2, NULL, priKey, 1))) 
	{
		printf("Getpkey err");
		return ( -1 );
	}

	if ((pkctx = EVP_PKEY_CTX_new_pkey_id(pkey, NID_sm2, NULL)) == NULL)
	{
		EVP_PKEY_free(pkey);
		printf("EVP_PKEY_CTX_new err");
		return ( -1 );
	}

	if (!EVP_PKEY_decrypt_init(pkctx)) 
	{
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(pkctx);
		printf("EVP_PKEY_decrypt_init err");
		return ( -1 );
	}

	int i_ret = 0;
	if (!(i_ret = EVP_PKEY_decrypt(pkctx, plainText, &plen, source, sourceLength))) 
	{
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(pkctx);
		printf("EVP_PKEY_decrypt err");
		return ( -1 );
	}
	*plainTextLength = plen;

	EVP_PKEY_free(pkey);
	EVP_PKEY_CTX_free(pkctx);
	
	return ( 0 );
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

int Sm2Sign(unsigned char *pubKey,unsigned char *priKey,unsigned char *msg,int msgLen,unsigned char *sig,int *sigLen)
{

	if ( pubKey == NULL )
	{
		printf("Sm2Sign pubKey is NULL");
		return ( -1 );
	}
	if ( priKey == NULL )
	{
		printf("Sm2Sign priKey is NULL");
		return ( -1 );
	}
	if ( msg == NULL || msgLen <= 0 )
	{
		printf("Sm2Sign msg is NULL");
		return ( -1 );
	}
	if ( sig == NULL )
	{
		printf("Sm2Sign sig is NULL");
		return ( -1 );
	}
	if ( sigLen == NULL )
	{
		printf("Sm2Sign sigLen is NULL");
		return ( -1 );
	}

	EVP_PKEY *pkey = NULL;
	if (!(pkey = Getpkey(NID_sm2, pubKey, priKey, 3)))
	{
		printf("Getpkey error");
		return ( -1 );
	}

	size_t sig_len = 0;
	const EVP_MD *md = EVP_sm3();
	EVP_MD_CTX *mdctx = NULL;

	EVP_PKEY_CTX *pkctx = NULL;

	if ((pkctx = EVP_PKEY_CTX_new_pkey_id(pkey, NID_sm2, NULL)) == NULL)
	{
		printf("EVP_PKEY_CTX_new_pkey_id error");
		EVP_PKEY_free(pkey);
		return ( -1 );
	}

	if (!(mdctx = EVP_MD_CTX_create()))
	{
		printf("EVP_MD_CTX_create error");
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(pkctx);
		return ( -1 );
	}

	if (EVP_DigestSignInit(mdctx, &pkctx, md, NULL, pkey) <= 0)
	{
		printf("EVP_DigestSignInit error");
		EVP_PKEY_free(pkey);
		EVP_PKEY_CTX_free(pkctx);
		return ( -1 );
	}

	if (!EVP_SignUpdate(mdctx, msg, msgLen))
	{
		printf("EVP_SignUpdate error");
		EVP_PKEY_free(pkey);
		EVP_MD_CTX_free(mdctx);
		return ( -1 );
	}

	if (!EVP_SignFinal(mdctx, sig, (unsigned int*)&sig_len, pkey))
	{
		printf("EVP_SignFinal error");
		EVP_PKEY_free(pkey);
		EVP_MD_CTX_free(mdctx);
		return ( -1 );
	}

	*sigLen = sig_len;

	EVP_PKEY_free(pkey);
	EVP_MD_CTX_free(mdctx);

	return ( 0 );
}



int Sm2Verify(unsigned char *pubKey,unsigned char *sig,int sigLen,unsigned char *msg,int msgLen)
{

	if ( pubKey == NULL )
	{
		printf("Sm2Verify pubKey is NULL");
		return ( -1 );
	}
	if ( msg == NULL || msgLen <= 0 )
	{
		printf("Sm2Verify msg is NULL");
		return ( -1 );
	}
	if ( sig == NULL || sigLen <= 0 )
	{
		printf("Sm2Verify sig is NULL");
		return ( -1 );
	}

	int i_ret = 0;

	EVP_PKEY *pkey = NULL;
	if (!(pkey = Getpkey(NID_sm2, pubKey, NULL, 2)))
	{
		printf("Getpkey error");
		return ( -1 );
	}

	const EVP_MD *md = EVP_sm3();

	EVP_PKEY_CTX *pkctx = NULL;
	EVP_MD_CTX *mdctx = NULL;

	if (!(pkctx = EVP_PKEY_CTX_new(pkey, NULL)))
	{
		EVP_PKEY_free(pkey);
		return ( -1 );
	}
	if (!(mdctx = EVP_MD_CTX_create()))
	{
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		return ( -1 );
	}
	if (EVP_DigestSignInit(mdctx, &pkctx, md, NULL, pkey) <= 0)
	{
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		return ( -1 );
	}
	if (!EVP_VerifyUpdate(mdctx, msg, msgLen))
	{
		fprintf(stderr, "error: %s %d\n", __FILE__, __LINE__);
		return ( -1 );
	 }

	if ((i_ret = EVP_VerifyFinal(mdctx, sig, sigLen, pkey)) != 1)
	{
		fprintf(stderr, "SM2 sign and verify with SM3 hash failed!\n");
		return ( -1 );
	 }

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

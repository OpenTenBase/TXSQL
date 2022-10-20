/**
 * @brief hmac for sm3
 * @author jiangweiti
 * @date 2020-06-02
 * @version 1.0.0
 * 
 * @copyright Copyright (C) 2020   Tencent Co.Ltd
 * @par Historys:
 * <table>
 * <tr><th>Date       <th>Version <th>Author  <th>Description
 * <tr><td>2019-11-17 <td>1.0     <td>wangh     <td>example
 * </table>
 */
#include <stdio.h>
#include "../include/tc.h"
#include "../include/tc_sm3.h"

#define IPAD	0x36
#define OPAD	0x5C

TstHmacSm3Ctx* tcsm_sm3_hmac_init(const unsigned char *key, size_t key_len)
{
	int i;
    if(key == NULL) {
        return NULL;
    }
    TstHmacSm3Ctx* ctx = tcsm_tc_secure_malloc(sizeof(*ctx));
    if(ctx == NULL) {
        return NULL;
    }

	if (key_len <= SM3_BLOCK_SIZE) {
		memcpy(ctx->key, key, key_len);
		memset(ctx->key + key_len, 0, SM3_BLOCK_SIZE - key_len);
	} else {
		tcsm_sm3_init_opt(&ctx->ctx);
		tcsm_sm3_update_opt(&ctx->ctx, key, key_len);
		tcsm_sm3_final_opt(&ctx->ctx, ctx->key);
		memset(ctx->key + SM3_DIGEST_LENGTH, 0,
			SM3_BLOCK_SIZE - SM3_DIGEST_LENGTH);
	}
	for (i = 0; i < SM3_BLOCK_SIZE; i++) {
		ctx->key[i] ^= IPAD;
	}

	tcsm_sm3_init_opt(&ctx->ctx);
	tcsm_sm3_update_opt(&ctx->ctx, ctx->key, SM3_BLOCK_SIZE);
    return ctx;
}

int tcsm_sm3_hmac_update(TstHmacSm3Ctx *ctx,
	const unsigned char *data, size_t data_len)
{
    if(ctx == NULL || data == NULL) {
        return ERR_SM3_HMAC_ARGUMENT;
    }
	tcsm_sm3_update_opt(&ctx->ctx, data, data_len);
    return ERR_TENCENTSM_OK;
}

int tcsm_sm3_hmac_final(TstHmacSm3Ctx *ctx, unsigned char mac[SM3_HMAC_SIZE])
{
	int i;
    if(ctx == NULL || mac == NULL) {
        return ERR_SM3_HMAC_ARGUMENT;
    }
	for (i = 0; i < SM3_BLOCK_SIZE; i++) {
		ctx->key[i] ^= (IPAD ^ OPAD);
	}
	tcsm_sm3_final_opt(&ctx->ctx, mac);
	tcsm_sm3_init_opt(&ctx->ctx);
	tcsm_sm3_update_opt(&ctx->ctx, ctx->key, SM3_BLOCK_SIZE);
	tcsm_sm3_update_opt(&ctx->ctx, mac, SM3_DIGEST_LENGTH);
	tcsm_sm3_final_opt(&ctx->ctx, mac);

	memset(ctx, 0, sizeof(*ctx));
    tcsm_tc_secure_free(ctx);
    return ERR_TENCENTSM_OK;
}

int tcsm_sm3_hmac(const unsigned char *data, size_t data_len,
	const unsigned char *key, size_t key_len,
	unsigned char mac[SM3_HMAC_SIZE])
{
	TstHmacSm3Ctx* ctx;

    if(data == NULL || key == NULL) {
        return ERR_SM3_HMAC_ARGUMENT;
    }
	ctx = tcsm_sm3_hmac_init(key, key_len);
    if(ctx == NULL) {
        return ERR_SM3_HMAC_ERR;
    }
	if(tcsm_sm3_hmac_update(ctx, data, data_len) != ERR_TENCENTSM_OK) {
        return ERR_SM3_HMAC_ERR;
    }
	if(tcsm_sm3_hmac_final(ctx, mac) != ERR_TENCENTSM_OK) {
        return ERR_SM3_HMAC_ERR;
    }
    return ERR_TENCENTSM_OK;
}

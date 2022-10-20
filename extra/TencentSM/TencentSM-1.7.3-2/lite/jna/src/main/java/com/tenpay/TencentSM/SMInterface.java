package com.tenpay.TencentSM;


import com.sun.jna.Library;
import com.sun.jna.Pointer;

interface SMInterface extends Library {

    int SM2CtxSize();

    void SM2InitCtx(Pointer ctx);

    void SM2FreeCtx(Pointer ctx);

    int generateKeyPair(Pointer ctx, byte[] prikey, byte[] pubkey);

    int SM2Encrypt(Pointer ctx, final byte[] plaintext, SizeT plaintextlen, final byte[] pubkey, SizeT pubkeylen,
            byte[] ciphertext, SizeTByReference ciphertextlen);

    int SM2Decrypt(Pointer ctx, final byte[] ciphertext, SizeT ciphertextlen, final byte[] prikey, SizeT prikeylen,
            byte[] plaintext, SizeTByReference plaintextlen);

    int SM2DecryptWithMode(Pointer ctx, final byte[] ciphertext, SizeT ciphertextlen, final byte[] prikey,
            SizeT prikeylen, byte[] plaintext, SizeTByReference plaintextlen, int mode);

    int SM2Sign(Pointer ctx, final byte[] msg, SizeT msglen, final byte[] id, SizeT idlen, final byte[] strPubKey,
            SizeT pubkeylen, final byte[] strPriKey, SizeT prikeylen, byte[] sig, SizeTByReference siglen);

    int SM2Verify(Pointer ctx, final byte[] msg, SizeT msglen, final byte[] id, SizeT idlen, final byte[] sig,
            SizeT siglen, final byte[] pubkey, SizeT pubkeylen);

    int SM3CtxSize();

    void SM3Init(Pointer ctx);

    void SM3Update(Pointer ctx, final byte[] data, SizeT data_len);

    void SM3Final(Pointer ctx, byte[] digest);

    void SM4_CBC_Encrypt(byte[] in, SizeT inlen, byte[] out, SizeTByReference outlen, byte[] key, byte[] iv);

    void SM4_CBC_Decrypt(byte[] in, SizeT inlen, byte[] out, SizeTByReference outlen, byte[] key, byte[] iv);

    void SM4_ECB_Encrypt(byte[] in, SizeT inlen, byte[] out, SizeTByReference outlen, byte[] key);

    void SM4_ECB_Decrypt(byte[] in, SizeT inlen, byte[] out, SizeTByReference outlen, byte[] key);
}
package com.tenpay.TencentSM;

import java.util.Arrays;


public class SM4Algo extends SMAlgoBase {

    public static byte[] encrypt_ecb(byte[] plaintext, byte[] key) throws Exception {
        if (key.length != 16) {
            throw new SMException(-27);
        }

        if (plaintext.length <= 0) {
            throw new SMException(-26);
        }

        int len = plaintext.length;
        byte[] ciphertext = new byte[(len / 16 + 1) * 16];
        SizeTByReference outlen = new SizeTByReference();

        SMAlgoBase.getSMInstance().SM4_ECB_Encrypt(plaintext, new SizeT(len), ciphertext, outlen, key);
        if (outlen.getValue().intValue() != (len / 16 + 1) * 16) {
            throw new SMException(-26);
        }

        return ciphertext;
    }

    public static byte[] decrypt_ecb(byte[] ciphertext, byte[] key) throws SMException {
        if (key.length != 16) {
            throw new SMException(-27);
        }

        if (ciphertext.length <= 0) {
            throw new SMException(-25);
        }

        int len = ciphertext.length;
        byte[] plaintext = new byte[len];
        SizeTByReference outlen = new SizeTByReference();

        SMAlgoBase.getSMInstance().SM4_ECB_Decrypt(ciphertext, new SizeT(len), plaintext, outlen, key);

        // ���������ݳ���С�ڵ���0�Ļ��������쳣����
        if (outlen.getValue().intValue() > len || outlen.getValue().intValue() <= 0) {
            throw new SMException(-25);
        }

        return Arrays.copyOfRange(plaintext, 0, outlen.getValue().intValue());

    }

    public static byte[] encrypt_cbc(byte[] plaintext, byte[] key, byte[] iv) throws SMException {

        if (key.length != 16) {
            throw new SMException(-27);
        }

        if (iv.length != 16) {
            throw new SMException(-28);
        }

        if (plaintext.length <= 0) {
            throw new SMException(-26);
        }

        int len = plaintext.length;
        byte[] ciphertext = new byte[(len / 16 + 1) * 16];
        SizeTByReference outlen = new SizeTByReference();

        SMAlgoBase.getSMInstance().SM4_CBC_Encrypt(plaintext, new SizeT(len), ciphertext, outlen, key, iv);
        if (outlen.getValue().intValue() != (len / 16 + 1) * 16) {
            throw new SMException(-25);
        }

        return ciphertext;
    }

    public static byte[] decrypt_cbc(byte[] ciphertext, byte[] key, byte[] iv) throws SMException {

        if (key.length != 16) {
            throw new SMException(-27);
        }

        if (iv.length != 16) {
            throw new SMException(-28);
        }

        if (ciphertext.length <= 0) {
            throw new SMException(-25);
        }

        int len = ciphertext.length;
        byte[] plaintext = new byte[len];
        SizeTByReference outlen = new SizeTByReference();

        SMAlgoBase.getSMInstance().SM4_CBC_Decrypt(ciphertext, new SizeT(len), plaintext, outlen, key, iv);

        // ���������ݳ���С�ڵ���0�Ļ��������쳣����
        if (outlen.getValue().intValue() > len || outlen.getValue().intValue() <= 0) {
            throw new SMException(-25);
        }

        return Arrays.copyOfRange(plaintext, 0, outlen.getValue().intValue());
    }
}

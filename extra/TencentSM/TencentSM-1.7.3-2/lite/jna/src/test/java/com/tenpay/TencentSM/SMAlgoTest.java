package com.tenpay.TencentSM;

import org.junit.Assert;
import org.junit.Test;

public class SMAlgoTest {

    public String byte2hex(byte[] buffer) {
        String ret = "";

        for (int i = 0; i < buffer.length; i++) {
            String temp = Integer.toHexString(buffer[i] & 0xFF);
            if (temp.length() == 1) {
                temp = "0" + temp;
            }
            ret = ret + " " + temp;
        }

        return ret;
    }

    @Test
    public void test_sm2_cipher_correctness() throws Exception {
        SM2Algo sm2algo = new SM2Algo();
        sm2algo.initCtx();

        String[] keyPairs;
        keyPairs = sm2algo.generateBase64KeyPair();

        String plaintext = "encryption standard";
        byte[] text = sm2algo.encrypt(plaintext.getBytes(), SMAlgoBase.base64ToBinary(keyPairs[1]));
        byte[] plain = sm2algo.decrypt(text, SMAlgoBase.base64ToBinary(keyPairs[0]));
        Assert.assertEquals(plaintext, new String(plain));
        sm2algo.freeCtx();
    }

    @Test
    public void test_sm2_sign() throws Exception {
        String data = "Faccount=";
        String publickey = "MDQ5ODRDMzQ4OUYyN0RDQkQ1MEE3ODQwQjAxMDgwMkM2RjdENTgzRDg5QjRBMUUwQzkwQjY0OUJCOTQ4MzA1QjREODE5NDZDNDY1NTExMUFBMjBGNDg1NDA5RUI2OTc0RjhCRTE3QjZCNUYzQTkyRUQwQUJFODcxMDFGNTVDMzEwNQ==";
        String privatKey = "ODIwMThEQUVBQTJERjJBNjAzRDYxNzVEMTE3Q0Q4MDlGNTg0QjQwNEQxMjY3MEZFQzVBMTlDNkQ4QUNBOEU3MQ==";

        String id = "trustsql.qq.com";

        SM2Algo sm2algo = new SM2Algo();
        sm2algo.initCtx();

        byte[] sign = sm2algo.sign(data.getBytes(), id.getBytes(), SMAlgoBase.base64ToBinary(publickey),
                SMAlgoBase.base64ToBinary(privatKey));
        Assert.assertNotNull(sign);
        sm2algo.freeCtx();
    }

    @Test
    public void test_sm2_encrypt_speed() throws Exception {
        SM2Algo sm2algo = new SM2Algo();
        sm2algo.initCtx();
        String[] keyPairs;
        keyPairs = sm2algo.generateBase64KeyPair();
        String plaintext = "encryption standard";

        int count = 10000;
        long startTime = System.currentTimeMillis();
        for (int i = 0; i < count; ++i) {
            sm2algo.encrypt(plaintext.getBytes(), SMAlgoBase.base64ToBinary(keyPairs[1]));
        }

        long endTime = System.currentTimeMillis();
        long interval = endTime - startTime;
        double fi = (double) interval / 1000.0;
        int speed = (int) (count / fi);
        System.out.println("test sm2 encrypt speed result:" + speed + "/s");
        sm2algo.freeCtx();
    }

    @Test
    public void test_sm2_decrypt_speed() throws Exception {
        SM2Algo sm2algo = new SM2Algo();
        sm2algo.initCtx();
        String[] keyPairs;
        keyPairs = sm2algo.generateBase64KeyPair();
        String plaintext = "encryption standard";
        byte[] text = sm2algo.encrypt(plaintext.getBytes(), SMAlgoBase.base64ToBinary(keyPairs[1]));

        int count = 10000;
        long startTime = System.currentTimeMillis();
        for (int i = 0; i < count; ++i) {
            sm2algo.decrypt(text, SMAlgoBase.base64ToBinary(keyPairs[0]));
        }

        long endTime = System.currentTimeMillis();
        long interval = endTime - startTime;
        double fi = (double) interval / 1000.0;
        int speed = (int) (count / fi);
        System.out.println("test sm2 decrypt speed result:" + speed + "/s");
        sm2algo.freeCtx();
    }

    @Test
    public void test_sm2_sign_speed() throws Exception {
        String data = "message digest";
        String pubkey = "MDQ5ODRDMzQ4OUYyN0RDQkQ1MEE3ODQwQjAxMDgwMkM2RjdENTgzRDg5QjRBMUUwQzkwQjY0OUJCOTQ4MzA1QjREODE5NDZDNDY1NTExMUFBMjBGNDg1NDA5RUI2OTc0RjhCRTE3QjZCNUYzQTkyRUQwQUJFODcxMDFGNTVDMzEwNQ==";
        String privatKey = "ODIwMThEQUVBQTJERjJBNjAzRDYxNzVEMTE3Q0Q4MDlGNTg0QjQwNEQxMjY3MEZFQzVBMTlDNkQ4QUNBOEU3MQ==";
        String id = "trustsql.qq.com";

        byte[] pukey = SMAlgoBase.base64ToBinary(pubkey);
        byte[] prkey = SMAlgoBase.base64ToBinary(privatKey);

        SM2Algo sm2algo = new SM2Algo();
        sm2algo.initCtx();
        int count = 10000;
        long startTime = System.currentTimeMillis();
        for (int i = 0; i < count; ++i) {
            sm2algo.sign(data.getBytes(), id.getBytes(), pukey, prkey);
        }

        long endTime = System.currentTimeMillis();
        long interval = endTime - startTime;
        double fi = (double) interval / 1000.0;
        int speed = (int) (count / fi);
        System.out.println("test sm2 sign speed result:" + speed + "/s");
        sm2algo.freeCtx();
    }

    @Test
    public void test_sm2_verify_speed() throws Exception {
        String data = "message digest";
        String pubkey = "MDQ5ODRDMzQ4OUYyN0RDQkQ1MEE3ODQwQjAxMDgwMkM2RjdENTgzRDg5QjRBMUUwQzkwQjY0OUJCOTQ4MzA1QjREODE5NDZDNDY1NTExMUFBMjBGNDg1NDA5RUI2OTc0RjhCRTE3QjZCNUYzQTkyRUQwQUJFODcxMDFGNTVDMzEwNQ==";
        String privatKey = "ODIwMThEQUVBQTJERjJBNjAzRDYxNzVEMTE3Q0Q4MDlGNTg0QjQwNEQxMjY3MEZFQzVBMTlDNkQ4QUNBOEU3MQ==";
        String id = "trustsql.qq.com";

        SM2Algo sm2algo = new SM2Algo();
        sm2algo.initCtx();

        byte[] pukey = SMAlgoBase.base64ToBinary(pubkey);
        byte[] prkey = SMAlgoBase.base64ToBinary(privatKey);
        byte[] sign = sm2algo.sign(data.getBytes(), id.getBytes(), pukey, prkey);

        int count = 10000;

        long startTime = System.currentTimeMillis();
        for (int i = 0; i < count; ++i) {
            sm2algo.verify(data.getBytes(), id.getBytes(), sign, SMAlgoBase.base64ToBinary(pubkey));
        }

        long endTime = System.currentTimeMillis();
        long interval = endTime - startTime;
        double fi = (double) interval / 1000.0;
        int speed = (int) (count / fi);
        System.out.println("test sm2 verify speed result:" + speed + "/s");
        sm2algo.freeCtx();
    }

    @Test
    public void test_sm3() {

        SM3Algo sm3 = new SM3Algo();
        sm3.update("abc".getBytes());
        byte[] hashvalue = sm3.digest();
        Assert.assertEquals(SM3Algo.SM3_DIGEST_LENGTH, hashvalue.length);
    }

    @Test
    public void test_sm3_32BYTE() {

        SM3Algo sm3 = new SM3Algo();
        sm3.update("abc_abc_abc_abc_abc_abc_abc_abc_".getBytes());
        byte[] hashvalue = sm3.digest();
        Assert.assertEquals(SM3Algo.SM3_DIGEST_LENGTH, hashvalue.length);
    }

    @Test
    public void test_sm4() throws Exception {
        String key = "1234567890123456";
        String plain = "abc";
        byte[] iv = new byte[]{61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61};

        byte[] ciphertext = SM4Algo.encrypt_cbc(plain.getBytes(), key.getBytes(), iv);
        Assert.assertNotNull(ciphertext);
        byte[] plaintext = SM4Algo.decrypt_cbc(ciphertext, key.getBytes(), iv);
        Assert.assertNotNull(plaintext);

        Assert.assertEquals(plain, new String(plaintext));
    }

    @Test
    public void test_sm4_padding() throws Exception {
        String key = "1234567890123456";
        String plain = "abcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabcabc";
        byte[] iv = new byte[]{61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61};

        byte[] ciphertext = SM4Algo.encrypt_cbc(plain.getBytes(), key.getBytes(), iv);
        System.out.println(byte2hex(ciphertext));

        byte[] plaintext = SM4Algo.decrypt_cbc(ciphertext, key.getBytes(), iv);
        System.out.println(byte2hex(plaintext));
        System.out.println(new String(plaintext));
        System.out.println(new String(plaintext).length());

        Assert.assertEquals(plain, new String(plaintext));
    }

    @Test
    public void test_sm4_padding_ebc() throws Exception {
        String keybase64 = "9pK32icafiBvg4QvSxuPCg==";
        String plainbase64 = "7gvKQQt7iREpxlwUuu01VqxY88PooUV3DNO/QZpbx1I=";

        byte[] key11 = SMAlgoBase.base64ToBinary(keybase64);
        byte[] plain11 = SMAlgoBase.base64ToBinary(plainbase64);

        byte[] ciphertext = SM4Algo.encrypt_ecb(plain11, key11);
        System.out.println(byte2hex(ciphertext));

    }

    @Test
    public void test_sm2_sign_loop() throws Exception {
        for (int i = 0; i < 1000; ++i) {
            test_sm2_sign();
        }

    }

    @Test
    public void test_sm4_padding_16Byte() throws Exception {
        String key = "1234567890123456";
        String plain = "AAA_AAA_AAA_AAA_";
        byte[] iv = new byte[]{61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61, 61};

        byte[] ciphertext = SM4Algo.encrypt_cbc(plain.getBytes(), key.getBytes(), iv);
        System.out.println(byte2hex(ciphertext));

        byte[] plaintext = SM4Algo.decrypt_cbc(ciphertext, key.getBytes(), iv);
        System.out.println(byte2hex(plaintext));
        System.out.println(new String(plaintext));
        System.out.println(new String(plaintext).length());

        Assert.assertEquals(plain, new String(plaintext));
    }

}

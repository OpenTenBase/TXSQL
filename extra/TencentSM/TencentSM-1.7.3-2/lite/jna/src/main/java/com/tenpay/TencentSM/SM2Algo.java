package com.tenpay.TencentSM;


import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import java.util.Arrays;


public class SM2Algo extends SMAlgoBase {

    public static final int SM2CipherMode_C1C3C2_ASN1 = 0;
    public static final int SM2CipherMode_C1C3C2 = 1;
    public static final int SM2CipherMode_C1C2C3_ASN1 = 2;
    public static final int SM2CipherMode_C1C2C3 = 3;

    public static final int SM2_PRIVATE_KEY_LENGTH = 66;
    public static final int SM2_PUBLICK_KEY_LENGTH = 132;
    public static final int SM2_SIGN_LENGTH = 164;

    private Pointer ctx;

    public byte[] append0(byte[] b) {
        b = Arrays.copyOf(b, b.length + 1);
        b[b.length - 1] = '\0';
        return b;
    }

    public void initCtx() {
        int len = SMAlgoBase.getSMInstance().SM2CtxSize();
        this.ctx = new Memory(len);
        SMAlgoBase.getSMInstance().SM2InitCtx(this.ctx);

        //System.out.println("----"+this.ctx+"----"+this.ctx.getPointer(0));
    }

    public void freeCtx() {
        SMAlgoBase.getSMInstance().SM2FreeCtx(this.ctx);
    }

    public String[] generateBase64KeyPair() throws Exception {
        byte[] prikey = new byte[SM2_PRIVATE_KEY_LENGTH];
        byte[] pubkey = new byte[SM2_PUBLICK_KEY_LENGTH];
        int ret = SMAlgoBase.getSMInstance().generateKeyPair(this.ctx, prikey, pubkey);
        if (ret == 0) {
            return new String[]{binaryToBase64(prikey), binaryToBase64(pubkey)};
        }

        throw new SMException(ret);
    }

    public byte[] encrypt(byte[] plaintext, byte[] pubkey) throws Exception {
        // �ĵ�˵���Ǽ��ܺ󳤶Ȼ��100�ֽ� ������û�о�ȷ˵���٣������ȼ�100�ֽڣ�������������

        byte[] ciphertext = new byte[plaintext.length + 200];
        SizeTByReference len = new SizeTByReference(new SizeT(plaintext.length + 200));
        pubkey = append0(pubkey);

        int ret = SMAlgoBase.getSMInstance()
                .SM2Encrypt(this.ctx, plaintext, new SizeT(plaintext.length), pubkey, new SizeT(pubkey.length),
                        ciphertext, len);
        if (ret == 0) {
            byte[] buf = new byte[len.getValue().intValue()];
            System.arraycopy(ciphertext, 0, buf, 0, len.getValue().intValue());
            return buf;
        }

        System.out.println(ret);

        throw new SMException(ret);
    }

    public byte[] decrypt(byte[] ciphertext, byte[] prikey) throws Exception {
        byte[] plaintext = new byte[ciphertext.length];
        SizeTByReference len = new SizeTByReference(new SizeT(ciphertext.length));
        prikey = append0(prikey);

        int ret = SMAlgoBase.getSMInstance()
                .SM2Decrypt(this.ctx, ciphertext, new SizeT(ciphertext.length), prikey, new SizeT(prikey.length),
                        plaintext, len);
        if (ret == 0) {
            byte[] buf = new byte[len.getValue().intValue()];
            System.arraycopy(plaintext, 0, buf, 0, len.getValue().intValue());
            return buf;
        }
        throw new SMException(ret);
    }

    public byte[] decryptWithMode(byte[] ciphertext, byte[] prikey, int mode) throws Exception {
        byte[] plaintext = new byte[ciphertext.length];
        SizeTByReference len = new SizeTByReference(new SizeT(ciphertext.length));
        prikey = append0(prikey);

        int ret = SMAlgoBase.getSMInstance()
                .SM2DecryptWithMode(this.ctx, ciphertext, new SizeT(ciphertext.length), prikey,
                        new SizeT(prikey.length), plaintext, len, mode);
        if (ret == 0) {
            byte[] buf = new byte[len.getValue().intValue()];
            System.arraycopy(plaintext, 0, buf, 0, len.getValue().intValue());
            return buf;
        }
        throw new SMException(ret);
    }

    // pubkey�Ľ�β��Ҫһ����������
    public byte[] sign(byte[] msg, byte[] id, byte[] pubkey, byte[] prikey) throws Exception {

        byte[] sig = new byte[SM2_SIGN_LENGTH];
        SizeTByReference siglen = new SizeTByReference(new SizeT(SM2_SIGN_LENGTH));
        pubkey = append0(pubkey);
        prikey = append0(prikey);

        int ret = SMAlgoBase.getSMInstance()
                .SM2Sign(this.ctx, msg, new SizeT(msg.length), id, new SizeT(id.length), pubkey,
                        new SizeT(pubkey.length), prikey, new SizeT(prikey.length), sig, siglen);

        if (ret == 0) {
            int len = siglen.getValue().intValue();
            if (len > SM2_SIGN_LENGTH) {
                throw new Exception("sign failed, result too long.");
            }

            return Arrays.copyOfRange(sig, 0, len);
        }

        throw new SMException(ret);
    }


    public boolean verify(byte[] msg, byte[] id, byte[] sig, byte[] pubkey) throws Exception {
        pubkey = append0(pubkey);
        int ret = SMAlgoBase.getSMInstance()
                .SM2Verify(this.ctx, msg, new SizeT(msg.length), id, new SizeT(id.length), sig, new SizeT(sig.length),
                        pubkey, new SizeT(pubkey.length));
        if (ret == 0) {
            return true;
        }

        throw new SMException(ret);
    }
}

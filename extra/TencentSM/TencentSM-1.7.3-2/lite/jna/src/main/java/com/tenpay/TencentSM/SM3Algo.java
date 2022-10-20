package com.tenpay.TencentSM;

import com.sun.jna.Memory;
import com.sun.jna.Pointer;

public class SM3Algo {

    public static int SM3_DIGEST_LENGTH = 32;

    private Pointer ctx;

    public SM3Algo() {
        int len = SMAlgoBase.getSMInstance().SM3CtxSize();
        this.ctx = new Memory(len);
        SMAlgoBase.getSMInstance().SM3Init(this.ctx);
    }

    public void update(byte[] data) {
        SMAlgoBase.getSMInstance().SM3Update(this.ctx, data, new SizeT(data.length));
    }

    public byte[] digest() {
        byte[] digest = new byte[SM3_DIGEST_LENGTH];
        SMAlgoBase.getSMInstance().SM3Final(this.ctx, digest);
        return digest;
    }

}

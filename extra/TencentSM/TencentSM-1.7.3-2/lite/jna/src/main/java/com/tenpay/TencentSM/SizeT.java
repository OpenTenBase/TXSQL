package com.tenpay.TencentSM;

import com.sun.jna.IntegerType;
import com.sun.jna.Native;

@SuppressWarnings("serial")
public class SizeT extends IntegerType {

    public SizeT() {
        this(0);
    }

    public SizeT(long value) {
        super(Native.SIZE_T_SIZE, value, true);
    }
}
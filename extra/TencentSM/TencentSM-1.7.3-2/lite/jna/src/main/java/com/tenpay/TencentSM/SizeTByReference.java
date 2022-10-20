package com.tenpay.TencentSM;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.ByReference;

public class SizeTByReference extends ByReference {

    public SizeTByReference() {
        this(new SizeT());
    }

    public SizeTByReference(SizeT value) {
        super(Native.SIZE_T_SIZE);
        setValue(value);
    }

    public SizeT getValue() {
        Pointer p = getPointer();
        return new SizeT(Native.SIZE_T_SIZE == 8 ? p.getLong(0) : p.getInt(0));
    }

    public void setValue(SizeT value) {
        Pointer p = getPointer();
        if (Native.SIZE_T_SIZE == 8) {
            p.setLong(0, value.longValue());
        } else {
            p.setInt(0, value.intValue());
        }
    }
}

package com.tenpay.TencentSM;

import com.sun.jna.Native;
import org.apache.commons.codec.binary.Base64;

class SMAlgoBase {
    public static SMInterface instance = null;

    public static final String LIB_VERSION = "1.7.1";

    public static SMInterface getSMInstance() {
        if (instance != null) {
            return instance;
        }

        try {
            instance = (SMInterface) Native.load("TencentSM-" + LIB_VERSION, SMInterface.class);
        } catch (Exception e) {
            e.printStackTrace();
        }

        return instance;
    }

    public static byte[] base64ToBinary(String base64) throws Exception {
        return Base64.decodeBase64(base64);
    }

    public static String binaryToBase64(byte[] bin) throws Exception {
        return Base64.encodeBase64String(bin);
    }

}

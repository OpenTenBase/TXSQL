package com.tenpay.TencentSM;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class SMException extends Exception {


    private static final long serialVersionUID = 14552145448L;

    private static final Map<Integer, String> errMap;

    static {
        Map<Integer, String> map = new HashMap<Integer, String>();
        map.put(-1, "sm2 error: input params error!");
        map.put(-2, "sm2 error: new key failed!");
        map.put(-3, "sm2 error: get private key failed");
        map.put(-4, "sm2 error: bn2hex failed");
        map.put(-5, "sm2 error: new new_ec_group_sm2p256v1 failed");
        map.put(-6, "sm2 error: calucate public key failed(EC_POINT_mul calc failed).");
        map.put(-7, "sm2 error: get public key failed(EC_KEY_get0_public_key failed).");
        map.put(-8, "sm2 error: point to hex failed(EC_POINT_point2hex failed).");
        map.put(-9, "sm2 error: convert string to public struct failed.");
        map.put(-10, "sm2 error: convert string to private struct failed.");
        map.put(-11, "sm2 error: encrypt failed.");
        map.put(-12, "sm2 error: decrypt failed.");
        map.put(-13, "sm2 error: compute digest failed.");
        map.put(-15, "sm2 error: sign failed.");
        map.put(-16, "sm2 error: verify failed.");
        map.put(-17, "sm2 error: init not called.");
        map.put(-18, "sm2 error: decode x9.62 public key failed.");
        map.put(-19, "sm2 error: wrong public key oid.");
        map.put(-20, "sm2 error: wrong ECGroup oid.");
        map.put(-21, "sm2 error: decode pkcs#8 private key failed.");
        map.put(-22, "sm2 error: wrong private key oid.");
        map.put(-23, "sm2 error: generate public key failed.");

        map.put(-25, "sm4 error: sm4 encrypt failed. wrong cipher len.");
        map.put(-26, "sm4 error: sm4 decrypt failed. wrong plain text len.");
        map.put(-27, "sm4 error: sm4 decrypt failed. wrong key len.");
        map.put(-28, "sm4 error: sm4 decrypt failed. wrong iv len.");

        errMap = Collections.unmodifiableMap(map);
    }


    SMException(int error) {
        super(errMap.get(error));
    }

}

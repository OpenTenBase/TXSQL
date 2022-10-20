package com.tenpay.utils;


/**
 * android平台使用国密动态链接库的java接口文件
 * @author jiangweiti
 */
public class SMUtils {
    public static final int SMS4_IV_LENGTH = 16;
    public static final String LIB_NAME = "libTencentSM";

    //load so
    private static boolean isLoadSuccess = false;
    static {
        try {
            System.loadLibrary(LIB_NAME);
            isLoadSuccess = true;
        } catch (Exception e) {
            isLoadSuccess = false;
        }
    }

    //singleton
    private static SMUtils mInstance = null;
    private SMUtils() {
    }
    public static SMUtils getInstance() {
        if(mInstance == null) {
            synchronized (SMUtils.class) {
                if(mInstance == null) {
                    mInstance = new SMUtils();
                }
            }
        }
        return mInstance;
    }

    /**
     *  是否库文件加载成功
     */
    public static boolean isLoadOK() {
        return isLoadSuccess;
    }

    /**
     * 获取SMLib版本
     * @return
     */
    public native String version();
    /***********************************************SM2 ******************* */
    public native long    SM2InitCtx();
    public native long    SM2InitCtxWithPubKey(String strPubKey);
    public native void     SM2FreeCtx(long sm2Handler);
    /**
     * @return array[0] privatekey str
     *         array[1] publickey str
     */
    public native Object[]   SM2GenKeyPair(long sm2Handler);
    public native byte[]     SM2Encrypt(long sm2Handler, byte[] in, String strPubKey);
    public native byte[]     SM2Decrypt(long sm2Handler, byte[] in, String strPriKey);
    public native byte[]     SM2Sign(long sm2Handler, byte[] msg, byte[] id, String strPubKey, String strPriKey);
    public native int     SM2Verify(long sm2Handler, byte[] msg, byte[] id, String strPubKey, byte[] sig);
    public native int     SM2ReSeed(long sm2Handler, byte[] buf);
    /***********************************************SM3 ******************* */
    public native long    SM3Init();
    public native void    SM3Update(long sm3Handler, byte[] data);
    public native byte[]  SM3Final(long sm3Handler);
    public native void    SM3Free(long sm3Handler);
    public native byte[]     SM3(byte[] data);
    /***********************************************SM4 ******************* */
    public native byte[]     SM4GenKey();
    public native byte[]     SM4CBCEncrypt(byte[] in, byte[] key, byte[] iv);
    public native byte[]     SM4CBCDecrypt(byte[] in, byte[] key, byte[] iv);
    public native byte[]     SM4CBCEncryptNoPadding(byte[] in, byte[] key, byte[] iv);
    public native byte[]     SM4CBCDecryptNoPadding(byte[] in, byte[] key, byte[] iv);
    public native byte[]     SM4ECBEncrypt(byte[] in, byte[] key);
    public native byte[]     SM4ECBDecrypt(byte[] in, byte[] key);
    public native byte[]     SM4ECBEncryptNoPadding(byte[] in, byte[] key);
    public native byte[]     SM4ECBDecryptNoPadding(byte[] in, byte[] key);
}

package com.tenpay.utils;

/**
 * android平台使用国密动态链接库KeyMannager的java接口文件
 * @author jiangweiti
 */
public class SMKeyManUtils {
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
    private static SMKeyManUtils mInstance = null;
    private SMKeyManUtils() {
    }
    public static SMKeyManUtils getInstance() {
        if(mInstance == null) {
            synchronized (SMKeyManUtils.class) {
                if(mInstance == null) {
                    mInstance = new SMKeyManUtils();
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

    /***************************native fun***************************************/
    public native int     checkKmsFile(Object[] factors, String dir_path);
    public native int     removeKmsFile(Object[] factors, String dir_path);

    public native int     SM4KeyGenWriteKms(Object[] factors, String dir_path, String description,int force_update);
    public native int     SM2KeyPairGenWriteKms(Object[] factors, String dir_path, String description,int force_update);

    /**
     * @return <0 失败   >=0 数量值
     */
    public native int     allKeyDescriptionCount(Object[] factors, String dir_path);
    /**
     * @return null 失败   其它：descriptions string数组
     */
    public native Object[]     allKeyDescription(Object[] factors, String dir_path);

    /**
     * @return <0 失败   >=0 数量值
     */
    public native int     allKeyPairDescriptionCount(Object[] factors, String dir_path);
    /**
     * @return null 失败   其它：descriptions string数组
     */
    public native Object[]     allKeyPairDescription(Object[] factors, String dir_path);

    /**
     * @return null 失败   其它：string数组--[0]char *prikey, [1]char *pubkey,
     */
    public native Object[]     keyPairWithDescription(Object[] factors, String dir_path, String description);
    /**
     * @return null 失败   其它：string--char* key
     */
    public native String     keyWithDescription(Object[] factors, String dir_path, String description);

    public native int     importKeyWithDescription(Object[] factors, String dir_path, String description, String key, int force_update);
    public native int     importKeyPairWithDescription(Object[] factors, String dir_path, String description, String pubkey, String prikey, int force_update);

    public native int     delDescription(Object[] factors, String dir_path, String description);

}

package com.tenpay.utils;

import android.content.Context;
import android.text.TextUtils;

import java.util.ArrayList;
import java.util.Arrays;

/**
 * tencentSM keymanager
 * @author jiangweiti
 */
public class SMKeyManager {

    private final Context mContext;
    private final SMKeyManUtils mSMKeyManUtils;
    private String mKeyPath;
    private Object[] mFactorArray;

    public static SMKeyManager mInstance = null;
    public static SMKeyManager getInstance(Context context, String keypath) {
        if(mInstance == null) {
            synchronized (SMKeyManager.class) {
                if(mInstance == null) {
                    mInstance = new SMKeyManager(context, keypath);
                }
            }
        }
        return mInstance;
    }
    public static  synchronized void destroyInstance() {
        mInstance = null;
    }


    /**
     *
     * @param context  上下文
     * @param keypath  路径可以为null，需绝对路径
     */
    private SMKeyManager(Context context, String keypath) {
        mSMKeyManUtils = SMKeyManUtils.getInstance(context);
        mContext = context.getApplicationContext();
        if(TextUtils.isEmpty(keypath)) {
            mKeyPath = context.getFilesDir().getAbsolutePath();
        } else {
            mKeyPath = keypath;
        }
    }

    /**
     如首次使用KMS模块，需先调用该接口以Key空间，首次调用传入合法参数后即可以使用。
     针对传入不同参数的情形，可多次调用，不同参数将会初始化不同的密钥管理空间，如业务
     App需要针对不同用户使用不同的密钥存储空间，可以参数不同来进行分割密钥管理空间。
     @idHash - 用户ID散列字符串
     @pinHash - 用户密码散列字符串
     @userSalt - 可使用秘盐
     */
    public boolean initWithAuthority(String idHash, String pinHash, String userSalt) {
        ArrayList<String> list = new ArrayList<>();
        if(!TextUtils.isEmpty(idHash)) {
            list.add(idHash);
        }
        if(!TextUtils.isEmpty(pinHash)) {
            list.add(pinHash);
        }
        if(!TextUtils.isEmpty(userSalt)) {
            list.add(userSalt);
        }
        list.add(Utils.getDeviceUUID(mContext));
        Object[] factors = list.toArray();
        //tc_check_kms_file
        int result = mSMKeyManUtils.checkKmsFile(factors, mKeyPath);
        if(result == 0) {
            mFactorArray = factors;
            return true;
        }
        return false;
    }

    /**
     * 删除授权对应key空间，该操作将会删除该授权下的所有密钥记录，谨慎操作。
     @idHash - 用户ID散列字符串
     @pinHash - 用户密码散列字符串
     @userSalt - 可使用秘盐
     */
    public boolean delWithAuthority(String idHash, String pinHash, String userSalt) {
        mFactorArray = null;
        ArrayList<String> list = new ArrayList<>();
        if(!TextUtils.isEmpty(idHash)) {
            list.add(idHash);
        }
        if(!TextUtils.isEmpty(pinHash)) {
            list.add(pinHash);
        }
        if(!TextUtils.isEmpty(userSalt)) {
            list.add(userSalt);
        }
        list.add(Utils.getDeviceUUID(mContext));
        Object[] factors = list.toArray();
        //tc_check_kms_file
        int result = mSMKeyManUtils.removeKmsFile(factors, mKeyPath);
        if(result == 0) {
            return true;
        }
        return false;
    }

    /**
     * 生成非对称加密的key
     */
    public boolean genAsymSM2Key(String desc, boolean forceupdate) {
        if(mFactorArray == null) {
            return false;
        }
        int result = mSMKeyManUtils.SM2KeyPairGenWriteKms(mFactorArray, mKeyPath, desc, forceupdate?1:0);
        if(result == 0) {
            return true;
        }
        return false;
    }

    /**
     * 生成SM3对称秘钥
     */
    public boolean genSymSM4Key(String desc, boolean forceupdate) {
        if(mFactorArray == null) {
            return false;
        }
        int result = mSMKeyManUtils.SM4KeyGenWriteKms(mFactorArray, mKeyPath, desc, forceupdate?1:0);
        if(result == 0) {
            return true;
        }
        return false;
    }

    /**
     * 获取非对称加密秘钥所有的描述字符串
     */
    public String[] getAllAsymKeyDesc() {
        if(mFactorArray == null) {
            return null;
        }
        Object[] result = mSMKeyManUtils.allKeyPairDescription(mFactorArray, mKeyPath);
        if(result != null) {
            return Arrays.copyOf(result, result.length, String[].class);
        }
        return null;
    }

    /**
     * 获取对称加密秘钥所有的描述字符串
     */
    public String[] getAllSymKeyDesc() {
        if(mFactorArray == null) {
            return null;
        }
        Object[] result = mSMKeyManUtils.allKeyDescription(mFactorArray, mKeyPath);
        if(result != null) {
            return Arrays.copyOf(result, result.length, String[].class);
        }
        return null;
    }

    /**
     * 获取非对称加密秘钥的公钥
     */
    public String getAsymPubKey(String desc) {
        if(mFactorArray == null) {
            return null;
        }
        Object[] result = mSMKeyManUtils.keyPairWithDescription(mFactorArray, mKeyPath, desc);
        if(result != null) {
            String[] keys =  Arrays.copyOf(result, result.length, String[].class);
            if(!TextUtils.isEmpty(keys[1])) {
                return keys[1];
            }
        }
        return null;
    }
    /**
     * 获取非对称加密秘钥的私钥
     */
    public String getAsymPriKey(String desc) {
        if(mFactorArray == null) {
            return null;
        }
        Object[] result = mSMKeyManUtils.keyPairWithDescription(mFactorArray, mKeyPath, desc);
        if(result != null) {
            String[] keys =  Arrays.copyOf(result, result.length, String[].class);
            if(!TextUtils.isEmpty(keys[0])) {
                return keys[0];
            }
        }
        return null;
    }
    /**
     * 获取非对称加密秘钥的私钥和公钥
     * @return  返回【0】私钥 【1】公钥
     */
    public String[] getAsymPriKeypair(String desc) {
        if(mFactorArray == null) {
            return null;
        }
        Object[] result = mSMKeyManUtils.keyPairWithDescription(mFactorArray, mKeyPath, desc);
        if(result != null) {
            String[] keys =  Arrays.copyOf(result, result.length, String[].class);
            return keys;
        }
        return null;
    }
    /**
     * 获取对称加密秘钥
     */
    public String getSymKey(String desc) {
        if(mFactorArray == null) {
            return null;
        }
        String result = mSMKeyManUtils.keyWithDescription(mFactorArray, mKeyPath, desc);
        if(result != null) {
            if(!TextUtils.isEmpty(result)) {
                return result;
            }
        }
        return null;
    }

    /**
     * 保存对应描述的非对称加密秘钥对
     */
    public boolean saveAsymKeypair(String desc, String pubKey, String priKey, boolean forceupdate) {
        if(mFactorArray == null) {
            return false;
        }
        int result = mSMKeyManUtils.importKeyPairWithDescription(mFactorArray, mKeyPath, desc, pubKey, priKey, forceupdate?1:0);
        if(result == 0) {
            return true;
        }
        return false;
    }
    /**
     * 保存对应描述的对称加密秘钥
     */
    public boolean saveSymKey(String desc, String key, boolean forceupdate) {
        if(mFactorArray == null) {
            return false;
        }
        int result = mSMKeyManUtils.importKeyWithDescription(mFactorArray, mKeyPath, desc, key, forceupdate?1:0);
        if(result == 0) {
            return true;
        }
        return false;
    }
    /**
     * 删除对应描述的秘钥数据
     */
    public boolean delDataWithDesc(String desc) {
        if(mFactorArray == null) {
            return false;
        }
        int result = mSMKeyManUtils.delDescription(mFactorArray, mKeyPath, desc);
        if(result == 0) {
            return true;
        }
        return false;
    }
}

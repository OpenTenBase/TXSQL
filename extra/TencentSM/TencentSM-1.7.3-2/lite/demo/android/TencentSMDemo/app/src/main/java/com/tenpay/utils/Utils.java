package com.tenpay.utils;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.text.TextUtils;
import android.util.TypedValue;

import java.security.MessageDigest;
import java.util.Random;
import java.util.UUID;

public class Utils {
    public static final String SHAREDPREF_NAME = "sharedpref_name";
    public static final String SHAREDPREF_UUID = "sharedpref_uuid";
    public static String DEVICE_UUID = null;
    public static String getDeviceUUID(Context context) {
        if(DEVICE_UUID != null) {
            return DEVICE_UUID;
        }
        //get UUID from SharedPreference
        SharedPreferences sp = context.getSharedPreferences(SHAREDPREF_NAME, Context.MODE_PRIVATE);
        String tmpStr = sp.getString(SHAREDPREF_UUID, null);
        if(tmpStr != null) {
            return tmpStr;
        }
        //create UUID
        tmpStr = Settings.Secure.getString(context.getContentResolver(), Settings.Secure.ANDROID_ID);
        UUID uuid = null;
        if(TextUtils.isEmpty(tmpStr)) {
            uuid = UUID.randomUUID();
        } else {
            uuid = new UUID(tmpStr.hashCode(), (long)tmpStr.hashCode() << 32);
        }
        String deviceuuid = uuid.toString();
        sp.edit().putString(SHAREDPREF_NAME, deviceuuid).commit();
        return deviceuuid;
    }

    public static String md5(String string) {
        if (TextUtils.isEmpty(string)) {
            return "";
        }
        MessageDigest md5 = null;
        try {
            md5 = MessageDigest.getInstance("MD5");
            byte[] bytes = md5.digest(string.getBytes());
            StringBuffer result = new StringBuffer();
            for (byte b : bytes) {
                String temp = Integer.toHexString(b & 0xff);
                if (temp.length() == 1) {
                    result.append('0');
                }
                result.append(temp);
            }
            return result.toString();
        } catch (Exception e) {
            e.printStackTrace();
        }
        return "";
    }


    private static String[] PERMISSIONS_STORAGE = {
            "android.permission.READ_EXTERNAL_STORAGE",
            "android.permission.WRITE_EXTERNAL_STORAGE" };


    public static void verifyStoragePermissions(Activity activity, int requestcode) {

        try {
            //检测是否有写的权限
            int permission = ActivityCompat.checkSelfPermission(activity,
                    "android.permission.WRITE_EXTERNAL_STORAGE");
            if (permission != PackageManager.PERMISSION_GRANTED) {
                // 没有写的权限，去申请写的权限，会弹出对话框
                ActivityCompat.requestPermissions(activity, PERMISSIONS_STORAGE, requestcode);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }


    /**
     * byte to string  按16进制格式
     */
    public static String byte2String(byte[] src) {
        StringBuffer sb = new StringBuffer();
        int low,hight;
        for(byte item : src) {
            low = item&0x0f;
            hight = (item&0xf0)>>4;
            if(hight >= 10) {
                sb.append((char)((hight-10)+'a'));
            } else {
                sb.append((char)((hight)+'0'));
            }
            if(low >= 10) {
                sb.append((char)((low-10)+'a'));
            } else {
                sb.append((char)((low)+'0'));
            }
        }
        return sb.toString();
    }
    public static byte[] string2bytes(String src) {
        int len = (src.length()+1)/2;
        byte[] bytes = new byte[len];
        int pos = 0;
        for(int i = 0; i < len; i++) {
            bytes[i] = 0;
            //hight
            if(pos < src.length()) {
                bytes[i] = (byte) (char2bytewithasc2(src.charAt(pos)) << 4);
            } else {
                break;
            }
            pos++;
            //low
            if(pos < src.length()) {
                bytes[i] = (byte) ((char2bytewithasc2(src.charAt(pos)))|(bytes[i]));
            } else {
                break;
            }
            pos++;
        }
        return bytes;
    }

    public static byte char2bytewithasc2(char c) {
        byte result;
        if(c >= '0'&& c <='9') {
            result = (byte) (c-'0');
        } else if(c >= 'A' && c <= 'F') {
            result = (byte) (c-'A'+10);
        } else if(c >= 'a' && c <= 'f') {
            result = (byte) (c-'a'+10);
        } else {
            result = 0;
        }
        return result;
    }


    /**
     * 随机字节码
     */
    public static byte[] getRandombytes(int len) {
        byte[] results = new byte[len];
        Random random = new Random();
        random.setSeed(System.currentTimeMillis());
        random.nextBytes(results);

        return results;
    }

    /**
     * dp to px
     */
    public static int dp2px(Context context, float size) {
        return (int) TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, size, context.getResources().getDisplayMetrics());
    }

}

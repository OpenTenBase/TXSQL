package com.tenpay.utils;

import java.security.MessageDigest;
import java.util.Random;

public class Utils {
    public static final String SHAREDPREF_NAME = "sharedpref_name";
    public static final String SHAREDPREF_UUID = "sharedpref_uuid";
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


    /**
     * byte to string  鎸�16杩涘埗鏍煎紡
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
     * 闅忔満瀛楄妭鐮�
     */
    public static byte[] getRandombytes(int len) {
        byte[] results = new byte[len];
        Random random = new Random();
        random.setSeed(System.currentTimeMillis());
        random.nextBytes(results);

        return results;
    }

}

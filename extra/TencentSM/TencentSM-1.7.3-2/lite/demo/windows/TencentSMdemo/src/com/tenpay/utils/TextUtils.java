package com.tenpay.utils;

public class TextUtils {
	public static final boolean isEmpty(String str) {
		if(str == null || str.length() == 0) {
			return true;
		}
		return false;
	}
}

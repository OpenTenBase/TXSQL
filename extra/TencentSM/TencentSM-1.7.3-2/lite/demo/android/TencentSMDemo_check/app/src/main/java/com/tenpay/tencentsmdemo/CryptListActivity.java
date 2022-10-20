package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.Toast;

import com.tenpay.utils.SMUtils;
import com.tenpay.utils.Utils;

public class CryptListActivity extends BaseToolbarActivity implements AdapterView.OnItemClickListener {
    public static final int[] LIST_STRS_RES = {R.string.sm2, R.string.en_decrypt, R.string.sign_check, R.string.sm3, R.string.gen_hash,
            R.string.sm4, R.string.cbc_en_decrypt, R.string.ecb_en_decrypt, R.string.gcm_en_decrypt};
    private ArrayAdapter mAdapter;

    public static Intent getStartIntent(Context context, String backtxt, String id, String pincode) {
        Intent intent = new Intent(context, CryptListActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        intent.putExtra(STRID_IDENTIRY, id);
        intent.putExtra(STRID_PINCODE, pincode);
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);
        setTitle(R.string.module_crypt);

        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, android.R.layout.simple_expandable_list_item_1);
        CharSequence[] arrayStrs = new String[LIST_STRS_RES.length];
        for(int i = 0; i < LIST_STRS_RES.length; i++) {
            arrayStrs[i] = getText(LIST_STRS_RES[i]);
        }
        mAdapter.addAll(arrayStrs);
        listview.setAdapter(mAdapter);
        listview.setOnItemClickListener(this);
    }


    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        switch (position) {
            case 1://加密解密
                startActivity(SelectDescActivity.getStartIntent(this, getString(R.string.module_crypt), SelectDescActivity.TYPE_SM2_ENCRYPT));
                break;
            case 2://签名验签
                startActivity(SelectDescActivity.getStartIntent(this, getString(R.string.module_crypt), SelectDescActivity.TYPE_SM2_SIGN));
                break;
            case 4://生成Hash
                startActivity(SM3HashActivity.getStartIntent(this, getString(R.string.module_crypt)));
                break;
            case 6://CBC加解密
                startActivity(SelectDescActivity.getStartIntent(this, getString(R.string.module_crypt), SelectDescActivity.TYPE_SM4_CBC));
                break;
            case 7://ECB加解密
                startActivity(SelectDescActivity.getStartIntent(this, getString(R.string.module_crypt), SelectDescActivity.TYPE_SM4_ECB));
                break;
            case 8://GCM加解密
                testSM4GCM();
                break;
        }
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
    }

    //------------------gcm encrypt&decrypt test------
    public static final String SM4_CRYPT_PLAIN = "123456qwrqrew111111111111111";
    void testSM4GCM() {
        byte[] plains = SM4_CRYPT_PLAIN.getBytes();
        byte[] key = null;
        byte[] iv = {(byte)0x26,(byte)0x77,(byte)0xF4,(byte)0x6B,(byte)0x09,(byte)0xC1,(byte)0x22,
                (byte)0xCC,(byte)0x97,(byte)0x55,(byte)0x33,(byte)0x10};
        byte[] aad = {(byte)0x26,(byte)0x77,(byte)0xF4,(byte)0x6B,(byte)0x09,(byte)0xC1,(byte)0x22,
                (byte)0xCC,(byte)0x97,(byte)0x55,(byte)0x33,(byte)0x10,(byte)0x5B,(byte)0xD4,
                (byte)0xA2,(byte)0x2A};
        SMUtils smUtils = SMUtils.getInstance(this);
        key = smUtils.SM4GenKey();
        if (key == null) {
            Toast.makeText(this, "SM4GenKey err!", Toast.LENGTH_SHORT).show();
            return;
        }
        byte[][] outdata = new byte[2][];

        int iret = smUtils.SM4GCMEncrypt(plains, key, iv, aad, outdata);
        if (iret != 0) {
            Toast.makeText(this, "SM4GCMEncrypt err!", Toast.LENGTH_SHORT).show();
            return;
        }
        Log.v("JNITag", "---java-- encrypt cipher=" + Utils.byte2String(outdata[0]));
        Log.v("JNITag", "---java-- encrypt tag=" + Utils.byte2String(outdata[1]));
        byte[][] outdata2 = new byte[1][];
        iret = smUtils.SM4GCMDecrypt(outdata[0], key, iv, aad, outdata[1], outdata2);
        if (iret != 0) {
            Toast.makeText(this, "SM4GCMDecrypt err!", Toast.LENGTH_SHORT).show();
            return;
        }
        String outplain = new String(outdata2[0]);
        if(!SM4_CRYPT_PLAIN.equals(outplain)) {
            Toast.makeText(this, "SM4 encrypt&decrypt err!", Toast.LENGTH_SHORT).show();
            return;
        } else {
            Toast.makeText(this, "SM4 encrypt&decrypt successful!", Toast.LENGTH_SHORT).show();
        }
        //-----------------nopadding--
        outdata = new byte[2][];
        iret = smUtils.SM4GCMEncryptNoPadding(plains, key, iv, aad, outdata);
        if (iret != 0) {
            Toast.makeText(this, "SM4GCMEncryptNoPadding err!", Toast.LENGTH_SHORT).show();
            return;
        }
        Log.v("JNITag", "---java-- SM4GCMEncryptNoPadding cipher=" + Utils.byte2String(outdata[0]));
        Log.v("JNITag", "---java-- SM4GCMEncryptNoPadding tag=" + Utils.byte2String(outdata[1]));
        outdata2 = new byte[1][];
        iret = smUtils.SM4GCMDecryptNoPadding(outdata[0], key, iv, aad, outdata[1], outdata2);
        if (iret != 0) {
            Toast.makeText(this, "SM4GCMDecryptNoPadding err!", Toast.LENGTH_SHORT).show();
            return;
        }
        outplain = new String(outdata2[0]);
        if(!SM4_CRYPT_PLAIN.equals(outplain)) {
            Toast.makeText(this, "SM4  nopadding encrypt&decrypt err!", Toast.LENGTH_SHORT).show();
            return;
        } else {
            Toast.makeText(this, "SM4 nopadding encrypt&decrypt successful!", Toast.LENGTH_SHORT).show();
        }

    }
}

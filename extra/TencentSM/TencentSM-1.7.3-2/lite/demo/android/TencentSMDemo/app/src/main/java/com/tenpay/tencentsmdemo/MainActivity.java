package com.tenpay.tencentsmdemo;

import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Base64;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;
import com.tenpay.utils.Utils;

import java.lang.reflect.Method;
import java.security.Permissions;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemClickListener {

    public static final String[] STR_LIST = {"Version", "SM2", "SM3", "SM4", "KeyMananger"};
    private ArrayAdapter<String> mAdapter;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setTitle("Test TencentSM");
        setContentView(R.layout.activity_main);
        SMUtils.getInstance(this);

        ListView listView = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, android.R.layout.simple_expandable_list_item_1);
        mAdapter.addAll(STR_LIST);
        listView.setAdapter(mAdapter);
        listView.setOnItemClickListener(this);
        Utils.verifyStoragePermissions(this, 110);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        try {
            Method method = getClass().getMethod("test"+STR_LIST[position]);
            method.invoke(this);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void testVersion() {
        String result = SMUtils.getInstance(this).version();
        if(result == null) {
            result = "call Version() failed!!";
        } else {
            result = "testVersion result:" +  result;
        }
        showMsg(result);
    }

    public void testSM2() {
        StringBuffer sb = new StringBuffer();
        sb.append("SM2:\n");
        String tmp;
        //初始化SM2
        long sm2Handler = SMUtils.getInstance(this).SM2InitCtx();
        if(sm2Handler != 0) {
            tmp = "\nsm2Handler="+sm2Handler;
            sb.append(tmp);
        } else {
            tmp = "\nSM2InitCtx error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //生成Keypair
        Object[] keypairs = SMUtils.getInstance(this).SM2GenKeyPair(sm2Handler);
        if(keypairs != null) {
            tmp = "\nprivatekey="+keypairs[0] + "\npublickey="+keypairs[1];
            sb.append(tmp);
        } else {
            tmp = "\nSM2GenKeyPair error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //加密数据
        Log.v("JNITag", "--------------SM2Encrypt");
        String strPlainTxt = "plaintxt0123456789!@#$%^&*()";
        byte[] bytesPlain = strPlainTxt.getBytes();
        byte[] cipherbytes = SMUtils.getInstance(this).SM2Encrypt(sm2Handler, bytesPlain, (String) (keypairs[1]));
        if(cipherbytes != null) {
            tmp = "\nbytesPlain="+ Base64.encodeToString(bytesPlain, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM2Encrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //解密数据
        Log.v("JNITag", "--------------SM2Decrypt");
        bytesPlain = SMUtils.getInstance(this).SM2Decrypt(sm2Handler, cipherbytes, (String) (keypairs[0]));
        if(bytesPlain != null) {
            tmp = "\nbytesPlain="+ Base64.encodeToString(bytesPlain, Base64.DEFAULT);
            sb.append(tmp);
            tmp = "\ndecodedString="+ new String(bytesPlain);
            sb.append(tmp);
        } else {
            tmp = "\nSM2Decrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //签名
        Log.v("JNITag", "--------------SM2Sign");
        String id = "id_tencentsm";
        bytesPlain = strPlainTxt.getBytes();
        byte[] signDatas = SMUtils.getInstance(this).SM2Sign(sm2Handler, bytesPlain, id.getBytes(), (String) (keypairs[1]), (String) (keypairs[0]));
        if(signDatas != null) {
            tmp = "\nsignDatas="+ Base64.encodeToString(signDatas, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM2Sign error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }

        //验证签名
        Log.v("JNITag", "--------------SM2Verify");
        int result = SMUtils.getInstance(this).SM2Verify(sm2Handler, bytesPlain, id.getBytes(), (String) (keypairs[1]), signDatas);
        if(result == 0) {
            tmp = "\nSM2Verify OK";
            sb.append(tmp);
        } else {
            tmp = "\nSM2Verify error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //free
        Log.v("JNITag", "--------------SM2FreeCtx");
        SMUtils.getInstance(this).SM2FreeCtx(sm2Handler);
        tmp = "\nSM2FreeCtx finish()";
        sb.append(tmp);
//        showMsg(sb.toString());
        tmp = "\n-------------------";
        sb.append(tmp);

        //SM2 公钥初始化
        Log.v("JNITag", "--------------SM2InitCtxWithPubKey");
        long newContext = SMUtils.getInstance(this).SM2InitCtxWithPubKey((String) (keypairs[1]));
        if(newContext != 0) {
            tmp = "\nnewContext="+newContext;
            sb.append(tmp);
        } else {
            tmp = "\nSM2InitCtxWithPubKey error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //加密
        Log.v("JNITag", "--------------SM2Encrypt()");
        bytesPlain = strPlainTxt.getBytes();
        cipherbytes = SMUtils.getInstance(this).SM2Encrypt(newContext, bytesPlain, (String) (keypairs[1]));
        if(cipherbytes != null) {
            tmp = "\nSM2Encrypt OK";
            sb.append(tmp);
        } else {
            tmp = "\nSM2Encrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //解密
        Log.v("JNITag", "--------------SM2Decrypt()");
        bytesPlain = SMUtils.getInstance(this).SM2Decrypt(newContext, cipherbytes, (String) (keypairs[0]));
        if(bytesPlain != null) {
            tmp = "\ndecodedString="+ new String(bytesPlain);
            sb.append(tmp);
        } else {
            tmp = "\nSM2Decrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //seed
        Log.v("JNITag", "--------------SM2ReSeed()");
        byte[] byteSeeds = new byte[10];
        Arrays.fill(byteSeeds, (byte)0x3D);
        int isSeed = SMUtils.getInstance(this).SM2ReSeed(newContext, byteSeeds);
        if(isSeed == 0) {
            tmp = "\nSM2ReSeed ok";
            sb.append(tmp);
        } else {
            tmp = "\nSM2ReSeed error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }

        //free
        Log.v("JNITag", "--------------SM2FreeCtx()");
        SMUtils.getInstance(this).SM2FreeCtx(newContext);
        tmp = "\nSM2FreeCtx finish()";
        sb.append(tmp);

        //单独解密
        tmp = "\n-------------------";
        sb.append(tmp);
        Log.v("JNITag", "--------------单独解密");
        long handler = SMUtils.getInstance(this).SM2InitCtx();
        bytesPlain = SMUtils.getInstance(this).SM2Decrypt(handler, cipherbytes, (String)(keypairs[0]));
        if(bytesPlain != null) {
            tmp = "\ndecodedString="+ new String(bytesPlain);
            sb.append(tmp);
        } else {
            tmp = "\nSM2Decrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //free
        Log.v("JNITag", "--------------SM2FreeCtx()");
        SMUtils.getInstance(this).SM2FreeCtx(handler);
        tmp = "\nSM2FreeCtx finish()";
        sb.append(tmp);

        showMsg(sb.toString());
    }

    public void testSM3() {
        StringBuffer sb = new StringBuffer();
        sb.append("SM3:\n");
        //初始化
        String tmp;
        //初始化SM2
        long sm3Handler = SMUtils.getInstance(this).SM3Init();
        if(sm3Handler != 0) {
            tmp = "\nsm3Handler="+sm3Handler;
            sb.append(tmp);
        } else {
            tmp = "\nSM3Init error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //update
        String strPlainTxt = "plaintxt0123456789!@#$%^&*()";
        byte[] bytesPlain = strPlainTxt.getBytes();
        SMUtils.getInstance(this).SM3Update(sm3Handler, bytesPlain);
        tmp = "\nSM3Update ok";
        sb.append(tmp);
        //final
        byte[] bytesMD = SMUtils.getInstance(this).SM3Final(sm3Handler);
        if(bytesMD != null) {
            tmp = "\nbytesMD="+Base64.encodeToString(bytesMD, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM3Final error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //free
        SMUtils.getInstance(this).SM3Free(sm3Handler);
        tmp = "\nSM3Free ok";
        sb.append(tmp);

        tmp = "\n---------------------------";
        sb.append(tmp);
        //sm3
        byte[] bytesMD2 = SMUtils.getInstance(this).SM3(bytesPlain);
        if(bytesMD2 != null) {
            tmp = "\nbytesMD2 ="+Base64.encodeToString(bytesMD2, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM3 error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }

        showMsg(sb.toString());
    }
    public void testSM4() {
        StringBuffer sb = new StringBuffer();
        sb.append("SM4:\n");
        String tmp;
        byte[] iv = new byte[SMUtils.SMS4_IV_LENGTH];
        Arrays.fill(iv, (byte)0x02);
        //key
        byte[] sm4keys = SMUtils.getInstance(this).SM4GenKey();
        if(sm4keys != null) {
            tmp = "\nsm4keys ="+Base64.encodeToString(sm4keys, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM4_GenKey error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //CBC 加密
        String plaintxt = "sm4plaintxt1234561890!@@#$%^&*()QWERTYUIOP";
        byte[] bytesPlain = plaintxt.getBytes();
        byte[] sm4CBCCipher = SMUtils.getInstance(this).SM4CBCEncrypt(bytesPlain, sm4keys, iv);
        if(sm4CBCCipher != null) {
            tmp = "\nSM4_CBC_Encrypt ok";
            sb.append(tmp);
        } else {
            tmp = "\nSM4_CBC_Encrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //CBC 解密
        byte[] decryptbytes = SMUtils.getInstance(this).SM4CBCDecrypt(sm4CBCCipher, sm4keys, iv);
        if(decryptbytes != null) {
//            tmp = "\ndecryptbytes ="+Base64.encodeToString(decryptbytes, Base64.DEFAULT);
//            sb.append(tmp);
            tmp = "\ndecryptstring ="+new String(decryptbytes);
            sb.append(tmp);
        } else {
            tmp = "\nSM4_CBC_Decrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        tmp = "\n----cbc nopadding---------";
        sb.append(tmp);
        //CBC nopadding 加密
        byte[] bytesPlain1 =  Arrays.copyOf(bytesPlain, 16);
        sm4CBCCipher = SMUtils.getInstance(this).SM4CBCEncryptNoPadding(bytesPlain1, sm4keys, iv);
        if(sm4CBCCipher != null) {
            tmp = "\nbytesPlain1 ="+Base64.encodeToString(bytesPlain1, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM4_CBC_Encrypt_NoPadding error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //CBC nopadding 解密
        decryptbytes = SMUtils.getInstance(this).SM4CBCDecryptNoPadding(sm4CBCCipher, sm4keys, iv);
        if(decryptbytes != null) {
            tmp = "\ndecryptbytes ="+Base64.encodeToString(decryptbytes, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM4_CBC_Decrypt_NoPadding error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        tmp = "\n----EBC---------";
        sb.append(tmp);
        //EBC 加密
        byte[] sm4EBCCipher = SMUtils.getInstance(this).SM4ECBEncrypt(bytesPlain, sm4keys);
        if(sm4EBCCipher != null) {
            tmp = "\nSM4_ECB_Encrypt ok";
            sb.append(tmp);
        } else {
            tmp = "\nSM4_ECB_Encrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //EBC 解密
        decryptbytes = SMUtils.getInstance(this).SM4ECBDecrypt(sm4EBCCipher, sm4keys);
        if(decryptbytes != null) {
            tmp = "\ndecryptstring ="+new String(decryptbytes);
            sb.append(tmp);
        } else {
            tmp = "\nSM4_ECB_Decrypt error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        tmp = "\n----ebc nopadding---------";
        sb.append(tmp);
        //EBC nopadding 加密
        sm4EBCCipher = SMUtils.getInstance(this).SM4ECBEncryptNoPadding(bytesPlain1, sm4keys);
        if(sm4EBCCipher != null) {
            tmp = "\nSM4_ECB_Encrypt_NoPadding ok";
            sb.append(tmp);
        } else {
            tmp = "\nSM4_ECB_Encrypt_NoPadding error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //EBC nopadding 解密
        decryptbytes = SMUtils.getInstance(this).SM4ECBDecryptNoPadding(sm4EBCCipher, sm4keys);
        if(decryptbytes != null) {
            tmp = "\ndecryptbytes ="+Base64.encodeToString(decryptbytes, Base64.DEFAULT);
            sb.append(tmp);
        } else {
            tmp = "\nSM4_ECB_Decrypt_NoPadding error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        showMsg(sb.toString());
    }

    public void testKeyMananger() {
        StringBuffer sb = new StringBuffer();
        sb.append("KeyManager:\n");
        String tmp;
        SMKeyManager keymanager  = SMKeyManager.getInstance(this.getApplicationContext(), "/sdcard/tecentsm/");
        //init
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ initWithAuthority");
        boolean result = keymanager.initWithAuthority(Utils.md5("user000"), Utils.md5("pwd000"), null);
        if(result) {
            tmp = "\ninitWithAuthority OK!";
            sb.append(tmp);
        } else {
            tmp = "\ninitWithAuthority error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //keypair gen with desc:desc001.
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ genAsymSM2Key");
        result = keymanager.genAsymSM2Key("desc001", true);
        if(result) {
            tmp = "\ngenAsymSM2Key OK!";
            sb.append(tmp);
        } else {
            tmp = "\ngenAsymSM2Key error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //pubkey get with desc:desc001
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ getAsymPubKey");
        String resStr = keymanager.getAsymPubKey("desc001");
        if(!TextUtils.isEmpty(resStr)) {
            tmp = "\ngetAsymPubKey OK! ="+resStr;
            sb.append(tmp);
        } else {
            tmp = "\ngetAsymPubKey error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        String pubKey = resStr;
        //prikey get with desc:desc001
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ getAsymPriKey");
        resStr = keymanager.getAsymPriKey("desc001");
        if(!TextUtils.isEmpty(resStr)) {
            tmp = "\ngetAsymPriKey OK! ="+resStr;
            sb.append(tmp);
        } else {
            tmp = "\ngetAsymPriKey error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        String priKey = resStr;
        //save keypair with desc:desc002 // 这里只允许保存keypair，不能单独保存公钥和私钥？
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ saveAsymKeypair");
        result = keymanager.saveAsymKeypair("desc002", pubKey, priKey, true);
        if(result) {
            tmp = "\nsaveAsymKeypair OK!";
            sb.append(tmp);
        } else {
            tmp = "\nsaveAsymKeypair error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //key gen with desc:desc003
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ genSymSM4Key");
        result = keymanager.genSymSM4Key("desc003", true);
        if(result) {
            tmp = "\ngenSymSM4Key OK!";
            sb.append(tmp);
        } else {
            tmp = "\ngenSymSM4Key error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //key get with desc:desc003
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ getSymKey");
        resStr = keymanager.getSymKey("desc003");
        if(!TextUtils.isEmpty(resStr)) {
            tmp = "\ngetSymKey OK! ="+resStr;
            sb.append(tmp);
        } else {
            tmp = "\ngetSymKey error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        String symkey = resStr;
        //save key with desc:desc004
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ saveSymKey");
        result = keymanager.saveSymKey("desc004", symkey, true);
        if(result) {
            tmp = "\nsaveSymKey OK!";
            sb.append(tmp);
        } else {
            tmp = "\nsaveSymKey error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }
        //get all key descs
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ getAllAsymKeyDesc");
        String[] descs = keymanager.getAllAsymKeyDesc();
        if(descs != null) {
            tmp = "\ngetAllAsymKeyDesc OK!";
            sb.append(tmp);
        } else {
            tmp = "\ngetAllAsymKeyDesc error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }

        //remove with desc001,desc002,desc003,desc004
        for(int i=1; i<5; i++) {
            String desctmp = "desc00"+i;
            Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$ delDataWithDesc");
            result = keymanager.delDataWithDesc(desctmp);
            if(result) {
                tmp = "\ndelDataWithDesc("+desctmp+") OK!";
                sb.append(tmp);
            } else {
                tmp = "\ndelDataWithDesc("+desctmp+") error!";
                sb.append(tmp);
                showMsg(sb.toString());
                return;
            }
        }
        //remove all user data
        Log.v("JNITag", "$$$$$$$$$$$$$$$$$$$$$$$$$$delWithAuthority");
        result = keymanager.delWithAuthority(Utils.md5("user000"), Utils.md5("pwd000"), null);
        if(result) {
            tmp = "\ndelWithAuthority OK!";
            sb.append(tmp);
        } else {
            tmp = "\ndelWithAuthority error;";
            sb.append(tmp);
            showMsg(sb.toString());
            return;
        }


        showMsg(sb.toString());
    }

    private void showMsg(String msg) {
        AlertDialog dlg = new AlertDialog.Builder(this).setTitle("invoke result")
                .setCancelable(true)
                .setNegativeButton(android.R.string.ok, null)
                .setMessage(msg)
                .create();
        dlg.show();
    }

}

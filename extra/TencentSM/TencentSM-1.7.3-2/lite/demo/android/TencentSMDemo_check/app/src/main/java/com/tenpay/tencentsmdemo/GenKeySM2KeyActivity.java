package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.tenpay.utils.Constant;
import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;

import java.util.ArrayList;
import java.util.Arrays;

public class GenKeySM2KeyActivity extends BaseToolbarActivity implements View.OnClickListener{
    private ArrayAdapter mAdapter;
    private SMKeyManager keymanager;
    private long smHandler = 0;
    private SMUtils smutils;
    private String[] mDataItems;
    private boolean isOKKeys = false;

    public static Intent getStartIntent(Context context, String backtxt) {
        Intent intent = new Intent(context, GenKeySM2KeyActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_2btn);
        setTitle(""+getText(R.string.sm2)+getText(R.string.gen_keypair));
        setMyTheme(THEME_BG_WHITE);

        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, R.layout.layout_txt);
        listview.setAdapter(mAdapter);

        //btn
        findViewById(R.id.b_bottom).setOnClickListener(this);
        findViewById(R.id.b_bottom2).setOnClickListener(this);

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        smutils = SMUtils.getInstance(this);
        smHandler = smutils.SM2InitCtx();
        mDataItems = new String[4];
        mDataItems[0] = "公钥";
        mDataItems[2] = "私钥";
        reCreate();
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            //save key
            if(isOKKeys) {
                showInputDlg(getText(R.string.input_desc), getText(R.string.input_desc_note), new IDialogOnClick() {
                    @Override
                    public void onClick(DialogInterface dialog, int which, Object[] params) {
                        if(!TextUtils.isEmpty((String)params[0])) {
                            boolean lret = keymanager.saveAsymKeypair((String)params[0], mDataItems[1], mDataItems[3], true);
                            if(lret) {
                                showIconDlg(getText(R.string.save_secretkey), R.mipmap.ic_blue_ok);
                            }
                        }
                    }
                });
            }
        } else {
            //recreate
            reCreate();
        }
    }

    private void reCreate() {
        isOKKeys = false;
        mDataItems[1] = mDataItems[3] = "generate failed!";
        if(smHandler > 0) {
            Object[] keyPair= smutils.SM2GenKeyPair(smHandler);
            if(keyPair != null) {
                mDataItems[1] = (String) keyPair[1];
                mDataItems[3] = (String) keyPair[0];
                isOKKeys = true;
            }
        }
        mAdapter.clear();
        mAdapter.addAll(mDataItems);
        mAdapter.notifyDataSetChanged();
    }


    @Override
    protected void onDestroy() {
        if(smHandler > 0) {
            smutils.SM2FreeCtx(smHandler);
        }

        super.onDestroy();
    }

}

package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;

public class ImportSM2KeyActivity extends BaseToolbarActivity implements View.OnClickListener{
    private SMKeyManager keymanager;
    private long smHandler = 0;
    private SMUtils smutils;
    private EditText et_top;
    private EditText et_top2;

    public static Intent getStartIntent(Context context, String backtxt) {
        Intent intent = new Intent(context, ImportSM2KeyActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_2et_btn);
        setTitle(getText(R.string.import_keypair));
        setMyTheme(THEME_BG_WHITE);

        //btn
        findViewById(R.id.b_bottom).setOnClickListener(this);
        //edit
        et_top = findViewById(R.id.et_top);
        et_top2 = findViewById(R.id.et_top2);

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        smutils = SMUtils.getInstance(this);
        smHandler = smutils.SM2InitCtx();

        if(smHandler > 0) {
            Object[] keyPair= smutils.SM2GenKeyPair(smHandler);
            if(keyPair != null) {
                et_top.setText((String)keyPair[1]);
                et_top2.setText((String)keyPair[0]);
            }
        }

    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            final String pubkey = et_top.getText().toString();
            final String prikey = et_top2.getText().toString();
            if(TextUtils.isEmpty(pubkey)&&TextUtils.isEmpty(prikey)) {
                Toast.makeText(this, "content is empty", Toast.LENGTH_SHORT).show();
                return;
            }
            showInputDlg(getText(R.string.input_desc), getText(R.string.input_desc_note), new IDialogOnClick() {
                @Override
                public void onClick(DialogInterface dialog, int which, Object[] params) {
                if(!TextUtils.isEmpty((String)params[0])) {
                    boolean lret = keymanager.saveAsymKeypair((String)params[0], pubkey, prikey, true);
                    if(lret) {
                        showIconDlg(getText(R.string.save_secretkey), R.mipmap.ic_blue_ok);
                    }
                }
                }
            });

        }
    }


    @Override
    protected void onDestroy() {
        if(smHandler > 0) {
            smutils.SM2FreeCtx(smHandler);
        }
        super.onDestroy();
    }

}

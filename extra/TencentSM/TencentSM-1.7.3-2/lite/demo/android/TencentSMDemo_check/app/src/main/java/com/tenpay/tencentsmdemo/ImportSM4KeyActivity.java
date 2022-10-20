package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;
import com.tenpay.utils.Utils;

public class ImportSM4KeyActivity extends BaseToolbarActivity implements View.OnClickListener{
    private SMKeyManager keymanager;
    private SMUtils smutils;
    private EditText et_top;

    public static Intent getStartIntent(Context context, String backtxt) {
        Intent intent = new Intent(context, ImportSM4KeyActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_2et_btn);
        setTitle(getText(R.string.import_key));
        setMyTheme(THEME_BG_WHITE);

        //btn
        Button b_botom = findViewById(R.id.b_bottom);
        b_botom.setOnClickListener(this);
        b_botom.setText(R.string.import_key);
        //edit
        et_top = findViewById(R.id.et_top);
        findViewById(R.id.tv_top2).setVisibility(View.GONE);
        findViewById(R.id.et_top2).setVisibility(View.GONE);
        ((TextView)findViewById(R.id.tv_top)).setText(R.string.secret_key);
        et_top.setHint(R.string.input_key_desc);

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        smutils = SMUtils.getInstance(this);

        byte[] keys = smutils.SM4GenKey();
        if(keys != null) {
            et_top.setText(Utils.byte2String(keys));
        }
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            final String keystr = et_top.getText().toString();

            if(TextUtils.isEmpty(keystr)) {
                Toast.makeText(this, "content is empty", Toast.LENGTH_SHORT).show();
                return;
            }
            showInputDlg(getText(R.string.input_desc), getText(R.string.input_desc_note), new IDialogOnClick() {
                @Override
                public void onClick(DialogInterface dialog, int which, Object[] params) {
                if(!TextUtils.isEmpty((String)params[0])) {
                    boolean lret = keymanager.saveSymKey((String)params[0], keystr, true);
                    if(lret) {
                        showIconDlg(getText(R.string.save_secretkey), R.mipmap.ic_blue_ok);
                    }
                }
                }
            });

        }
    }

}

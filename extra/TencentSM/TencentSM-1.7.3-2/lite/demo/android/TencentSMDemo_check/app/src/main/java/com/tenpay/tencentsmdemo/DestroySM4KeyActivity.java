package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

import com.tenpay.utils.SMKeyManager;

public class DestroySM4KeyActivity extends BaseToolbarActivity implements View.OnClickListener{
    private SMKeyManager keymanager;
    private EditText et_top;

    public static Intent getStartIntent(Context context, String backtxt) {
        Intent intent = new Intent(context, DestroySM4KeyActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_1et_btn);
        setTitle(getText(R.string.del_key));
        setMyTheme(THEME_BG_WHITE);

        //btn
        findViewById(R.id.b_bottom).setOnClickListener(this);
        //edit
        et_top = findViewById(R.id.et_top);

        keymanager = SMKeyManager.getInstance(this, null);
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            String desc = et_top.getText().toString();
            if(TextUtils.isEmpty(desc)) {
                Toast.makeText(this, getText(R.string.input_desc), Toast.LENGTH_SHORT).show();
                return;
            }
            boolean lret = keymanager.delDataWithDesc(desc);
            if(lret) {
                showIconDlg(getText(R.string.del_key), R.mipmap.ic_blue_ok);
            } else {
                Toast.makeText(this, "" + getText(R.string.del_key)+getText(R.string.failed), Toast.LENGTH_SHORT).show();
            }
        }
    }

}

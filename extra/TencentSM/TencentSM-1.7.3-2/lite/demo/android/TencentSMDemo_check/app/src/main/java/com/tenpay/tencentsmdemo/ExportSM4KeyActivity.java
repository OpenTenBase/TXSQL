package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import com.tenpay.utils.SMKeyManager;

public class ExportSM4KeyActivity extends BaseToolbarActivity implements View.OnClickListener{
    private ArrayAdapter mAdapter;
    private SMKeyManager keymanager;
    private String[] mDataItems;
    private EditText et_desc;

    public static Intent getStartIntent(Context context, String backtxt) {
        Intent intent = new Intent(context, ExportSM4KeyActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_et_btn_list);
        setTitle(getText(R.string.export_key));
        setMyTheme(THEME_BG_WHITE);

        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, R.layout.layout_txt);
        listview.setAdapter(mAdapter);
        //text and edit
        et_desc = findViewById(R.id.et_desc);

        //btn
        findViewById(R.id.b_bottom).setOnClickListener(this);

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        mDataItems = new String[2];
        mDataItems[0] = (String) getText(R.string.secret_key);

        setContent(true, null);
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.b_bottom) {
            String desc = et_desc.getText().toString();
            if(!TextUtils.isEmpty(desc)) {
                setContent(false, desc);
            }
        }
    }

    private void setContent(boolean init, String desc) {
        if(init) {
            mDataItems[1] = "empty";
        } else {
            mDataItems[1] = "export failed!";
        }
        if(keymanager != null && desc != null) {
            String key = keymanager.getSymKey(desc);
            if(key != null) {
                mDataItems[1] = key;
            }
        }
        mAdapter.clear();
        mAdapter.addAll(mDataItems);
        mAdapter.notifyDataSetChanged();
    }

}

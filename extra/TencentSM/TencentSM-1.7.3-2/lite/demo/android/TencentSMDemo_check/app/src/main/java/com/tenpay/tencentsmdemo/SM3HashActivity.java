package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;
import com.tenpay.utils.Utils;

public class SM3HashActivity extends BaseToolbarActivity implements View.OnClickListener{
    private ArrayAdapter mAdapter;
    private SMKeyManager keymanager;
    private String[] mDataItems;
    private EditText et_desc;
    private SMUtils smutils;

    public static Intent getStartIntent(Context context, String backtxt) {
        Intent intent = new Intent(context, SM3HashActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_et_btn_list);
        setTitle(getString(R.string.gen_hash));
        setMyTheme(THEME_BG_WHITE);

        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, R.layout.layout_txt);
        listview.setAdapter(mAdapter);
        //text and edit
        ((TextView)findViewById(R.id.tv_top)).setText(R.string.str_forhash);
        et_desc = findViewById(R.id.et_desc);
        et_desc.setText("id：xxxxxx\npswd：xxxxxx");
        et_desc.setHint(R.string.str_forhash_hint);
        et_desc.setMinHeight((int) Utils.dp2px(this, 50));
        et_desc.setGravity(Gravity.LEFT);
        et_desc.setPadding(Utils.dp2px(this, 3), Utils.dp2px(this, 2), Utils.dp2px(this, 2), Utils.dp2px(this, 3));
        et_desc.clearFocus();
        //btn
        Button btn = findViewById(R.id.b_bottom);
        btn.setOnClickListener(this);
        btn.setText(R.string.gen_hash);

        //tools init
        keymanager = SMKeyManager.getInstance(this, null);
        smutils = SMUtils.getInstance(this);
        mDataItems = new String[2];
        mDataItems[0] = (String) getText(R.string.hash_data);

        setContent(true, null);
    }

    @Override
    public void onClick(View v) {
        if(v.getId() == R.id.b_bottom) {
            String desc = et_desc.getText().toString();
            if(!TextUtils.isEmpty(desc)) {
                setContent(false, desc);
            } else {
                mDataItems[1] = "empty";
                mAdapter.clear();
                mAdapter.addAll(mDataItems);
                mAdapter.notifyDataSetChanged();
            }
        }
    }

    private void setContent(boolean init, String strForHash) {
        if(init) {
            mDataItems[1] = "empty";
        } else {
            mDataItems[1] = "export failed!";
        }
        if(keymanager != null && strForHash != null) {
            byte[] results = smutils.SM3(strForHash.getBytes());
            if(results != null) {
                mDataItems[1] = Utils.byte2String(results);
            }
        }
        mAdapter.clear();
        mAdapter.addAll(mDataItems);
        mAdapter.notifyDataSetChanged();
    }

}

package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;
import com.tenpay.utils.Utils;

import java.util.ArrayList;
import java.util.List;

public class SM4ECBEnCryptActivity extends BaseToolbarActivity implements View.OnClickListener{
    private myAdapter mAdapter;
    private SMKeyManager keymanager;
    private SMUtils smutils;
    private ArrayList<String> mDataItems;
    private EditText mEditText;
    private String mDesc;
    private byte[] mCiphers;
    private String symnkey;

    public static Intent getStartIntent(Context context, String backtxt, String desc) {
        Intent intent = new Intent(context, SM4ECBEnCryptActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        intent.putExtra(STRID_DESC, desc);
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_1et_list_btn);
        setTitle(getString(R.string.ecb_encrypt));
        setMyTheme(THEME_BG_WHITE);

        mDataItems = new ArrayList<>();
        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new myAdapter(mDataItems, this);
        //edit
        mEditText = findViewById(R.id.et_top);
        //btn
        findViewById(R.id.b_bottom).setOnClickListener(this);
        findViewById(R.id.b_bottom2).setOnClickListener(this);
        //desc
        mDataItems.add(getString(R.string.use_key_desc));
        mDesc = getIntent().getStringExtra(STRID_DESC);
        if(mDesc == null) {
            mDesc = "";
        }
        mDataItems.add(mDesc);
        //result
        mDataItems.add(getString(R.string.ciphertext));

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        smutils = SMUtils.getInstance(this);
        symnkey = keymanager.getSymKey(mDesc);
        reEncrypt();
        listview.setAdapter(mAdapter);
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            if(mCiphers != null) {
                startActivity(SM4ECBDecryptActivity.getStartIntent(this, getString(R.string.cbc_encrypt), mDesc, mCiphers));
            }
        } else {
            reEncrypt();
        }
    }

    private void reEncrypt() {
        String str = mEditText.getText().toString();
        if(TextUtils.isEmpty(str)) {
            return;
        }
        byte[] cipher = smutils.SM4ECBEncrypt(str.getBytes(), Utils.string2bytes(symnkey));
        if(cipher == null) {
            Toast.makeText(this, "encrypt error!", Toast.LENGTH_SHORT).show();
            return;
        }
        mCiphers = cipher;
        String strtmp = Utils.byte2String(cipher);
        synchronized (this) {
            if(mDataItems.size() > 3) {
                mDataItems.remove(3);
            }
            mDataItems.add(3, strtmp);
        }
        mAdapter.notifyDataSetChanged();
    }

    private class myAdapter extends BaseAdapter {
        private final LayoutInflater mLayoutInflater;
        private final List<String> mList;

        public myAdapter(List<String> list, Context context) {
            mLayoutInflater = LayoutInflater.from(context);
            mList = list;
        }

        @Override
        public int getCount() {
            return mList.size();
        }

        @Override
        public Object getItem(int position) {
            return null;
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if(convertView == null) {
                convertView = mLayoutInflater.inflate(R.layout.layout_txt, parent, false);
            }
            TextView tvLeft = (TextView) convertView;
            synchronized (SM4ECBEnCryptActivity.this) {
                tvLeft.setText(mList.get(position));
            }
            if(position%2 == 0) {
                tvLeft.setTextColor(Color.DKGRAY);
                tvLeft.setTextSize(TypedValue.COMPLEX_UNIT_SP, 16);
            } else {
                tvLeft.setTextColor(Color.BLACK);
                tvLeft.setTextSize(TypedValue.COMPLEX_UNIT_SP, 18);
            }
            return convertView;
        }
    }
}

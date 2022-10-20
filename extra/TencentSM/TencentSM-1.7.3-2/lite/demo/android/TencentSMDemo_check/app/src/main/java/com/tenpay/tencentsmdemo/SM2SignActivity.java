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
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.tenpay.utils.SMKeyManager;
import com.tenpay.utils.SMUtils;
import com.tenpay.utils.Utils;

import java.util.ArrayList;
import java.util.List;

public class SM2SignActivity extends BaseToolbarActivity implements View.OnClickListener{
    private myAdapter mAdapter;
    private SMKeyManager keymanager;
    private SMUtils smutils;
    private ArrayList<String> mDataItems;
    private String mDesc;
    private long mHandler = 0;
    private String priKey;
    private String pubKey;
    private byte[] mMessage;
    private byte[] mSignedData;

    public static Intent getStartIntent(Context context, String backtxt, String desc) {
        Intent intent = new Intent(context, SM2SignActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        intent.putExtra(STRID_DESC, desc);
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_2btn);
        setTitle(getText(R.string.sm2_sign));
        setMyTheme(THEME_BG_WHITE);

        mDataItems = new ArrayList<>();
        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new myAdapter(mDataItems, this);
        //btn
        Button btn = findViewById(R.id.b_bottom);
        btn.setOnClickListener(this);
        btn.setText(R.string.sm2_verigy);
        btn = findViewById(R.id.b_bottom2);
        btn.setOnClickListener(this);
        btn.setText(R.string.sm2_resign);

        //desc
        Intent intent = getIntent();
        mDataItems.add(getString(R.string.msg_forsigned));
        mMessage = Utils.getRandombytes(128);
        mDataItems.add(Utils.byte2String(mMessage));
        mDataItems.add(getString(R.string.id));
        mDataItems.add("ID123456789");
        mDataItems.add(getString(R.string.use_key_desc));
        mDesc = intent.getStringExtra(STRID_DESC);
        if(mDesc == null) {
            mDesc = "";
        }
        mDataItems.add(mDesc);
        mDataItems.add(getString(R.string.signed_data));

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        smutils = SMUtils.getInstance(this);
        String[] keypair = keymanager.getAsymPriKeypair(mDesc);
        priKey = keypair[0];
        pubKey = keypair[1];
        mHandler = smutils.SM2InitCtx();
        reSign();
        listview.setAdapter(mAdapter);
    }
    @Override
    protected void onDestroy() {
        if(mHandler > 0) {
            smutils.SM2FreeCtx(mHandler);
        }
        super.onDestroy();
    }


    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            //签名验证
            startActivity(SM2VerifyActivity.getStartIntent(this, getString(R.string.back), mDesc, mMessage, mDataItems.get(3), mSignedData));
        } else {
            reSign();
        }
    }

    private void reSign() {
        if(mHandler <= 0) {
            return;
        }
        String id = mDataItems.get(3);
        byte[] results = smutils.SM2Sign(mHandler, mMessage, id.getBytes(), pubKey, priKey);
        if(results == null) {
            Toast.makeText(this, "sign error!", Toast.LENGTH_SHORT).show();
            return;
        }
        mSignedData = results;
        String strtmp = Utils.byte2String(results);
        synchronized (this) {
            if(mDataItems.size() > 7) {
                mDataItems.remove(7);
            }
            mDataItems.add(7, strtmp);
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
            synchronized (SM2SignActivity.this) {
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

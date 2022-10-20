package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;

import com.tenpay.utils.SMKeyManager;

import java.util.ArrayList;
import java.util.List;

public class SelectDescActivity extends BaseToolbarActivity implements View.OnClickListener{
    public static final int TYPE_SM2_ENCRYPT = 1;
    public static final int TYPE_SM2_SIGN = 2;
    public static final int TYPE_SM4_CBC = 3;
    public static final int TYPE_SM4_ECB = 4;

    private myAdapter mAdapter;
    private SMKeyManager keymanager;
    private ArrayList<String> mDataItems;
    private int mPageType = TYPE_SM2_ENCRYPT;
    public static Intent getStartIntent(Context context, String backtxt, int pagetype) {
        Intent intent = new Intent(context, SelectDescActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        intent.putExtra(STRID_PAGETYPE, pagetype);
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_btn);
        setMyTheme(THEME_BG_WHITE);

        mPageType = getIntent().getIntExtra(STRID_PAGETYPE, TYPE_SM2_ENCRYPT);
        mDataItems = new ArrayList<>();
        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new myAdapter(mDataItems, this);
        listview.setOnItemClickListener(mAdapter);

        //btn
        Button btn = findViewById(R.id.b_bottom);
        btn.setOnClickListener(this);
        if(mPageType == TYPE_SM2_ENCRYPT) {
            setTitle(getText(R.string.sel_keypair));
            btn.setText(R.string.start_crypt);
        } else if(mPageType == TYPE_SM2_SIGN){
            setTitle(getText(R.string.sel_keypair));
            btn.setText(R.string.start_sign);
        } else if(mPageType == TYPE_SM4_CBC){
            setTitle(getText(R.string.sel_keydesc));
            btn.setText(R.string.start_crypt);
        } else {
            setTitle(getText(R.string.sel_keydesc));
            btn.setText(R.string.start_crypt);
        }

        keymanager = SMKeyManager.getInstance(this, null);

        genlist();
        listview.setAdapter(mAdapter);
    }

    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            //save key
            String desc = null;
            desc = mAdapter.getSelectStr();
            if(!TextUtils.isEmpty(desc)) {
                if(mPageType == TYPE_SM2_ENCRYPT) {
                    startActivity(SM2CryptActivity.getStartIntent(this, getString(R.string.back), desc));
                } else if(mPageType == TYPE_SM2_SIGN){
                    startActivity(SM2SignActivity.getStartIntent(this, getString(R.string.back), desc));
                } else if(mPageType == TYPE_SM4_CBC){
                    startActivity(SM4CBCEnCryptActivity.getStartIntent(this, getString(R.string.back), desc));
                } else if(mPageType == TYPE_SM4_ECB){
                    startActivity(SM4ECBEnCryptActivity.getStartIntent(this, getString(R.string.back), desc));
                }
            }
        }
    }

    private void genlist() {
        String[] desc = null;
        if(mPageType == TYPE_SM2_ENCRYPT || mPageType == TYPE_SM2_SIGN ) {
            desc = keymanager.getAllAsymKeyDesc();
        } else {
            desc = keymanager.getAllSymKeyDesc();
        }
        synchronized (this) {
            mDataItems.clear();
            if(desc != null) {
                for(String str : desc) {
                    mDataItems.add(str);
                }
            }
        }
        mAdapter.notifyDataSetChanged();
    }

    private class myAdapter extends BaseAdapter implements AdapterView.OnItemClickListener {
        private final LayoutInflater mLayoutInflater;
        private final List<String> mList;
        private int selectedPos = 0;

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
            Holder holder = null;
            if(convertView == null) {
                convertView = mLayoutInflater.inflate(R.layout.layout_txt_check, parent, false);
                holder = new Holder();
                holder.mTvLeft = convertView.findViewById(R.id.tv_item_left);
                holder.mIvRight = convertView.findViewById(R.id.iv_item_right);
                convertView.setTag(holder);
            } else {
                holder = (Holder) convertView.getTag();
                holder.mTvLeft = convertView.findViewById(R.id.tv_item_left);
                holder.mIvRight = convertView.findViewById(R.id.iv_item_right);
            }

            synchronized (SelectDescActivity.this) {
                holder.mTvLeft.setText(mList.get(position));
                if(position == selectedPos) {
                    holder.mIvRight.setVisibility(View.VISIBLE);
                } else {
                    holder.mIvRight.setVisibility(View.GONE);
                }
            }

            return convertView;
        }

        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            this.selectedPos = position;
            notifyDataSetChanged();
        }

        public String getSelectStr() {
            String result = null;
            if(mList != null && mList.size()>0) {
                result = mList.get(selectedPos);
            }
            return result;
        }

        class Holder {
            public TextView mTvLeft;
            public ImageView mIvRight;
        }
    }
}

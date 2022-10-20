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

public class SM4ECBDecryptActivity extends BaseToolbarActivity implements View.OnClickListener{
    private myAdapter mAdapter;
    private SMKeyManager keymanager;
    private SMUtils smutils;
    private ArrayList<String> mDataItems;
    private String mDesc;
    private byte[] ciphertxt;
    private String symkey;

    public static Intent getStartIntent(Context context, String backtxt, String desc, byte[] ciphertext) {
        Intent intent = new Intent(context, SM4ECBDecryptActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        intent.putExtra(STRID_DESC, desc);
        intent.putExtra(STRID_CIPHERTXT, ciphertext);
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list_2btn);
        setTitle(getText(R.string.ecb_decrypt));
        setMyTheme(THEME_BG_WHITE);

        mDataItems = new ArrayList<>();
        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new myAdapter(mDataItems, this);
        //btn
        Button btn = findViewById(R.id.b_bottom);
        btn.setOnClickListener(this);
        btn.setText(R.string.go_entry_page);
        findViewById(R.id.b_bottom2).setVisibility(View.INVISIBLE);
        //desc
        Intent intent = getIntent();
        ciphertxt = intent.getByteArrayExtra(STRID_CIPHERTXT);
        mDataItems.add(getString(R.string.ciphertxt));
        mDataItems.add(Utils.byte2String(ciphertxt));
        mDataItems.add(getString(R.string.use_key_desc));
        mDesc = intent.getStringExtra(STRID_DESC);
        if(mDesc == null) {
            mDesc = "";
        }
        mDataItems.add(mDesc);
        //result
        mDataItems.add(getString(R.string.decrypt_res));

        keymanager = SMKeyManager.getInstance(this, null);

        //genkey init
        smutils = SMUtils.getInstance(this);
        symkey = keymanager.getSymKey(mDesc);
        reDecrypt();
        listview.setAdapter(mAdapter);
    }


    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_bottom) {
            //goto entry page
            mLocalBroadcastMannager.sendBroadcast(new Intent(BC_ID_BACKTOENTRY));
        }
    }

    private void reDecrypt() {
        byte[] plain = smutils.SM4ECBDecrypt(ciphertxt, Utils.string2bytes(symkey));
        if(plain == null) {
            Toast.makeText(this, "encrypt error!", Toast.LENGTH_SHORT).show();
            return;
        }
        synchronized (this) {
            if(mDataItems.size() > 5) {
                mDataItems.remove(5);
            }
            mDataItems.add(5, new String(plain));
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
            synchronized (SM4ECBDecryptActivity.this) {
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

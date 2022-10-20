package com.tenpay.tencentsmdemo;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.tenpay.utils.SMKeyManager;

public class ModuleListActivity extends BaseToolbarActivity implements AdapterView.OnItemClickListener {
    public static final int[] LIST_STRS_RES = {R.string.module_authority, R.string.module_crypt};
    private ArrayAdapter mAdapter;
    private String identity;
    private String pincode;


    public static Intent getStartIntent(Context context, String backtxt, String id, String pincode) {
        Intent intent = new Intent(context, ModuleListActivity.class);
        if(!TextUtils.isEmpty(backtxt)) {
            intent.putExtra(STRID_BACKTXT, backtxt);
        }
        intent.putExtra(STRID_IDENTIRY, id);
        intent.putExtra(STRID_PINCODE, pincode);
        return intent;
    }
    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_list);
        setTitle(R.string.module_title);
        setMyTheme(THEME_BG_WHITE);

        Intent intent = getIntent();
        identity =intent.getStringExtra(STRID_IDENTIRY);
        pincode =intent.getStringExtra(STRID_PINCODE);

        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, android.R.layout.simple_expandable_list_item_1);
        String[] arrayStrs = new String[LIST_STRS_RES.length];
        for(int i = 0; i < LIST_STRS_RES.length; i++) {
            arrayStrs[i] = (String)getText(LIST_STRS_RES[i]);
        }
        mAdapter.addAll(arrayStrs);
        listview.setAdapter(mAdapter);
        listview.setOnItemClickListener(this);

        SMKeyManager.getInstance(this, null).initWithAuthority(identity, pincode, null);
    }


    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if(position == 0) {
            startActivity(KeyManListActivity.getStartIntent(this, getString(R.string.module_title), identity, pincode));
        } else {
            startActivity(CryptListActivity.getStartIntent(this, getString(R.string.module_title), identity, pincode));
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        SMKeyManager.destroyInstance();
    }
}

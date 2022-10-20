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

public class KeyManListActivity extends BaseToolbarActivity implements AdapterView.OnItemClickListener {
    public static final int[] LIST_STRS_RES = {R.string.sm2, R.string.gen_keypair, R.string.import_keypair, R.string.export_keypair, R.string.del_keypair,
            R.string.sm4, R.string.gen_key, R.string.import_key, R.string.export_key, R.string.del_key};
    private ArrayAdapter mAdapter;

    public static Intent getStartIntent(Context context, String backtxt, String id, String pincode) {
        Intent intent = new Intent(context, KeyManListActivity.class);
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
        setTitle(R.string.module_authority);

        ListView listview = findViewById(R.id.lv_content);
        mAdapter = new ArrayAdapter(this, android.R.layout.simple_expandable_list_item_1);
        CharSequence[] arrayStrs = new String[LIST_STRS_RES.length];
        for(int i = 0; i < LIST_STRS_RES.length; i++) {
            arrayStrs[i] = getText(LIST_STRS_RES[i]);
        }
        mAdapter.addAll(arrayStrs);
        listview.setAdapter(mAdapter);
        listview.setOnItemClickListener(this);
    }


    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        switch (position) {
            case 1://生成秘钥对
                startActivity(GenKeySM2KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 2://导入秘钥对
                startActivity(ImportSM2KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 3://导出秘钥对
                startActivity(ExportSM2KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 4://销毁秘钥对
                startActivity(DestroySM2KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 6://生成秘钥
                startActivity(GenSM4KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 7://导入秘钥
                startActivity(ImportSM4KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 8://导出秘钥
                startActivity(ExportSM4KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
            case 9://销毁秘钥
                startActivity(DestroySM4KeyActivity.getStartIntent(this, (String) getText(R.string.module_authority)));
                break;
        }
    }
}

package com.tenpay.tencentsmdemo;

import android.app.Activity;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

public class EntryActivity extends BaseToolbarActivity implements View.OnClickListener {
    private EditText et_id;
    private EditText et_pincode;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_entry);
        setTitle(R.string.main_title);

        et_id = findViewById(R.id.et_id);
        et_pincode = findViewById(R.id.et_pincode);
        findViewById(R.id.b_enter).setOnClickListener(this);

        //test
        et_id.setText("test");
        et_pincode.setText("123456");
    }


    @Override
    public void onClick(View v) {
        int id = v.getId();
        if(id == R.id.b_enter) {
            String identity = et_id.getText().toString();
            String pincode = et_pincode.getText().toString();
            if(TextUtils.isEmpty(identity)|| TextUtils.isEmpty(pincode)) {
                Toast.makeText(this, "some input is empty", Toast.LENGTH_SHORT).show();
            } else {
                startActivity(ModuleListActivity.getStartIntent(this, (String)getText(R.string.back), identity, pincode));
            }
        }
    }
}

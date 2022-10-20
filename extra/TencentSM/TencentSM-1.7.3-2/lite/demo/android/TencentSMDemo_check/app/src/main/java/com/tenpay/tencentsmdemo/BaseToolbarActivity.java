package com.tenpay.tencentsmdemo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

public class BaseToolbarActivity extends AppCompatActivity {
    public static final String STRID_BACKTXT = "strid_backtxt";
    public static final String STRID_IDENTIRY = "strid_identiry";
    public static final String STRID_PINCODE = "strid_pincode";
    public static final String STRID_DESC = "strid_desc";
    public static final String STRID_CIPHERTXT = "strid_ciphertxt";
    public static final String STRID_PAGETYPE = "strid_pagetype";
    public static final String STRID_MESSAGE = "strid_message";
    public static final String STRID_ID = "strid_id";
    public static final String STRID_DATA = "strid_data";
    public static final String STRID_IV = "strid_iv";
    private ViewGroup mRootlayout;
    private ImageView iv_title_back;
    private TextView tv_title_left;
    private TextView tv_title_center;
    private View ll_title_left;
    protected LocalBroadcastManager mLocalBroadcastMannager;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        super.setContentView(R.layout.activity_base);
        mRootlayout = findViewById(R.id.ll_rootlayout);
        iv_title_back = mRootlayout.findViewById(R.id.iv_title_back);
        tv_title_left = mRootlayout.findViewById(R.id.tv_title_left);
        tv_title_center = mRootlayout.findViewById(R.id.tv_title_center);
        ll_title_left = findViewById(R.id.ll_title_left);
        ll_title_left.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onTitleBackClick();                
            }
        });

        setMyTheme(THEME_BG_BLUE);

        //根据back_txt
        Intent intent = getIntent();
        String backtxt = intent.getStringExtra(STRID_BACKTXT);
        if(TextUtils.isEmpty(backtxt)) {
            setTitleBackVisible(View.GONE);
        } else {
            setTitleBackVisible(View.VISIBLE);
            setTitleLeftTxt(backtxt);
        }

        //broaccat inner
        mLocalBroadcastMannager = LocalBroadcastManager.getInstance(this);
        IntentFilter intentFilter = new IntentFilter();
        onSetBroadCast(intentFilter);
        mLocalBroadcastMannager.registerReceiver(new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                onLocalReceive(context, intent, intent.getAction());
            }
        }, intentFilter);
    }


    protected void onTitleBackClick() {
        onBackPressed();
    }

    @Override
    public void setContentView(int layoutResID) {
        View view = getLayoutInflater().inflate(layoutResID, mRootlayout, false);
        mRootlayout.addView(view);
    }

    @Override
    public void setTitle(CharSequence title) {
        tv_title_center.setText(title);
    }

    public void setTitleBackVisible(int visible) {
        ll_title_left.setVisibility(visible);
    }
    public void setTitleLeftTxt(CharSequence cs) {
        tv_title_left.setText(cs);
    }

    /***********     theme          *********/
    public static final int THEME_BG_BLUE = 1;
    public static final int THEME_BG_WHITE = 2;

    class Theme {
        public int backcolor;
        public int forecolor;
        public int backtextcolor;
        public int titlebackres;
    }
    private Theme mTheme;
    public void setMyTheme(int themetype) {
        if(mTheme == null) {
            mTheme = new Theme();
        }
        if(themetype == THEME_BG_WHITE) {
            mTheme.backcolor = Color.WHITE;
            mTheme.forecolor = Color.BLACK;
            mTheme.backtextcolor = getResources().getColor(R.color.colorPrimary);
            mTheme.titlebackres = R.drawable.shape_back_blue;
        } else {
            mTheme.backcolor = getResources().getColor(R.color.colorPrimary);
            mTheme.forecolor = Color.WHITE;
            mTheme.backtextcolor = Color.WHITE;
            mTheme.titlebackres = R.drawable.shape_back_white;
        }
        iv_title_back.setImageResource(mTheme.titlebackres);
        tv_title_center.setTextColor(mTheme.forecolor);
        tv_title_left.setTextColor(mTheme.backtextcolor);
        mRootlayout.setBackgroundColor(mTheme.backcolor);
    }

    /***********     Dlg          *********/
    protected void showMsgDlg(CharSequence title, CharSequence msg) {
        AlertDialog dlg = new AlertDialog.Builder(this).setTitle(title)
                .setCancelable(true)
                .setNegativeButton(android.R.string.ok, null)
                .setMessage(msg)
                .create();
        dlg.show();
    }

    protected void showIconDlg(CharSequence title, int imageresid) {
        ImageView iv = new ImageView(this);
        iv.setPadding(20, 20, 20, 20);
        iv.setImageResource(imageresid);
        AlertDialog dlg = new AlertDialog.Builder(this).setTitle(title)
                .setCancelable(true)
                .setNegativeButton(android.R.string.ok, null)
                .setView(iv)
                .create();
        dlg.show();
    }

    protected void showInputDlg(CharSequence title, CharSequence msg, final IDialogOnClick listener) {
        final ViewGroup vp = (ViewGroup) getLayoutInflater().inflate(R.layout.layout_fl_et, null);
        AlertDialog dlg = new AlertDialog.Builder(this).setTitle(title)
                .setView(vp)
                .setCancelable(true)
                .setNegativeButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        EditText et = vp.findViewById(R.id.et_content);
                        listener.onClick(dialog, which, new Object[]{et.getText().toString()});
                    }
                })
                .setMessage(msg)
                .create();
        dlg.show();
    }

    public interface IDialogOnClick {
        void onClick(DialogInterface dialog, int which, Object[] params);
    }

    /***********     broadcast          *********/
    public static final String BC_ID_BACKTOENTRY = "bc_id_backtoentry";
    protected  void onSetBroadCast(IntentFilter intentFilter) {
        intentFilter.addAction(BC_ID_BACKTOENTRY);
    }
    private void onLocalReceive(Context context, Intent intent, String action) {
        if(BC_ID_BACKTOENTRY.equals(action)) {//返回首页机制
            if(!getClass().getSimpleName().equals(EntryActivity.class.getSimpleName())) {
                finish();
            }
        }
    }


}

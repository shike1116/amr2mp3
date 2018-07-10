package com.research.amr2mp3;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity implements View.OnClickListener{

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findViewById(R.id.btn_2pcm).setOnClickListener(this);
        findViewById(R.id.btn_2mp3).setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {

        switch (v.getId()){
            case R.id.btn_2pcm:
                new Thread(){
                    @Override
                    public void run() {
                        FFmpegUtil.run("/storage/emulated/0/test/a1.amr","/storage/emulated/0/test/a2.pcm");
                    }
                }.start();

                break;
            case R.id.btn_2mp3:
                new Thread(){
                    @Override
                    public void run() {
                        LameUtil.run("/storage/emulated/0/test/a2.pcm", "/storage/emulated/0/test/a3.mp3");
                    }
                }.start();
                break;
            default:
                break;
        }
    }
}

package com.nokia.arplayer;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;

public class SplashActivity extends AppCompatActivity implements Runnable
{
    private static int SPLASH_DURATION = 1000;
    private Handler durationHandler = new Handler();

    @Override
    public void run() {
        Intent intent = new Intent(this, MainMenu.class);
        startActivity(intent);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_splash);

        durationHandler.postDelayed(this, SPLASH_DURATION);
    }

    @Override
    protected void onDestroy() {
        durationHandler.removeCallbacks(this);

        super.onDestroy();
    }
}

package com.nokia.arplayer;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.CompoundButton;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class MainMenu extends AppCompatActivity {
    private static final String TAG = "ARPlayer::Settings";

    public static final String CONTENT_ROOT_PATH = "/Movies/";

    public static final String MSG_CONTENT = "nokia.arplayer.CONTENT";
    public static final String MSG_ENABLE_ARCORE = "nokia.arplayer.ENABLE_ARCORE";
    public static final String MSG_ENABLE_DEBUG_MODE = "nokia.arplayer.ENABLE_DEBUG_MODE";
    public static final String MSG_ENABLE_DUAL_LAYER_MODE = "nokia.arplayer.ENABLE_DUAL_LAYER_MODE";
    public static final String MSG_ENABLE_MANUAL_TEXTURE_UPLOAD_MODE = "nokia.arplayer.ENABLE_MANUAL_TEXTURE_UPLOAD_MODE";

    private static final int APP_PERMISSIONS = 0;

    private static String[] mPermissions = new String[] {
        Manifest.permission.CAMERA,
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    private void putCommonParameters(Intent intent) {
        boolean enableAR = ((Switch) findViewById(R.id.switchAR)).isChecked();
        intent.putExtra(MSG_ENABLE_ARCORE, enableAR);
        Log.d(TAG, enableAR ? "AR enabled" : "AR disabled");

        boolean enabledDebugMode = ((Switch) findViewById(R.id.switchDebugMode)).isChecked();
        intent.putExtra(MSG_ENABLE_DEBUG_MODE, enabledDebugMode);
        Log.d(TAG, enabledDebugMode ? "Debug Mode enabled" : "Debug Mode disabled");

        boolean enabledDualLayerMode = ((Switch) findViewById(R.id.switchDualLayerMode)).isChecked();
        intent.putExtra(MSG_ENABLE_DUAL_LAYER_MODE, enabledDualLayerMode);
        Log.d(TAG, enabledDualLayerMode ? "Dual Layer Mode enabled" : "Dual Layer Mode disabled");

        boolean enabledManualTextureUploadMode = ((Switch) findViewById(R.id.switchManualTextureUploadMode)).isChecked();
        intent.putExtra(MSG_ENABLE_MANUAL_TEXTURE_UPLOAD_MODE, enabledManualTextureUploadMode);
        Log.d(TAG, enabledManualTextureUploadMode ? "Manual Texture Upload Mode enabled" : "Manual Texture Upload Mode disabled");

        try
        {
            String content = CONTENT_ROOT_PATH + ((Spinner) findViewById(R.id.spinner)).getSelectedItem().toString();
            intent.putExtra(MSG_CONTENT, content);
            Log.d(TAG, "Content " + content);
        }
        catch (Exception e)
        {

        }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main_menu);

        requestPermissions(mPermissions, APP_PERMISSIONS);

        updateSpinner();

        Switch enabledManualTextureUpload = (Switch)findViewById(R.id.switchManualTextureUploadMode);

        enabledManualTextureUpload.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Switch enabledDualLayer = (Switch)findViewById(R.id.switchDualLayerMode);
                enabledDualLayer.setChecked(false);
            }
        });
    }

    public void updateSpinner() {
        List<String> allNames = new ArrayList<>();

        File storageRoot = Environment.getExternalStorageDirectory();
        File dir = new File(storageRoot, CONTENT_ROOT_PATH);

        for (File f : dir.listFiles()) {
            if (f.isFile()) {
                String filename = f.getName();

                if (filename.contains(".bin")) {
                    allNames.add(filename);

                    Log.d(TAG, filename);
                }
            }
        }

        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(this, android.R.layout.simple_spinner_item, allNames);
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);

        Spinner spinner = (Spinner) findViewById(R.id.spinner);
        spinner.setAdapter(dataAdapter);
    }

    public void onStartPlaybackClick(View v) {
        Intent intent = new Intent(this, ARPlayerActivity.class);
        putCommonParameters(intent);

        startActivity(intent);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        Log.d(TAG, String.format("onRequestPermissionsResult %d", requestCode));

        for (int i = 0; i < permissions.length; i++) {
            Log.d(TAG, String.format("%s %d", permissions[i], results[i]));

            if (results[i] != PackageManager.PERMISSION_GRANTED) {
                requestPermissions(mPermissions, APP_PERMISSIONS);
            }
        }
    }
}

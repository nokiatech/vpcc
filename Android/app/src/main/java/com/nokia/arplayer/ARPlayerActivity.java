package com.nokia.arplayer;

import android.content.Intent;
import android.hardware.display.DisplayManager;

// import android.opengl.GLES20;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Build;

import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;

import android.util.Log;

import android.view.GestureDetector;
import android.view.ScaleGestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;

import android.widget.Toast;

import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaCodecInfo.CodecCapabilities;
import android.media.MediaCodecInfo.CodecProfileLevel;

import java.io.File;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class ARPlayerActivity
       extends AppCompatActivity
       implements GLSurfaceView.Renderer, DisplayManager.DisplayListener
{
  private static final String TAG = ARPlayerActivity.class.getSimpleName();

  private GLSurfaceView surfaceView;

  private boolean viewportChanged = false;
  private int viewportWidth;
  private int viewportHeight;

  private long nativeApplication;

  private GestureDetector gestureDetector;
  private ScaleGestureDetector scaleDetector;

  private float scaleFactor = 1.0f;

  private boolean enableAR = false;
  private boolean enableDebugMode = false;
  private boolean enableDualLayerMode = false;
  private boolean enableManualTextureUploadMode = false;

  private String videoFilename = "";

    static MediaCodecInfo[] getCodecs() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            MediaCodecList mediaCodecList = new MediaCodecList(MediaCodecList.ALL_CODECS);
            return mediaCodecList.getCodecInfos();
        } else {
            int numCodecs = MediaCodecList.getCodecCount();
            MediaCodecInfo[] mediaCodecInfo = new MediaCodecInfo[numCodecs];

            for (int i = 0; i < numCodecs; i++) {
                MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
                mediaCodecInfo[i] = codecInfo;
            }

            return mediaCodecInfo;
        }
    }

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);

    setContentView(R.layout.activity_main);
    surfaceView = (GLSurfaceView) findViewById(R.id.surfaceview);

    // Read passed parameters.
    Intent intent = getIntent();
    enableAR = intent.getBooleanExtra(MainMenu.MSG_ENABLE_ARCORE, false);
    enableDebugMode = intent.getBooleanExtra(MainMenu.MSG_ENABLE_DEBUG_MODE, false);
    enableDualLayerMode = intent.getBooleanExtra(MainMenu.MSG_ENABLE_DUAL_LAYER_MODE, false);
    enableManualTextureUploadMode = intent.getBooleanExtra(MainMenu.MSG_ENABLE_MANUAL_TEXTURE_UPLOAD_MODE, false);
    videoFilename = intent.getStringExtra(MainMenu.MSG_CONTENT);

    // Set up gesture listeners.
    scaleDetector = new ScaleGestureDetector(this,
            new ScaleGestureDetector.SimpleOnScaleGestureListener() {
            @Override
            public boolean onScale(ScaleGestureDetector detector) {
                scaleFactor *= detector.getScaleFactor();

                scaleFactor = Math.max(0.1f, Math.min(scaleFactor, 5.0f));
                surfaceView.queueEvent(() -> JniInterface.onScale(nativeApplication, scaleFactor));

                return true;
            }
        });

    gestureDetector =
        new GestureDetector(
            this,
            new GestureDetector.SimpleOnGestureListener() {
              @Override
              public boolean onSingleTapUp(final MotionEvent e) {
                if (scaleDetector.isInProgress()) {
                    return false;
                }

                surfaceView.queueEvent(() -> JniInterface.onSingleTap(nativeApplication, e.getX(), e.getY()));

                return true;
              }

                @Override
                public boolean onDoubleTap(final MotionEvent e) {
                    if (scaleDetector.isInProgress()) {
                        return false;
                    }

                    surfaceView.queueEvent(() -> JniInterface.onDoubleTap(nativeApplication, e.getX(), e.getY()));

                    return true;
                }

              @Override
              public boolean onDown(MotionEvent e)
              {
                if (scaleDetector.isInProgress()) {
                    return false;
                }

                return true;
              }
            });

    surfaceView.setOnTouchListener(
        (View v, MotionEvent event) -> gestureDetector.onTouchEvent(event));

    // Set up renderer.
    surfaceView.setPreserveEGLContextOnPause(true);
    surfaceView.setEGLContextClientVersion(3); // 2
    surfaceView.setEGLConfigChooser(8, 8, 8, 8, 16, 0); // Alpha used for plane blending.
    surfaceView.setRenderer(this);
    surfaceView.setRenderMode(GLSurfaceView.RENDERMODE_CONTINUOUSLY);

    String internalStoragePath = getFilesDir().getAbsolutePath();
    JniInterface.assetManager = getAssets();

    File storageRoot = Environment.getExternalStorageDirectory();
    File file = new File(storageRoot, videoFilename);
    String filename = file.getAbsolutePath();

    nativeApplication = JniInterface.createNativeApplication(getAssets(), internalStoragePath, filename, enableAR, enableDebugMode, enableDualLayerMode);
  }

  @Override
  protected void onResume() {
    super.onResume();
    // ARCore requires camera permissions to operate. If we did not yet obtain runtime
    // permission on Android M and above, now is a good time to ask the user for it.
    if (!CameraPermissionHelper.hasCameraPermission(this)) {
      CameraPermissionHelper.requestCameraPermission(this);
      return;
    }

    JniInterface.onResume(nativeApplication, getApplicationContext(), this);
    surfaceView.onResume();

    // Listen to display changed events to detect 180° rotation, which does not cause a config change or view resize.
    getSystemService(DisplayManager.class).registerDisplayListener(this, null);
  }

  @Override
  public void onPause() {
    super.onPause();
    surfaceView.onPause();
    JniInterface.onPause(nativeApplication);

    getSystemService(DisplayManager.class).unregisterDisplayListener(this);
  }

  @Override
  public void onDestroy() {
    super.onDestroy();

    // Synchronized to avoid racing onDrawFrame.
    synchronized (this) {
      JniInterface.destroyNativeApplication(nativeApplication);
      nativeApplication = 0;
    }
  }

  @Override
  public void onWindowFocusChanged(boolean hasFocus) {
    super.onWindowFocusChanged(hasFocus);
    if (hasFocus) {
      // Standard Android full-screen functionality.
      getWindow()
          .getDecorView()
          .setSystemUiVisibility(
              View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                  | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                  | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                  | View.SYSTEM_UI_FLAG_FULLSCREEN
                  | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
      getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
    }
  }

  @Override
  public void onSurfaceCreated(GL10 gl, EGLConfig config) {
    GLES30.glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    JniInterface.onGlSurfaceCreated(nativeApplication);
  }

  @Override
  public void onSurfaceChanged(GL10 gl, int width, int height) {
    viewportWidth = width;
    viewportHeight = height;
    viewportChanged = true;
  }

  @Override
  public void onDrawFrame(GL10 gl) {
    // Synchronized to avoid racing onDestroy.
    synchronized (this) {
      if (nativeApplication == 0) {
        return;
      }

      if (viewportChanged) {
        int displayRotation = getWindowManager().getDefaultDisplay().getRotation();
        JniInterface.onDisplayGeometryChanged(nativeApplication, displayRotation, viewportWidth, viewportHeight);
        viewportChanged = false;
      }

      JniInterface.onGlSurfaceDrawFrame(nativeApplication);
    }
  }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] results) {
        if (!CameraPermissionHelper.hasCameraPermission(this)) {
            Toast.makeText(this, "Camera permission is needed to run this application", Toast.LENGTH_LONG).show();

            if (!CameraPermissionHelper.shouldShowRequestPermissionRationale(this)) {
                // Permission denied with checking "Do not ask again".
                CameraPermissionHelper.launchPermissionSettings(this);
            }

            finish();
        }
    }

    // DisplayListener methods.
    @Override
    public void onDisplayAdded(int displayId) {
    }

    @Override
    public void onDisplayRemoved(int displayId) {
    }

    @Override
    public void onDisplayChanged(int displayId) {
        viewportChanged = true;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        boolean retVal = scaleDetector.onTouchEvent(event);
        retVal = gestureDetector.onTouchEvent(event) || retVal;

        return retVal || super.onTouchEvent(event);
    }
}

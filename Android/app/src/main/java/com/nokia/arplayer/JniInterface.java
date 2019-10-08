package com.nokia.arplayer;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;

public class JniInterface {
  static {
    System.loadLibrary("arplayer_native");
  }

  private static final String TAG = "JniInterface";
  static AssetManager assetManager;

  public static native long createNativeApplication(AssetManager assetManager, String internalStoragePath, String videoFilename, boolean enableAR, boolean enableDebugMode, boolean enableDualLayerMode);
  public static native void destroyNativeApplication(long nativeApplication);

  public static native void onPause(long nativeApplication);
  public static native void onResume(long nativeApplication, Context context, Activity activity);
  public static native void onGlSurfaceCreated(long nativeApplication);
  public static native void onDisplayGeometryChanged(long nativeApplication, int displayRotation, int width, int height);
  public static native void onGlSurfaceDrawFrame(long nativeApplication);
  public static native void onSingleTap(long nativeApplication, float x, float y);
  public static native void onScale(long nativeApplication, float s);
  public static native void onDoubleTap(long nativeApplication, float x, float y);
}

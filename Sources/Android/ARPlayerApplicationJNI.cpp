/**
* This file is part of Nokia VPCC implementation
*
* Copyright (c) 2019-2020 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
*
* Contact: VPCC.contact@nokia.com
*
* This software, including documentation, is protected by copyright controlled by Nokia Corporation and/ or its
* subsidiaries. All rights are reserved.
*
* Copying, including reproducing, storing, adapting or translating, any or all of this material requires the prior
* written consent of Nokia.
*/

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <jni.h>

#include "ARPlayerApplication.h"
#include "FileSystem.h"

#define JNI_METHOD(return_type, method_name)            \
    JNIEXPORT return_type JNICALL                       \
    Java_com_nokia_arplayer_JniInterface_##method_name

extern "C"
{
    namespace
    {
        inline jlong jptr(ARPlayerApplication *native_ar_application)
        {
            return reinterpret_cast<intptr_t>(native_ar_application);
        }

        inline ARPlayerApplication *native(jlong ptr)
        {
            return reinterpret_cast<ARPlayerApplication *>(ptr);
        }
    }

    JNI_METHOD(jlong, createNativeApplication)
    (JNIEnv *env, jclass, jobject j_asset_manager, jstring j_internal_storage_path, jstring j_video_filename, jboolean j_enable_ar, jboolean j_enable_debug_mode, jboolean j_enable_dual_layer_mode)
    {
        // Initialize filesystem
        AAssetManager *asset_manager = AAssetManager_fromJava(env, j_asset_manager);
        const char *internal_storage_path = env->GetStringUTFChars(j_internal_storage_path, 0);

        FileSystem::Config config;
        config.assetManager = asset_manager;
        config.internalStoragePath = internal_storage_path;

        FileSystem::initialize(config);

        // Initialize application
        const char *video_filename = env->GetStringUTFChars(j_video_filename, 0);
        bool enable_ar = (bool)j_enable_ar;
        bool enable_debug_mode = (bool)j_enable_debug_mode;
        bool enable_dual_layer_mode = (bool)j_enable_dual_layer_mode;

        ARPlayerApplication::Settings settings;
        settings.filename = video_filename;
        settings.enableAR = enable_ar;
        settings.enableDebugMode = enable_debug_mode;
        settings.enableDualLayerMode = enable_dual_layer_mode;

        return jptr(new ARPlayerApplication(settings));
    }

    JNI_METHOD(void, destroyNativeApplication)
    (JNIEnv *, jclass, jlong native_application)
    {
        delete native(native_application);

        FileSystem::shutdown();
    }

    JNI_METHOD(void, onPause)
    (JNIEnv *, jclass, jlong native_application)
    {
        native(native_application)->onPause();
    }

    JNI_METHOD(void, onResume)
    (JNIEnv *env, jclass, jlong native_application, jobject context, jobject activity)
    {
        native(native_application)->onResume(env, context, activity);
    }

    JNI_METHOD(void, onGlSurfaceCreated)
    (JNIEnv *, jclass, jlong native_application)
    {
        native(native_application)->onSurfaceCreated();
    }

    JNI_METHOD(void, onDisplayGeometryChanged)
    (JNIEnv *, jclass, jlong native_application, int display_rotation, int width, int height)
    {
        native(native_application)->onDisplayGeometryChanged(display_rotation, width, height);
    }

    JNI_METHOD(void, onGlSurfaceDrawFrame)
    (JNIEnv *, jclass, jlong native_application)
    {
        native(native_application)->onDrawFrame();
    }

    JNI_METHOD(void, onSingleTap)
    (JNIEnv *, jclass, jlong native_application, jfloat x, jfloat y)
    {
        native(native_application)->onSingleTap(x, y);
    }

    JNI_METHOD(void, onDoubleTap)
    (JNIEnv *, jclass, jlong native_application, jfloat x, jfloat y)
    {
        native(native_application)->onDoubleTap(x, y);
    }

    JNI_METHOD(void, onScale)
    (JNIEnv *, jclass, jlong native_application, jfloat s)
    {
        native(native_application)->onScale(s);
    }
}

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

#include "Android/JNIInterface.h"

#include <assert.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <jni.h>

extern "C"
{
    namespace
    {
        static JavaVM* _vm = NULL;
    }

    jint JNI_OnLoad(JavaVM* vm, void*)
    {
        _vm = vm;

        return JNI_VERSION_1_6;
    }

    JNIEnv* getJNIEnv()
    {
        assert(_vm != NULL);

        JNIEnv* env = NULL;

        int result = _vm->GetEnv((void**)&env, JNI_VERSION_1_6);

        if (result != JNI_OK)
        {
            assert(false);
        }

        return env;
    }

    JavaVM* getJavaVM()
    {
        return _vm;
    }

    jclass findClass(const char* className)
    {
        JNIEnv* env = getJNIEnv();

        if (env)
        {
            return env->FindClass(className);
        }

        return NULL;
    }

    void attachThread()
    {
        assert(_vm != NULL);

        JNIEnv* env = NULL;

        int result = _vm->GetEnv((void**)&env, JNI_VERSION_1_6);

        if (result != JNI_OK)
        {
            result = _vm->AttachCurrentThread(&env, NULL);

            return;
        }

        assert(false);
    }

    void detachThread()
    {
        assert(_vm != NULL);

        _vm->DetachCurrentThread();
    }
}

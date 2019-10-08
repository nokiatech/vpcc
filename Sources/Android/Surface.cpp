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

#include "Android/Surface.h"
#include "Android/JNIInterface.h"

Surface::Surface()
: _env(NULL)
, _object(NULL)
{
    _env = getJNIEnv();
    _surfaceTexture = new SurfaceTexture();

    // Find class
    const char *className = "android/view/Surface";
    const jclass surfaceClass = _env->FindClass(className);

    if (surfaceClass == 0)
    {
        assert(false);
    }

    // Find constructor
    const jmethodID constructor = _env->GetMethodID(surfaceClass, "<init>", "(Landroid/graphics/SurfaceTexture;)V");

    if (constructor == 0)
    {
        assert(false);
    }

    // Create object
    jobject surfaceTextureObject = _surfaceTexture->getJavaObject();
    jobject object = _env->NewObject(surfaceClass, constructor, surfaceTextureObject);

    if (object == 0)
    {
        assert(false);
    }

    // Create global ref
    jobject javaObject = _env->NewGlobalRef(object);

    if (javaObject == 0)
    {
        assert(false);
    }

    _object = javaObject;

    _env->DeleteLocalRef(object);
    _env->DeleteLocalRef(surfaceClass);
}

Surface::~Surface()
{
    delete _surfaceTexture;
    _surfaceTexture = NULL;

    if (_object != NULL)
    {
        // _env->DeleteGlobalRef(_object); // TODO
        _object = NULL;
    }
}

SurfaceTexture* Surface::getSurfaceTexture()
{
    return _surfaceTexture;
}

jobject Surface::getJavaObject()
{
    return _object;
}

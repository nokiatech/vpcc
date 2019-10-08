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

#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>

#include <jni.h>
#include <android/log.h>

#include "GraphicsAPI.h"

class SurfaceTexture
{
public:

    SurfaceTexture();
    ~SurfaceTexture();

    bool updateTextImage();

    const float* getTransformMatrix();
    long long getNanoTimestamp();
    GLuint getTexture();
    jobject	getJavaObject();

private:

    JNIEnv* _env = NULL;
    jobject	_object;

    GLuint _texture = 0;
    long long _nanoTimestamp;
    float _transformMatrix[16];

    jmethodID _updateTexImageMethodId;
    jmethodID _getTimestampMethodId;
    jmethodID _getTransformMatrixId;
};

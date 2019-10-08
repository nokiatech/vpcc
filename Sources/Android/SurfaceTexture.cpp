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

#include "Android/SurfaceTexture.h"
#include "Android/JNIInterface.h"

SurfaceTexture::SurfaceTexture()
: _env(NULL)
, _object(NULL)
, _texture(0)
, _nanoTimestamp(0)
, _updateTexImageMethodId(NULL)
, _getTimestampMethodId(NULL)
, _getTransformMatrixId(NULL)
{
    _env = getJNIEnv();

    GLint currentTexture = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_EXTERNAL_OES, &currentTexture);

    // Create texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _texture);

    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_EXTERNAL_OES, currentTexture);

    // Find class
    const char *className = "android/graphics/SurfaceTexture";
    const jclass surfaceTextureClass = _env->FindClass(className);

    if (surfaceTextureClass == 0)
    {
        assert(false);
    }

    // Find constructor
    const jmethodID constructor = _env->GetMethodID(surfaceTextureClass, "<init>", "(I)V");

    if (constructor == 0)
    {
        assert(false);
    }

    // Create object
    jobject object = _env->NewObject(surfaceTextureClass, constructor, _texture);

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

    // Get updateTexImage method id
    jmethodID updateTexImageMethodId = _env->GetMethodID(surfaceTextureClass, "updateTexImage", "()V");

    if (!updateTexImageMethodId)
    {
        assert(false);
    }

    _updateTexImageMethodId = updateTexImageMethodId;

    // Get getTimestamp method id
    jmethodID getTimestampMethodId = _env->GetMethodID(surfaceTextureClass, "getTimestamp", "()J");

    if (!getTimestampMethodId)
    {
        assert(false);
    }

    _getTimestampMethodId = getTimestampMethodId;

    // Create getTransformMatrix method id
    jmethodID getTransformMatrixId = _env->GetMethodID(surfaceTextureClass, "getTransformMatrix", "([F)V");

    if (!getTransformMatrixId)
    {
        assert(false);
    }

    _getTransformMatrixId = getTransformMatrixId;

    _object = javaObject;

    _env->DeleteLocalRef(object);
    _env->DeleteLocalRef(surfaceTextureClass);
}

SurfaceTexture::~SurfaceTexture()
{
    if (_texture != 0)
    {
        glDeleteTextures(1, &_texture);
        _texture = 0;
    }

    if (_object != NULL)
    {
        // _env->DeleteGlobalRef(_object); // TODO
        _object = NULL;
    }

    _env = NULL;
}

bool SurfaceTexture::updateTextImage()
{
    assert(_env != NULL);
    assert(_object != NULL);

    // Call updateTextImage
    _env->CallVoidMethod(_object, _updateTexImageMethodId);

    // Call getTimestamp
    _nanoTimestamp = _env->CallLongMethod(_object, _getTimestampMethodId);

    // Call getTransformMatrix
    jfloatArray jarray = _env->NewFloatArray(16);

    _env->CallVoidMethod(_object, _getTransformMatrixId, jarray);

    jfloat* array = _env->GetFloatArrayElements(jarray, NULL);

    _transformMatrix[0] = array[0];
    _transformMatrix[1] = array[1];
    _transformMatrix[2] = array[2];
    _transformMatrix[3] = array[3];

    _transformMatrix[4] = array[4];
    _transformMatrix[5] = array[5];
    _transformMatrix[6] = array[6];
    _transformMatrix[7] = array[7];

    _transformMatrix[8] = array[8];
    _transformMatrix[9] = array[9];
    _transformMatrix[10] = array[10];
    _transformMatrix[11] = array[11];

    _transformMatrix[12] = array[12];
    _transformMatrix[13] = array[13];
    _transformMatrix[14] = array[14];
    _transformMatrix[15] = array[15];

    _env->ReleaseFloatArrayElements(jarray, array, JNI_COMMIT);

    _env->DeleteLocalRef(jarray);

    return true;
}

GLuint SurfaceTexture::getTexture()
{
    return _texture;
}

const float* SurfaceTexture::getTransformMatrix()
{
    return _transformMatrix;
}

long long SurfaceTexture::getNanoTimestamp()
{
    return _nanoTimestamp;
}

jobject SurfaceTexture::getJavaObject()
{
    return _object;
}

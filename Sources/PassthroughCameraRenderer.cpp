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

#include "PassthroughCameraRenderer.h"

#include "Logger.h"

PassthroughCameraRenderer::PassthroughCameraRenderer()
{
}

PassthroughCameraRenderer::~PassthroughCameraRenderer()
{
}

void PassthroughCameraRenderer::create()
{
#if PLATFORM_ANDROID

    char kVertexShaderFilename[] = "Assets/Shaders/passthroughCamera.vert";
    char kFragmentShaderFilename[] = "Assets/Shaders/passthroughCamera.frag";

    _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename);

    if (!_shaderProgram)
    {
        LOG_E("Could not create program.");
    }

    glGenTextures(1, &_textureHandle);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureHandle);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _attributeVertices = glGetAttribLocation(_shaderProgram, "a_position");
    _attributeTexcoords = glGetAttribLocation(_shaderProgram, "a_tex_coord");

    _uniformTexture = glGetUniformLocation(_shaderProgram, "u_texture");

#endif
}

void PassthroughCameraRenderer::destroy()
{
#if PLATFORM_ANDROID

    glDeleteProgram(_shaderProgram);
    glDeleteTextures(1, &_textureHandle);

    GL_CHECK_ERRORS();

#endif
}

void PassthroughCameraRenderer::draw()
{
#if PLATFORM_ANDROID

    pushDebugMarker("PassthroughCameraRenderer");

    // Note: Only fixed portrait orientation
    const GLfloat kVertices[] =
    {
        -1.0f, -1.0f, 0.0f,
        +1.0f, -1.0f, 0.0f,
        -1.0f, +1.0f, 0.0f,
        +1.0f, +1.0f, 0.0f,
    };

    const GLfloat kTexcoords[] =
    {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
    };

    glUseProgram(_shaderProgram);
    glDepthMask(GL_FALSE);

    glUniform1i(_uniformTexture, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, _textureHandle);

    glEnableVertexAttribArray(_attributeVertices);
    glVertexAttribPointer(_attributeVertices, 3, GL_FLOAT, GL_FALSE, 0, kVertices);

    glEnableVertexAttribArray(_attributeTexcoords);
    glVertexAttribPointer(_attributeTexcoords, 2, GL_FLOAT, GL_FALSE, 0, kTexcoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Cleanup
    glDisableVertexAttribArray(_attributeVertices);
    glDisableVertexAttribArray(_attributeTexcoords);

    glUseProgram(0);

    glDepthMask(GL_TRUE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    popDebugMarker();

    GL_CHECK_ERRORS();

#endif
}

GLuint PassthroughCameraRenderer::getCameraTextureHandle() const
{
    return _textureHandle;
}

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

#include "glm.h"

#include "GraphicsAPI.h"

class PassthroughCameraRenderer
{
public:

    PassthroughCameraRenderer();
    ~PassthroughCameraRenderer();

    void create();
    void destroy();

    void draw();

    GLuint getCameraTextureHandle() const;

private:

    GLuint _shaderProgram = 0;
    GLuint _textureHandle = 0;

    GLuint _attributeVertices = 0;
    GLuint _attributeTexcoords = 0;

    GLuint _uniformTexture = 0;
};

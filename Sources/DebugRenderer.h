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
#include "Texture2D.h"

class DebugRenderer
{
public:

    struct Type
    {
        enum Enum
        {
            INVALID = 0,
            DEBUG_COLOR = 3,
            DEBUG_DEPTH = 4,
            DEBUG_OCCUPANCY = 5,
        };
    };

public:

    DebugRenderer();
    ~DebugRenderer();

    void create(Type::Enum type, bool manualVideoTextureUpload);
    void destroy();

    void draw(Texture2D& textureY,
              Texture2D& textureUV,
              glm::vec2 position,
              glm::vec2 size = glm::vec2(1.0, 1.0));

private:

    GLuint _vbo = 0;
    GLuint _attributeVertices = 0;

    GLuint _shaderProgram = 0;

    GLuint _uniformTextureY = 0;
    GLuint _uniformTextureUV = 0;
    GLuint _uniformModel = 0;
    GLuint _uniformProjection = 0;
};

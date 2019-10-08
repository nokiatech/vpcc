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

class QuadRenderer
{
public:

    QuadRenderer();
    ~QuadRenderer();

    void create(Texture2D::Type::Enum type, bool manualVideoTextureUpload);
    void destroy();


    void draw(Texture2D& texture,
              glm::vec2 position,
              glm::vec2 size = glm::vec2(1.0, 1.0),
              GLfloat rotate = 0.0f,
              glm::vec4 color = glm::vec4(1.0f));

private:

    GLuint _vbo = 0;
    GLuint _attributeVertices = 0;

    GLuint _shaderProgram = 0;

    GLuint _uniformTexture0 = 0;
    GLuint _uniformModel = 0;
    GLuint _uniformProjection = 0;
    GLuint _uniformTintColor = 0;
};

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

#include <vector>

#include "glm.h"

#include "GraphicsAPI.h"
#include "Texture2D.h"

class DebugTextRenderer
{
public:

    DebugTextRenderer();
    ~DebugTextRenderer();

    void create();
    void destroy();

    void printFormat(glm::vec2 position, glm::vec4 textColor, glm::vec4 backgroundColor, const char* format, ...);
    void printFormatV(glm::vec2 position, glm::vec4 textColor, glm::vec4 backgroundColor, const char* format, va_list args);

private:

    struct GlyphVertex
    {
        float x;
        float y;
        float u;
        float v;
        glm::vec4 textColor;
        glm::vec4 backgroundColor;
    };

    static const size_t MAX_CHARACTERS = 2048;
    char _textBuffer[MAX_CHARACTERS] = { 0 };

    static const size_t MAX_VERTICES = 6 * MAX_CHARACTERS;
    std::vector<GlyphVertex> _vertices;

    GLuint _vbo = 0;
    GLuint _texture = 0;
    GLuint _attributePositions = 0;
    GLuint _attributeTexCoords = 0;
    GLuint _attributeTextColors = 0;
    GLuint _attributeBackgroundColors = 0;

    GLuint _shaderProgram = 0;

    GLuint _uniformTexture0 = 0;
    GLuint _uniformProjection = 0;
};

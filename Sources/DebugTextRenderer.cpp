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

#include "DebugTextRenderer.h"

#include "Logger.h"
#include "DebugCharacterSet.h"

namespace
{
    static const uint16_t glyphTexelWidth = 8;
    static const uint16_t glyphTexelHeight = 16;

    static const uint16_t glyphScreenWidth = 8;
    static const uint16_t glyphScreenHeight = 16;

    static const uint16_t fontAtlasWidth = 2048;
    static const uint16_t fontAtlasHeight = 16;
}

DebugTextRenderer::DebugTextRenderer()
{
}

DebugTextRenderer::~DebugTextRenderer()
{
}

void DebugTextRenderer::create()
{
    // Create shader
    char kVertexShaderFilename[] = "Assets/Shaders/debugText.vert";
    char kFragmentShaderFilename[] = "Assets/Shaders/debugText.frag";

    _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename);

    if (!_shaderProgram)
    {
        LOG_E("Could not create program.");
    }

    _attributePositions = glGetAttribLocation(_shaderProgram, "a_position");
    _attributeTexCoords = glGetAttribLocation(_shaderProgram, "a_tex_coord");
    _attributeTextColors = glGetAttribLocation(_shaderProgram, "a_text_color");
    _attributeBackgroundColors = glGetAttribLocation(_shaderProgram, "a_background_color");

    _uniformTexture0 = glGetUniformLocation(_shaderProgram, "u_texture0");
    _uniformProjection = glGetUniformLocation(_shaderProgram, "u_projection");

    // Create VBO
    _vertices.reserve(MAX_VERTICES);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GlyphVertex) * MAX_VERTICES, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create texture
    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, fontAtlasWidth, fontAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, vga8x16);
    glBindTexture(GL_TEXTURE_2D, 0);

    GL_CHECK_ERRORS();
}

void DebugTextRenderer::destroy()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteTextures(1, &_texture);
    glDeleteProgram(_shaderProgram);

    GL_CHECK_ERRORS();
}

void DebugTextRenderer::printFormat(glm::vec2 position, glm::vec4 textColor, glm::vec4 backgroundColor, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    printFormatV(position, textColor, backgroundColor, format, args);

    va_end(args);
}

void DebugTextRenderer::printFormatV(glm::vec2 position, glm::vec4 textColor, glm::vec4 backgroundColor, const char* format, va_list args)
{
    pushDebugMarker("DebugTextRenderer");

    va_list copy;
    va_copy(copy, args);

    // Calculate glyph positions
    size_t stringLength = ::vsnprintf(_textBuffer, MAX_CHARACTERS, format, args);

    const float startPositionX = (float)(position.x * glyphScreenWidth);
    const float startPositionY = (float)(position.y * glyphScreenHeight);

    float positionX = startPositionX;
    float positionY = startPositionY;

    const float horizontalTabSize = (float)(glyphScreenWidth * 4);
    const float verticalTabSize = (float)(glyphScreenHeight * 4);

    const float lineHeight = (float)glyphScreenHeight;

    // Prepare transformations
    GLint viewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);

    int32_t width = viewport[2];
    int32_t height = viewport[3];

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

    glm::mat4 projection = glm::ortho(0.0f,
                                      (GLfloat)width,
                                      (GLfloat)height,
                                      0.0f,
                                      -1.0f,
                                      1.0f);

    glm::mat4 mvp = projection * scale;

    // Bind shader
    glUseProgram(_shaderProgram);
    glDepthMask(GL_FALSE);

    // Bind texture
    glUniform1i(_uniformTexture0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture);

    // Set shader constants
    glUniformMatrix4fv(_uniformProjection, 1, GL_FALSE, glm::value_ptr(mvp));

    for (size_t i = 0; i < stringLength; ++i)
    {
        char character = _textBuffer[i];

        // Handle control characters
        if (character == '\t')
        {
            positionX += horizontalTabSize;

            continue;
        }

        if (character == '\n')
        {
            positionY += lineHeight;
            positionX = startPositionX;

            continue;
        }

        if (character == '\v')
        {
            positionY += verticalTabSize;
            positionX = startPositionX;

            continue;
        }

        uint16_t glyphIndex = _textBuffer[i];

        // Skip control codes
        if (glyphIndex < 32)
        {
            continue;
        }

        float texelWidth = 1.0f / (float)fontAtlasWidth;
        float texelHeight = 1.0f / (float)fontAtlasHeight;

        bool enableTexelOffset = true;

        float halfTexelWidth = enableTexelOffset ? 1.0f * texelWidth : 0.0f;
        float halfTexelHeight = enableTexelOffset ? 1.0f * texelHeight : 0.0f;

        float left = ((float)glyphIndex * glyphTexelWidth * texelWidth) - halfTexelWidth;
        float right = ((float)(glyphIndex + 1) * glyphTexelWidth * texelWidth) - halfTexelWidth;
        float bottom = 0.0f - halfTexelHeight;
        float top = 1.0f - halfTexelHeight;

        // a c
        // b d
        GlyphVertex glyph[6] =
        {
            { (positionX + 0.0f),                        (positionY + 0.0f),                         left, top,     textColor, backgroundColor }, // a
            { (positionX + 0.0f),                        (positionY + (float)glyphScreenHeight),     left, bottom,  textColor, backgroundColor }, // b
            { (positionX + (float)glyphScreenWidth),     (positionY + 0.0f),                         right, top,    textColor, backgroundColor }, // c

            { (positionX + (float)glyphScreenWidth),     (positionY + 0.0f),                         right, top,    textColor, backgroundColor }, // c
            { (positionX + 0.0f),                        (positionY + (float)glyphScreenHeight),     left, bottom,  textColor, backgroundColor }, // b
            { (positionX + (float)glyphScreenWidth),     (positionY + (float)glyphScreenHeight),     right, bottom, textColor, backgroundColor }, // d
        };

        _vertices.push_back(glyph[0]);
        _vertices.push_back(glyph[1]);
        _vertices.push_back(glyph[2]);
        _vertices.push_back(glyph[3]);
        _vertices.push_back(glyph[4]);
        _vertices.push_back(glyph[5]);

        positionX += (float)glyphScreenWidth;

        // Check if stream cannot handle next glyph and submit draw call
        if (_vertices.capacity() - _vertices.size() < 6)
        {
            // Update buffer
            glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(GlyphVertex) * _vertices.size(), (const GLvoid*)_vertices.data(), GL_DYNAMIC_DRAW);

            glEnableVertexAttribArray(_attributePositions);
            glVertexAttribPointer(_attributePositions, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(0)); // 4 * 2 = 8

            glEnableVertexAttribArray(_attributeTexCoords);
            glVertexAttribPointer(_attributeTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(8)); // 4 * 2 = 8

            glEnableVertexAttribArray(_attributeTextColors);
            glVertexAttribPointer(_attributeTextColors, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(16)); // 4 * 4 = 16

            glEnableVertexAttribArray(_attributeBackgroundColors);
            glVertexAttribPointer(_attributeBackgroundColors, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(32)); // 4 * 4 = 16

            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)_vertices.size());

            _vertices.clear();
        }
    }

    if (_vertices.size() > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GlyphVertex) * _vertices.size(), (const GLvoid*)_vertices.data(), GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(_attributePositions);
        glVertexAttribPointer(_attributePositions, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(0)); // 4 * 2 = 8

        glEnableVertexAttribArray(_attributeTexCoords);
        glVertexAttribPointer(_attributeTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(8)); // 4 * 2 = 8

        glEnableVertexAttribArray(_attributeTextColors);
        glVertexAttribPointer(_attributeTextColors, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(16)); // 4 * 4 = 16

        glEnableVertexAttribArray(_attributeBackgroundColors);
        glVertexAttribPointer(_attributeBackgroundColors, 4, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), GL_BUFFER_OFFSET(32)); // 4 * 4 = 16

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)_vertices.size());

        _vertices.clear();
    }

    // Cleanup
    glDisableVertexAttribArray(_attributePositions);
    glDisableVertexAttribArray(_attributeTexCoords);
    glDisableVertexAttribArray(_attributeTextColors);
    glDisableVertexAttribArray(_attributeBackgroundColors);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);

    glDepthMask(GL_TRUE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    popDebugMarker();

    GL_CHECK_ERRORS();

    va_end(copy);
}

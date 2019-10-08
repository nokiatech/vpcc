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

#include "DebugRenderer.h"

#include "Logger.h"

DebugRenderer::DebugRenderer()
{
}

DebugRenderer::~DebugRenderer()
{
}

void DebugRenderer::create(Type::Enum type, bool manualVideoTextureUpload)
{
    // Create shader
    if (type == Type::DEBUG_COLOR)
    {
        char kVertexShaderFilename[] = "Assets/Shaders/debugVideo.vert";
        char kFragmentShaderFilename[] = "Assets/Shaders/debugVideoColor.frag";

        _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload);
    }
    else if (type == Type::DEBUG_DEPTH)
    {
        char kVertexShaderFilename[] = "Assets/Shaders/debugVideo.vert";
        char kFragmentShaderFilename[] = "Assets/Shaders/debugVideoDepth.frag";

        _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload);
    }
    else if (type == Type::DEBUG_OCCUPANCY)
    {
        char kVertexShaderFilename[] = "Assets/Shaders/debugVideo.vert";
        char kFragmentShaderFilename[] = "Assets/Shaders/debugVideoOccupancy.frag";

        _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload);
    }
    else
    {
        assert(false);
    }

    if (!_shaderProgram)
    {
        LOG_E("Could not create program.");
    }

    _attributeVertices = glGetAttribLocation(_shaderProgram, "a_vertex");

    _uniformTextureY = glGetUniformLocation(_shaderProgram, "u_texture_y");
    _uniformTextureUV = glGetUniformLocation(_shaderProgram, "u_texture_uv");
    _uniformModel = glGetUniformLocation(_shaderProgram, "u_model");
    _uniformProjection = glGetUniformLocation(_shaderProgram, "u_projection");

    // Create VBO
    GLfloat vertices[] =
    {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GL_CHECK_ERRORS();
}

void DebugRenderer::destroy()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteProgram(_shaderProgram);

    GL_CHECK_ERRORS();
}

void DebugRenderer::draw(Texture2D& textureY, Texture2D& textureUV, glm::vec2 position, glm::vec2 size)
{
    pushDebugMarker("DebugRenderer");

    // Prepare transformations
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    GLint viewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, viewport);

    int32_t width = viewport[2];
    int32_t height = viewport[3];

    glm::mat4 projection = glm::ortho(0.0f,
                                      (GLfloat)width,
                                      (GLfloat)height,
                                      0.0f,
                                      -1.0f,
                                      1.0f);

    // Bind shader
    glUseProgram(_shaderProgram);
    glDepthMask(GL_FALSE);

    // Bind texture
    if (textureY.type != Texture2D::Type::INVALID)
    {
        glUniform1i(_uniformTextureY, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(textureY.type == Texture2D::Type::VIDEO_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, textureY.handle);
    }

    if (textureUV.type != Texture2D::Type::INVALID)
    {
        glUniform1i(_uniformTextureUV, 1);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(textureUV.type == Texture2D::Type::VIDEO_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, textureUV.handle);
    }

    // Set shader constants
    glUniformMatrix4fv(_uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(_uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

    // Bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);

    glEnableVertexAttribArray(_attributeVertices);
    glVertexAttribPointer(_attributeVertices, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);

    // Draw
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Cleanup
    glDisableVertexAttribArray(_attributeVertices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(0);
    glDepthMask(GL_TRUE);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(textureUV.type == Texture2D::Type::VIDEO_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(textureY.type == Texture2D::Type::VIDEO_TEXTURE ? GL_TEXTURE_EXTERNAL_OES : GL_TEXTURE_2D, 0);

    popDebugMarker();

    GL_CHECK_ERRORS();
}

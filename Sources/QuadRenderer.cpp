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

#include "QuadRenderer.h"

#include "Logger.h"

QuadRenderer::QuadRenderer()
{
}

QuadRenderer::~QuadRenderer()
{
}

void QuadRenderer::create(Texture2D::Type::Enum type, bool manualVideoTextureUpload)
{
    // Create shader
    if (type == Texture2D::Type::TEXTURE_RGB ||
        type == Texture2D::Type::TEXTURE_RGBA)
    {
        char kVertexShaderFilename[] = "Assets/Shaders/quad.vert";
        char kFragmentShaderFilename[] = "Assets/Shaders/quad.frag";

        _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload);
    }
    else if (type == Texture2D::Type::VIDEO_TEXTURE)
    {
        char kVertexShaderFilename[] = "Assets/Shaders/quad.vert";
        char kFragmentShaderFilename[] = "Assets/Shaders/quadVideo.frag";

        _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload);
    }

    if (!_shaderProgram)
    {
        LOG_E("Could not create program.");
    }

    _attributeVertices = glGetAttribLocation(_shaderProgram, "a_vertex");

    _uniformTexture0 = glGetUniformLocation(_shaderProgram, "u_texture0");
    _uniformModel = glGetUniformLocation(_shaderProgram, "u_model");
    _uniformProjection = glGetUniformLocation(_shaderProgram, "u_projection");
    _uniformTintColor = glGetUniformLocation(_shaderProgram, "u_tint_color");

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

void QuadRenderer::destroy()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteProgram(_shaderProgram);

    GL_CHECK_ERRORS();
}

void QuadRenderer::draw(Texture2D& texture, glm::vec2 position, glm::vec2 size, GLfloat rotate, glm::vec4 color)
{
    pushDebugMarker("QuadRenderer");

    // Prepare transformations
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));

    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, rotate, glm::vec3(0.0f, 0.0f, 1.0f));
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind shader
    glUseProgram(_shaderProgram);
    glDepthMask(GL_FALSE);

    // Set shader constants
    glUniformMatrix4fv(_uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(_uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform4f(_uniformTintColor, color.r, color.g, color.b, color.a);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);

    if (texture.type == Texture2D::Type::VIDEO_TEXTURE)
    {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texture.handle);
    }
    else if (texture.type == Texture2D::Type::TEXTURE_RGB ||
             texture.type == Texture2D::Type::TEXTURE_RGBA)
    {
        glBindTexture(GL_TEXTURE_2D, texture.handle);
    }
    else
    {
        assert(false);
    }

    glUniform1i(_uniformTexture0, 0);

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
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_BLEND);

    if (texture.type == Texture2D::Type::VIDEO_TEXTURE)
    {
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    }
    else if (texture.type == Texture2D::Type::TEXTURE_RGB ||
             texture.type == Texture2D::Type::TEXTURE_RGBA)
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        assert(false);
    }

    popDebugMarker();

    GL_CHECK_ERRORS();
}

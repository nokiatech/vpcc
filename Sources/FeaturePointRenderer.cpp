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

#include "FeaturePointRenderer.h"

#include "Logger.h"

FeaturePointRenderer::FeaturePointRenderer()
{
}

FeaturePointRenderer::~FeaturePointRenderer()
{
}

void FeaturePointRenderer::create()
{
    char kVertexShaderFilename[] = "Assets/Shaders/featurePoint.vert";
    char kFragmentShaderFilename[] = "Assets/Shaders/featurePoint.frag";

    _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename);

    if (!_shaderProgram)
    {
        LOG_E("Could not create program.");
    }

    _attributeVertices = glGetAttribLocation(_shaderProgram, "a_position");

    _uniformMVP = glGetUniformLocation(_shaderProgram, "u_mvp");
    _uniformPointColor = glGetUniformLocation(_shaderProgram, "u_color");
    _uniformPointSize = glGetUniformLocation(_shaderProgram, "u_point_size");

    GL_CHECK_ERRORS();
}

void FeaturePointRenderer::destroy()
{
    glDeleteProgram(_shaderProgram);

    GL_CHECK_ERRORS();
}

void FeaturePointRenderer::draw(glm::mat4 mvp, std::vector<FeaturePoint> featurePoints)
{
    pushDebugMarker("FeaturePointRenderer");

    glUseProgram(_shaderProgram);

    glUniformMatrix4fv(_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));

    glEnableVertexAttribArray(_attributeVertices);
    glVertexAttribPointer(_attributeVertices, 4, GL_FLOAT, GL_FALSE, 0, featurePoints.data());

    // Set cyan color to the point cloud.
    glUniform4f(_uniformPointColor, 31.0f / 255.0f, 188.0f / 255.0f, 210.0f / 255.0f, 1.0f);
    glUniform1f(_uniformPointSize, 5.0f);

    glDrawArrays(GL_POINTS, 0, (int32_t)featurePoints.size());

    // Cleanup
    glDisableVertexAttribArray(_attributeVertices);

    glUseProgram(0);

    popDebugMarker();

    GL_CHECK_ERRORS();
}

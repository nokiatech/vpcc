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

#if PLATFORM_ANDROID

#include "arcore_c_api.h"

#endif

struct FeaturePoint
{
    float x;
    float y;
    float z;
    float w;
};

class FeaturePointRenderer
{
public:

    FeaturePointRenderer();
    ~FeaturePointRenderer();

    void create();
    void destroy();

    void draw(glm::mat4 mvp, std::vector<FeaturePoint> featurePoints);

private:

    GLuint _shaderProgram;

    GLint _attributeVertices;

    GLint _uniformMVP;
    GLint _uniformPointColor;
    GLint _uniformPointSize;
};

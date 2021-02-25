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
#include "QuadRenderer.h"
#include "HWVideoDecoder.h"

#include "VPCC/VPCCParser121.h"
#include "VPCC/VPCCDecoder121.h"

namespace VPCC = VPCC121;

class VPCCRenderer
{
public:

    struct PresentationFrame
    {
        CachedFrame* depth0 = NULL;
        CachedFrame* depth1 = NULL;

        CachedFrame* color0 = NULL;
        CachedFrame* color1 = NULL;

        CachedFrame* occupancy = NULL;

        std::vector<VPCC::Patch> patches;
        std::vector<size_t> blockToPatch;
    };

public:

    VPCCRenderer();
    ~VPCCRenderer();

    void create(bool manualVideoTextureUpload);
    void destroy();

    void draw(PresentationFrame& presentationFrame, glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::mat4 mvp, glm::vec3 offset, float scale);

    bool dumpFrame(PresentationFrame& presentationFrame);
    bool dumpTexture(std::string filename, Texture2D texture);

    uint8_t* readTexture(Texture2D texture);

    bool takeScreenshot(std::string filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t bytesPerPixel = 3);

private:

    struct TFPoint3D
    {
        GLuint x;
        GLuint y;
        GLuint z;
        GLuint color;
    };

    QuadRenderer _quadRenderer;

    bool _initialized = false;
    GLuint _vbo = 0;

    GLuint _shaderProgram = 0;

    // Attributes
    GLuint _attributeBlockUV = 0;
    GLuint _attributePatchU0V0 = 0;
    GLuint _attributePatchU1V1D1 = 0;
    GLuint _attributeProjection = 0;
    GLuint _attributePatchSizeU0V0 = 0;
    GLuint _attributePatchProperties = 0;

    // Uniforms
    GLint _uniformColorTextureY = 0;
    GLint _uniformColorTextureUV = 0;
    GLint _uniformDepthTextureY = 0;
    GLint _uniformOccupancyTextureY = 0;

    GLint _uniformMVP = 0;
    GLint _uniformOffset = 0;
    GLint _uniformScale = 0;

    GLuint _transformFeedbackBuffer[1];
    GLuint _transformFeedback[1];

    bool _transformFeedbackComparison = true;

private:

    void bindTextures(PresentationFrame& presentationFrame, uint8_t layerIndex);
    void unbindTextures(PresentationFrame& presentationFrame, uint8_t layerIndex);

    void setupTransformFeedback();
    void teardownTransformFeedback();

    void transformFeedbackBegin();
    void transformFeedbackEnd();

    void generateGPUPointCloud(size_t numActiveBlocks, size_t numPoints, std::vector<TFPoint3D> output);
    void generateCPUPointCloud(PresentationFrame& presentationFrame, std::vector<TFPoint3D> output);

    bool verifyPointCloud(std::vector<TFPoint3D> cpuPoints, std::vector<TFPoint3D> gpuPoints);

    Texture2D createTexture(CachedFrame* frame, bool luma);
};

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

#include "VPCC/VPCCRenderer.h"

#include "Logger.h"
#include "FileSystem.h"
#include "TGA.h"

#include <iostream>
#include <fstream>

#define EXPORT_PATCHES 0
#define EXPORT_FRAMES 0

#define ENABLE_VERIFICATION_LAYER 0

VPCCRenderer::VPCCRenderer()
: _initialized(false)
{
}

VPCCRenderer::~VPCCRenderer()
{
}

void VPCCRenderer::create(bool manualVideoTextureUpload)
{
    // Shader
    char kVertexShaderFilename[] = "Assets/Shaders/vpcc.vert";
    char kFragmentShaderFilename[] = "Assets/Shaders/vpcc.frag";

#if ENABLE_VERIFICATION_LAYER

    const char* varyings[1];
    varyings[0] = "v_transformFeedback";

    _shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload, varyings, 1);

#else

	_shaderProgram = createProgram(kVertexShaderFilename, kFragmentShaderFilename, manualVideoTextureUpload, NULL, 1);

#endif

    if (!_shaderProgram)
    {
        LOG_E("Could not create program.");
    }

    // Attributes
    _attributeBlockUV = glGetAttribLocation(_shaderProgram, "a_block_uv");
    _attributePatchU0V0 = glGetAttribLocation(_shaderProgram, "a_patch_u0v0");
    _attributePatchU1V1D1 = glGetAttribLocation(_shaderProgram, "a_patch_u1v1d1");
    _attributeProjection = glGetAttribLocation(_shaderProgram, "a_projection");
    _attributePatchSizeU0V0 = glGetAttribLocation(_shaderProgram, "a_patch_size_u0v0");
    _attributePatchProperties = glGetAttribLocation(_shaderProgram, "a_patch_properties");

    // Uniforms
    _uniformDepthTextureY = glGetUniformLocation(_shaderProgram, "u_depth_y");
    _uniformColorTextureY = glGetUniformLocation(_shaderProgram, "u_color_y");
    _uniformColorTextureUV = glGetUniformLocation(_shaderProgram, "u_color_uv");
    _uniformOccupancyTextureY = glGetUniformLocation(_shaderProgram, "u_occupancy_y");

    _uniformMVP = glGetUniformLocation(_shaderProgram, "u_mvp");
    _uniformOffset = glGetUniformLocation(_shaderProgram, "u_offset");
    _uniformScale = glGetUniformLocation(_shaderProgram, "u_scale");

    _quadRenderer.create(Texture2D::Type::VIDEO_TEXTURE, manualVideoTextureUpload);

    setupTransformFeedback();

    GL_CHECK_ERRORS();
}

void VPCCRenderer::destroy()
{
    _quadRenderer.destroy();

    glDeleteBuffers(1, &_vbo);

    teardownTransformFeedback();

    GL_CHECK_ERRORS();
}

void VPCCRenderer::draw(PresentationFrame& presentationFrame, glm::mat4 model, glm::mat4 view, glm::mat4 projection, glm::mat4 mvp, glm::vec3 offset, float scale)
{
    pushDebugMarker("VPCCRenderer Init");

#if EXPORT_FRAMES

    dumpFrame(presentationFrame);

#endif

#if EXPORT_PATCHES
    {
        size_t numPatches = presentationFrame.patches.size();

        std::string patchesFilename = FileSystem::config().internalStoragePath;
        patchesFilename.append("patches.binary");

        FILE* handle = fopen(patchesFilename.c_str(), "wb");

        fwrite(&numPatches, sizeof(uint32_t), 1, handle);

        for (size_t patchIndex = 0; patchIndex < numPatches; ++patchIndex)
        {
            VPCC::Patch& patch = presentationFrame.patches[patchIndex];

            uint16_t u1 = (uint16_t)patch.u1;
            uint16_t v1 = (uint16_t)patch.v1;
            uint16_t d1 = (uint16_t)patch.d1;
            uint16_t u0 = (uint16_t)patch.u0;
            uint16_t v0 = (uint16_t)patch.v0;
            uint16_t sizeU0 = (uint16_t)patch.sizeU0;
            uint16_t sizeV0 = (uint16_t)patch.sizeV0;
            uint16_t normalAxis = (uint16_t)patch.normalAxis;
            uint16_t tangentAxis = (uint16_t)patch.tangentAxis;
            uint16_t bitangentAxis = (uint16_t)patch.bitangentAxis;

            fwrite(&u1, sizeof(uint16_t), 1, handle);
            fwrite(&v1, sizeof(uint16_t), 1, handle);
            fwrite(&d1, sizeof(uint16_t), 1, handle);
            fwrite(&u0, sizeof(uint16_t), 1, handle);
            fwrite(&v0, sizeof(uint16_t), 1, handle);
            fwrite(&sizeU0, sizeof(uint16_t), 1, handle);
            fwrite(&sizeV0, sizeof(uint16_t), 1, handle);
            fwrite(&normalAxis, sizeof(uint16_t), 1, handle);
            fwrite(&tangentAxis, sizeof(uint16_t), 1, handle);
            fwrite(&bitangentAxis, sizeof(uint16_t), 1, handle);
        }

        fflush(handle);
        fclose(handle);
    }
#endif

    // Bind shader
    glUseProgram(_shaderProgram);

    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set shader constants

    // All frames need to have same size!
    uint32_t videoWidth = presentationFrame.depth0->width;
    uint32_t videoHeight = presentationFrame.depth0->height;

    glUniformMatrix4fv(_uniformMVP, 1, GL_FALSE, glm::value_ptr(mvp));

    glUniform3f(_uniformOffset, offset.x, offset.y, offset.z);
    glUniform1f(_uniformScale, scale);

    // Prepare block data
    uint16_t blockSize = 16;

    uint16_t blockW = blockSize;
    uint16_t blockH = blockSize;

    std::vector<int16_t> blockBuffer;
    uint16_t blockAttributeSize = sizeof(int16_t);
    uint16_t blockAttributes = 2 + 2 + 3 + 3 + 4 + 4;

    blockBuffer.resize(blockAttributes * (videoWidth / blockW) * (videoHeight / blockH));

    if (!_initialized)
    {
        _initialized = true;

        glGenBuffers(1, &_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, blockAttributeSize * blockBuffer.size(), &blockBuffer[0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        GL_CHECK_ERRORS();
    }

    // Prepare block buffer
    uint16_t activeBlocks = 0;
    size_t numPatches = presentationFrame.patches.size();

    for (size_t patchIndex = 0; patchIndex < numPatches; patchIndex++)
    {
        VPCC::Patch& p = presentationFrame.patches[patchIndex];

        const size_t blockToPatchWidth = presentationFrame.depth0->width / p.occupancyResolution;
        const size_t blockToPatchHeight = presentationFrame.depth0->height / p.occupancyResolution;

        if (!((p.normalAxis == 0 && p.tangentAxis == 2 && p.bitangentAxis == 1) ||
              (p.normalAxis == 2 && p.tangentAxis == 0 && p.bitangentAxis == 1) ||
              (p.normalAxis == 1 && p.tangentAxis == 2 && p.bitangentAxis == 0)))
        {
            assert(false);
        }

        for (uint32_t by = 0; by < p.sizeV0; by++)
        {
            for (uint32_t bx = 0; bx < p.sizeU0; bx++)
            {
                size_t blockToPatchIndex = VPCC::patchBlockToCanvasBlock(p, bx, by, blockToPatchWidth, blockToPatchHeight);
                size_t blockIndex = presentationFrame.blockToPatch[blockToPatchIndex];

                if (blockIndex == (patchIndex + 1))
                {
                    blockBuffer[activeBlocks * blockAttributes + 0] = bx;
                    blockBuffer[activeBlocks * blockAttributes + 1] = by;

                    blockBuffer[activeBlocks * blockAttributes + 2] = uint16_t(p.u0);
                    blockBuffer[activeBlocks * blockAttributes + 3] = uint16_t(p.v0);

                    blockBuffer[activeBlocks * blockAttributes + 4] = uint16_t(p.u1);
                    blockBuffer[activeBlocks * blockAttributes + 5] = uint16_t(p.v1);
                    blockBuffer[activeBlocks * blockAttributes + 6] = uint16_t(p.d1);

                    blockBuffer[activeBlocks * blockAttributes + 7] = uint16_t(p.normalAxis);
                    blockBuffer[activeBlocks * blockAttributes + 8] = uint16_t(p.tangentAxis);
                    blockBuffer[activeBlocks * blockAttributes + 9] = uint16_t(p.bitangentAxis);

                    blockBuffer[activeBlocks * blockAttributes + 10] = uint16_t(p.sizeU0);
                    blockBuffer[activeBlocks * blockAttributes + 11] = uint16_t(p.sizeV0);
                    blockBuffer[activeBlocks * blockAttributes + 12] = uint16_t(0);
                    blockBuffer[activeBlocks * blockAttributes + 13] = uint16_t(0);

                    blockBuffer[activeBlocks * blockAttributes + 14] = uint16_t(p.patchOrientation);
                    blockBuffer[activeBlocks * blockAttributes + 15] = uint16_t(p.occupancyResolution);
                    blockBuffer[activeBlocks * blockAttributes + 16] = uint16_t(p.projectionMode);
                    blockBuffer[activeBlocks * blockAttributes + 17] = uint16_t(0);

                    activeBlocks++;
                }
            }
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, blockAttributeSize * blockAttributes * activeBlocks, &blockBuffer[0]);

    glEnableVertexAttribArray(_attributeBlockUV);
    glEnableVertexAttribArray(_attributePatchU0V0);
    glEnableVertexAttribArray(_attributePatchU1V1D1);
    glEnableVertexAttribArray(_attributeProjection);
    glEnableVertexAttribArray(_attributePatchSizeU0V0);
    glEnableVertexAttribArray(_attributePatchProperties);

    GLsizei stride = blockAttributes * blockAttributeSize;
    GLuint divisor = 1;

    glVertexAttribIPointer(_attributeBlockUV, 2, GL_SHORT, stride, (GLvoid*)(size_t)(0 * blockAttributeSize));
    glVertexAttribIPointer(_attributePatchU0V0, 2, GL_SHORT, stride, (GLvoid*)(size_t)(2 * blockAttributeSize));
    glVertexAttribIPointer(_attributePatchU1V1D1, 3, GL_SHORT, stride, (GLvoid*)(size_t)(4 * blockAttributeSize));
    glVertexAttribIPointer(_attributeProjection, 3, GL_SHORT, stride, (GLvoid*)(size_t)(7 * blockAttributeSize));
    glVertexAttribIPointer(_attributePatchSizeU0V0, 4, GL_SHORT, stride, (GLvoid*)(size_t)(10 * blockAttributeSize));
    glVertexAttribIPointer(_attributePatchProperties, 4, GL_SHORT, stride, (GLvoid*)(size_t)(14 * blockAttributeSize));

    // update vertex attributes for each instance
    glVertexAttribDivisor(_attributeBlockUV, divisor);
    glVertexAttribDivisor(_attributePatchU0V0, divisor);
    glVertexAttribDivisor(_attributePatchU1V1D1, divisor);
    glVertexAttribDivisor(_attributeProjection, divisor);
    glVertexAttribDivisor(_attributePatchSizeU0V0, divisor);
    glVertexAttribDivisor(_attributePatchProperties, divisor);

    GL_CHECK_ERRORS();

    popDebugMarker();

    // Draw layer #1
    if (_transformFeedbackComparison)
    {
        pushDebugMarker("VPCCRenderer Transform feedback layer #1");

        transformFeedbackBegin();
    }
    else
    {
        pushDebugMarker("VPCCRenderer Draw layer #1");
    }

    bindTextures(presentationFrame, 0);

    glDrawArraysInstanced(GL_POINTS, 0, blockW * blockH, activeBlocks);
    GL_CHECK_ERRORS();

    if (_transformFeedbackComparison)
    {
        transformFeedbackEnd();

        // GPU points
        size_t numPoints = activeBlocks * blockSize * blockSize;

        std::vector<TFPoint3D> gpuPoints;
        gpuPoints.reserve(numPoints);

        generateGPUPointCloud(activeBlocks, numPoints, gpuPoints);

        // CPU points
        std::vector<TFPoint3D> cpuPoints;

        generateCPUPointCloud(presentationFrame, cpuPoints);

        // Verify that rendering results match to CPU reference implementation
        bool result = verifyPointCloud(cpuPoints, gpuPoints);

        _transformFeedbackComparison = false;
    }

    unbindTextures(presentationFrame, 0);

    popDebugMarker();

    // Draw layer #2
    if (presentationFrame.depth1 != NULL && presentationFrame.color1 != NULL)
    {
        pushDebugMarker("VPCCRenderer Draw layer #2");

        bindTextures(presentationFrame, 1);

        glDrawArraysInstanced(GL_POINTS, 0, blockW * blockH, activeBlocks);
        GL_CHECK_ERRORS();

        popDebugMarker();

        unbindTextures(presentationFrame, 1);
    }

    // Cleanup
    pushDebugMarker("VPCCRenderer Cleanup");

    glVertexAttribDivisor(_attributeBlockUV, 0);
    glVertexAttribDivisor(_attributePatchU0V0, 0);
    glVertexAttribDivisor(_attributePatchU1V1D1, 0);
    glVertexAttribDivisor(_attributeProjection, 0);
    glVertexAttribDivisor(_attributePatchSizeU0V0, 0);
    glVertexAttribDivisor(_attributePatchProperties, 0);

    glDisableVertexAttribArray(_attributeBlockUV);
    glDisableVertexAttribArray(_attributePatchU0V0);
    glDisableVertexAttribArray(_attributePatchU1V1D1);
    glDisableVertexAttribArray(_attributeProjection);
    glDisableVertexAttribArray(_attributePatchSizeU0V0);
    glDisableVertexAttribArray(_attributePatchProperties);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUseProgram(0);

    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    popDebugMarker();

    GL_CHECK_ERRORS();
}

void VPCCRenderer::bindTextures(PresentationFrame& presentationFrame, uint8_t layerIndex)
{
    CachedFrame* depth = (layerIndex == 0) ? presentationFrame.depth0 : presentationFrame.depth1;
    CachedFrame* color = (layerIndex == 0) ? presentationFrame.color0 : presentationFrame.color1;
    CachedFrame* occupancy = presentationFrame.occupancy;

    uint16_t textureUnit = 0;

    // Depth
    if (_uniformDepthTextureY != -1)
    {
        glUniform1i(_uniformDepthTextureY, textureUnit);

        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(depth->target, depth->yTextureHandle);

        textureUnit++;
    }

    // Color
    if (_uniformColorTextureY != -1)
    {
        glUniform1i(_uniformColorTextureY, textureUnit);

        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(color->target, color->yTextureHandle);

        textureUnit++;
    }

    if (_uniformColorTextureUV != -1)
    {
        glUniform1i(_uniformColorTextureUV, textureUnit);

        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(color->target, color->uvTextureHandle);

        textureUnit++;
    }

    // Occupancy
    if (_uniformOccupancyTextureY != -1)
    {
        glUniform1i(_uniformOccupancyTextureY, textureUnit);

        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(occupancy->target, occupancy->yTextureHandle);

        textureUnit++;
    }

    glActiveTexture(GL_TEXTURE0);

    GL_CHECK_ERRORS();
}

void VPCCRenderer::unbindTextures(PresentationFrame& presentationFrame, uint8_t layerIndex)
{
    CachedFrame* depth = (layerIndex == 0) ? presentationFrame.depth0 : presentationFrame.depth1;
    CachedFrame* color = (layerIndex == 0) ? presentationFrame.color0 : presentationFrame.color1;
    CachedFrame* occupancy = presentationFrame.occupancy;

    uint16_t textureUnit = 0;

    if (_uniformOccupancyTextureY != -1)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(depth->target, 0);

        textureUnit++;
    }

    if (_uniformColorTextureY != -1)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(color->target, 0);

        textureUnit++;
    }

    if (_uniformColorTextureUV != -1)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(color->target, 0);

        textureUnit++;
    }

    if (_uniformDepthTextureY != -1)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(occupancy->target, 0);

        textureUnit++;
    }

    glActiveTexture(GL_TEXTURE0);

    GL_CHECK_ERRORS();
}

Texture2D VPCCRenderer::createTexture(CachedFrame* frame, bool luma)
{
    Texture2D texture;
    texture.width = frame->width / ((luma) ? 1 : 2);
    texture.height = frame->height / ((luma) ? 1 : 2);
    texture.handle = frame->yTextureHandle;
    texture.type = (frame->target == GL_TEXTURE_EXTERNAL_OES) ? Texture2D::Type::VIDEO_TEXTURE : Texture2D::Type::TEXTURE_RGB;

    return texture;
}

bool VPCCRenderer::dumpFrame(VPCCRenderer::PresentationFrame& presentationFrame)
{
    bool result = false;

    {
        std::string depth0yFilename;
        depth0yFilename.append("geometry0_y_frame_");
        depth0yFilename.append(std::to_string(presentationFrame.depth0->pts));
        depth0yFilename.append(".tga");

        Texture2D texture = createTexture(presentationFrame.depth0, true);

        result |= dumpTexture(depth0yFilename, texture);
    }

    {
        std::string color0yFilename;
        color0yFilename.append("texture0_y_frame_");
        color0yFilename.append(std::to_string(presentationFrame.color0->pts));
        color0yFilename.append(".tga");

        Texture2D texture = createTexture(presentationFrame.color0, true);

        result |= dumpTexture(color0yFilename, texture);
    }

    {
        std::string color0uvFilename;
        color0uvFilename.append("texture0_uv_frame_");
        color0uvFilename.append(std::to_string(presentationFrame.color0->pts));
        color0uvFilename.append(".tga");

        Texture2D texture = createTexture(presentationFrame.color0, false);

        result |= dumpTexture(color0uvFilename, texture);
    }

    {
        std::string occupancyFilename;
        occupancyFilename.append("occupancy_frame_");
        occupancyFilename.append(std::to_string(presentationFrame.occupancy->pts));
        occupancyFilename.append(".tga");

        Texture2D texture = createTexture(presentationFrame.occupancy, true);

        result |= dumpTexture(occupancyFilename, texture);
    }

    if (presentationFrame.depth1 != NULL)
    {
        std::string depth1yFilename;
        depth1yFilename.append("geometry1_y_frame_");
        depth1yFilename.append(std::to_string(presentationFrame.depth0->pts));
        depth1yFilename.append(".tga");

        Texture2D texture = createTexture(presentationFrame.depth1, true);

        result |= dumpTexture(depth1yFilename, texture);
    }

    if (presentationFrame.color1 != NULL)
    {
        {
            std::string color1yFilename;
            color1yFilename.append("texture1_y_frame_");
            color1yFilename.append(std::to_string(presentationFrame.color1->pts));
            color1yFilename.append(".tga");

            Texture2D texture = createTexture(presentationFrame.color1, true);

            result |= dumpTexture(color1yFilename, texture);
        }

        {
            std::string color1uvFilename;
            color1uvFilename.append("texture1_uv_frame_");
            color1uvFilename.append(std::to_string(presentationFrame.color1->pts));
            color1uvFilename.append(".tga");

            Texture2D texture = createTexture(presentationFrame.color1, false);

            result |= dumpTexture(color1uvFilename, texture);
        }
    }

    GL_CHECK_ERRORS();

    return result;
}

bool VPCCRenderer::dumpTexture(std::string filename, Texture2D texture)
{
    uint16_t width = texture.width;
    uint16_t height = texture.height;

    // Default viewport
    GLint defaultViewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, defaultViewport);

    // Default FBO
    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

    // Create FBO for texture
    GLuint offscreenFramebuffer = 0;
    glGenFramebuffers(1, &offscreenFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer);

    // Create the texture
    GLuint targetTexture = 0;
    glGenTextures(1, &targetTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Bind the texture to off-screen FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);

    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            assert(false);
        }
    }

    // Bind off-screen FBO
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer);

    // Set the viewport for off-screen FBO
    glViewport(0, 0, width, height);

    // Render video decoder frame to texture since GL_TEXTURE_EXTERNAL_OES type cannot be bound to FBO
    _quadRenderer.draw(texture, glm::vec2(0.0, 0.0), glm::vec2(width, height));

    // Save FBO texture as TGA
    takeScreenshot(filename, 0, 0, width, height, 3);

    // Bind default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);

    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            assert(false);
        }
    }

    // Set default viewport
    glViewport(defaultViewport[0], defaultViewport[1], defaultViewport[2], defaultViewport[3]);

    // Cleanup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(1, &targetTexture);
    glDeleteFramebuffers(1, &offscreenFramebuffer);

    GL_CHECK_ERRORS();

    return true;
}

uint8_t* VPCCRenderer::readTexture(Texture2D texture)
{
    uint16_t width = texture.width;
    uint16_t height = texture.height;

    // Default viewport
    GLint defaultViewport[4] = { 0 };
    glGetIntegerv(GL_VIEWPORT, defaultViewport);

    // Default FBO
    GLint defaultFBO = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFBO);

    // Create FBO for texture
    GLuint offscreenFramebuffer = 0;
    glGenFramebuffers(1, &offscreenFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer);

    // Create the texture
    GLuint targetTexture = 0;
    glGenTextures(1, &targetTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, targetTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Bind the texture to off-screen FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);

    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            assert(false);
        }
    }

    // Bind off-screen FBO
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer);

    // Set the viewport for off-screen FBO
    glViewport(0, 0, width, height);

    // Render video decoder frame to texture since GL_TEXTURE_EXTERNAL_OES type cannot be bound to FBO
    _quadRenderer.draw(texture, glm::vec2(0.0, 0.0), glm::vec2(width, height));

    // Dump FBO texture
    uint8_t bytesPerPixel = 3;

    size_t bytes = width * height * bytesPerPixel;
    uint8_t* buffer = (uint8_t*)malloc(bytes);

    if (!buffer)
    {
        return NULL;
    }

    // Read
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0,
                 (GLint)width, (GLint)height,
                 (bytesPerPixel == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    GL_CHECK_ERRORS();

    // RGB -> BGR conversion
    for (uint16_t y = 0; y < height; y++)
    {
        for (uint16_t x = 0; x < width; x++)
        {
            size_t index = (y * width + x) * bytesPerPixel;

            uint8_t r = buffer[index + 0];
            uint8_t g = buffer[index + 1];
            uint8_t b = buffer[index + 2];

            buffer[index + 0] = b; // B
            buffer[index + 1] = g; // G
            buffer[index + 2] = r; // R
        }
    }

    // Bind default FBO
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFBO);

    {
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            assert(false);
        }
    }

    // Set default viewport
    glViewport(defaultViewport[0], defaultViewport[1], defaultViewport[2], defaultViewport[3]);

    // Cleanup
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDeleteTextures(1, &targetTexture);
    glDeleteFramebuffers(1, &offscreenFramebuffer);

    GL_CHECK_ERRORS();

    return buffer;
}

bool VPCCRenderer::takeScreenshot(std::string filename, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t bytesPerPixel)
{
    // Alloc
    size_t bytes = width * height * bytesPerPixel;
    uint8_t* buffer = (uint8_t*)malloc(bytes);

    if (!buffer)
    {
        return false;
    }

    // Read
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels((GLint)x, (GLint)y,
                 (GLint)width, (GLint)height,
                 (bytesPerPixel == 3) ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    GL_CHECK_ERRORS();

    // RGB -> BGR conversion
    for (uint16_t y = 0; y < height; y++)
    {
        for (uint16_t x = 0; x < width; x++)
        {
            size_t index = (y * width + x) * bytesPerPixel;

            uint8_t r = buffer[index + 0];
            uint8_t g = buffer[index + 1];
            uint8_t b = buffer[index + 2];

            buffer[index + 0] = b; // B
            buffer[index + 1] = g; // G
            buffer[index + 2] = r; // R
        }
    }

    // Save
    bool result = TGA::saveToDisk(filename, buffer, bytes, width, height, bytesPerPixel);

    // Cleanup
    free(buffer);
    buffer = NULL;

    return result;
}

void VPCCRenderer::setupTransformFeedback()
{
#if ENABLE_VERIFICATION_LAYER

    GL_CHECK_ERRORS();

    // TODO: Hackish max value calculation
    size_t maxPoints = 1280 * 1280 * 2;

    glGenBuffers(1, _transformFeedbackBuffer);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, _transformFeedbackBuffer[0]);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, maxPoints * sizeof(TFPoint3D), NULL, GL_STATIC_DRAW);
    GL_CHECK_ERRORS();

    glGenTransformFeedbacks(1, _transformFeedback);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _transformFeedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _transformFeedbackBuffer[0]);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    GL_CHECK_ERRORS();

#endif
}

void VPCCRenderer::transformFeedbackBegin()
{
#if ENABLE_VERIFICATION_LAYER
    
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _transformFeedback[0]);
    GL_CHECK_ERRORS();

    glBeginTransformFeedback(GL_POINTS);
    GL_CHECK_ERRORS();

#endif
}

void VPCCRenderer::transformFeedbackEnd()
{
#if ENABLE_VERIFICATION_LAYER

    GL_CHECK_ERRORS();
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

#endif
}

void VPCCRenderer::teardownTransformFeedback()
{
#if ENABLE_VERIFICATION_LAYER

    glDeleteTransformFeedbacks(1, _transformFeedback);
    glDeleteBuffers(1, _transformFeedbackBuffer);

#endif
}

void VPCCRenderer::generateGPUPointCloud(size_t numActiveBlocks, size_t numPoints, std::vector<TFPoint3D> output)
{
#if ENABLE_VERIFICATION_LAYER

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, _transformFeedbackBuffer[0]);
    GLuint* buffer = (GLuint*)glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, numPoints * sizeof(TFPoint3D), GL_MAP_READ_BIT);

    size_t skipped = 0;

    if (buffer)
    {
        for (size_t i = 0; i < numPoints; i++)
        {
            TFPoint3D p;
            p.x = buffer[i * 4 + 0];
            p.y = buffer[i * 4 + 1];
            p.z = buffer[i * 4 + 2];

            if ((p.x != 0x7fff) && (p.y != 0x7fff) && (p.z != 0x7fff))
            {
                p.color = buffer[i * 4 + 3];

                output.push_back(p);
            }
            else
            {
                skipped++;
            }
        }
    }

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

    LOG_V("%d points decoded with GPU, blocks = %d, skipped = %d\n", output.size(), numActiveBlocks, skipped);

#endif
}

void VPCCRenderer::generateCPUPointCloud(PresentationFrame& presentationFrame, std::vector<TFPoint3D> output)
{
#if ENABLE_VERIFICATION_LAYER

    // Read texture from GPU
    Texture2D depth0Texture = createTexture(presentationFrame.depth0, true);
    Texture2D color0Texture = createTexture(presentationFrame.color0, true);
    Texture2D occupancyTexture = createTexture(presentationFrame.occupancy, true);

    uint8_t* depth0Raw = readTexture(depth0Texture);
    uint8_t* color0Raw = readTexture(color0Texture);
    uint8_t* occupancyRaw = readTexture(occupancyTexture);

    uint8_t* depth1Raw = NULL; // TODO
    uint8_t* color1Raw = NULL; // TODO

#if 0
    {
        size_t emptys = 0;
        printf("CPU DECODE blockToPatch\n");

        for (size_t y = 0; y < frame.h / frame.occupancyResolution; ++y)
        {
            for (size_t x = 0; x < frame.w / frame.occupancyResolution; ++x)
            {
                const size_t p = y * (frame.w / frame.occupancyResolution) + x;
                uint16_t index = frame.blockToPatch[p];

                bool empty = blockEmpty(frame, x, y);

                if (index && empty)
                {
                    emptys++;
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7 + 16); // 4
                    printf("%3x", index);
                    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
                }
                else
                {
                    printf("%3x", index);
                }
            }
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
            printf("\n");
        }
        printf("emptys=%d\n", emptys);
    }
#endif

    auto &pointCloud = output;

    pointCloud.clear();

    size_t emptyBlocks = 0;
    size_t blocks = 0;

    const size_t layerCount = 1;

    const size_t imageWidth = presentationFrame.depth0->width;
    const size_t imageHeight = presentationFrame.depth0->height;

    const auto& blockToPatch = presentationFrame.blockToPatch;
    const size_t patchCount = presentationFrame.patches.size();

    uint8_t occupancyResolution = 0;

    for (size_t patchIndex = 0; patchIndex < patchCount; ++patchIndex)
    {
        const size_t patchIndexPlusOne = patchIndex + 1;

        VPCC::Patch& patch = presentationFrame.patches[patchIndex];
        occupancyResolution = patch.occupancyResolution;

        const size_t blockToPatchWidth = imageWidth / occupancyResolution;

        for (size_t v0 = 0; v0 < patch.sizeV0; ++v0)
        {
            for (size_t u0 = 0; u0 < patch.sizeU0; ++u0)
            {
                if (blockToPatch[(v0 + patch.v0) * blockToPatchWidth + u0 + patch.u0] == patchIndexPlusOne)
                {
                    blocks++;

                    // check empty blocks
                    //bool empty = blockEmpty(frame, u0, v0);
                    //if (empty) emptyBlocks++;

                    for (size_t v1 = 0; v1 < occupancyResolution; ++v1)
                    {
                        const size_t v = v0 * occupancyResolution + v1;

                        for (size_t u1 = 0; u1 < occupancyResolution; ++u1)
                        {
                            const size_t u = u0 * occupancyResolution + u1;
                            const size_t x = patch.u0 * occupancyResolution + u;
                            const size_t y = patch.v0 * occupancyResolution + v;

                            const size_t p = y * imageWidth + x;

                            const bool occupancy = occupancyRaw[p] != 0;

                            if (!occupancy)
                            {
                                continue;
                            }

                            {
                                GLuint point0[3];
                                point0[patch.normalAxis] = uint32_t(depth0Raw[p] + patch.d1);
                                point0[patch.tangentAxis] = uint32_t(u) + patch.u1;
                                point0[patch.bitangentAxis] = uint32_t(v) + patch.v1;

                                TFPoint3D p0;
                                p0.x = point0[0];
                                p0.y = point0[1];
                                p0.z = point0[2];

                                uint8_t r = color0Raw[p * 3 + 0];
                                uint8_t g = color0Raw[p * 3 + 1];
                                uint8_t b = color0Raw[p * 3 + 2];
                                p0.color = (b << 16) + (g << 8) + r;

                                pointCloud.push_back(p0);
                            }

                            if (layerCount == 2)
                            {
                                GLuint point1[3];
                                point1[patch.normalAxis] = uint32_t(depth1Raw[p] + patch.d1);
                                point1[patch.tangentAxis] = uint32_t(u) + patch.u1;
                                point1[patch.bitangentAxis] = uint32_t(v) + patch.v1;

                                TFPoint3D p1;
                                p1.x = point1[0];
                                p1.y = point1[1];
                                p1.z = point1[2];

                                uint8_t r = color1Raw[p * 3 + 0];
                                uint8_t g = color1Raw[p * 3 + 1];
                                uint8_t b = color1Raw[p * 3 + 2];
                                p1.color = (b << 16) + (g << 8) + r;

                                pointCloud.push_back(p1);
                            }
                        }
                    }
                }
            }
        }
    }

    LOG_V("%d points decoded with CPU, blocks = %d, emptyBlocks = %d, emptyBlockPoints = %d\n", output.size(), blocks, emptyBlocks, emptyBlocks * occupancyResolution * occupancyResolution);

#endif
}

bool VPCCRenderer::verifyPointCloud(std::vector<TFPoint3D> cpuPoints, std::vector<TFPoint3D> gpuPoints)
{
#if ENABLE_VERIFICATION_LAYER

    struct sortPoint
    {
        inline bool operator() (const TFPoint3D& p0, const TFPoint3D& p1)
        {
            if (p0.x == p1.x)
            {
                if (p0.y == p1.y)
                {
                    if (p0.z == p1.z)
                    {
                        return (p0.color < p1.color);
                    }
                    else
                    {
                        return (p0.z < p1.z);
                    }
                }
                else
                {
                    return (p0.y < p1.y);
                }
            }

            return (p0.x < p1.x);
        }
    };

    std::sort(cpuPoints.begin(), cpuPoints.end(), sortPoint());
    std::sort(gpuPoints.begin(), gpuPoints.end(), sortPoint());

    size_t points = std::min(cpuPoints.size(), gpuPoints.size());

    for (size_t i = 0; i < points; i++)
    {
        auto &p0 = cpuPoints[i];
        auto &p1 = gpuPoints[i];

        if ((p0.x != p1.x) || (p0.y != p1.y) || (p0.z != p1.z) || (p0.color != p1.color))
        {
            LOG_V("CPU %d %d %d 0x%x GPU %d %d %d 0x%x\n",
                   p0.x, p0.y, p0.z, p0.color,
                   p1.x, p1.y, p1.z, p1.color);

            // return false;
        }
    }

#endif

    return true;
}

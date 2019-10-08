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

#include <cstdlib>

#include <vector>
#include <string>
#include <queue>
#include <mutex>

#if PLATFORM_IOS || PLATFORM_MACOS

    #import <CoreMedia/CoreMedia.h>
    #import <VideoToolbox/VideoToolbox.h>

#endif

#include "GraphicsAPI.h"
#include "HEVC.h"

struct CachedFrame
{
    int64_t pts = 0;
    int64_t duration = 0;

    bool uploaded = false;

    uint16_t width = 0;
    uint16_t height = 0;

    GLenum target = 0;

    GLuint yTextureHandle = 0;
    GLuint uvTextureHandle = 0;

#if PLATFORM_ANDROID

    ssize_t outputBufferId = 0;
    uint8_t* buffer = NULL;

#elif PLATFORM_IOS

    CVPixelBufferRef pixelBuffer = NULL;
    CVOpenGLESTextureRef textureRef[2] = { NULL };

#elif PLATFORM_MACOS

    CVPixelBufferRef pixelBuffer = NULL;
    CVOpenGLTextureRef textureRef[2] = { NULL };

#elif PLATFORM_WINDOWS

	uint8_t* buffer = NULL;

#else

	#error Unsupported platform

#endif

    static void reset(CachedFrame* frame)
    {
        frame->pts = 0;
        frame->duration = 0;

        frame->uploaded = false;

        frame->width = 0;
        frame->height = 0;

#if PLATFORM_ANDROID

        frame->outputBufferId = 0;
        frame->buffer = NULL;

#elif PLATFORM_IOS || PLATFORM_MACOS

        frame->pixelBuffer = NULL;

        if (frame->textureRef[0] != NULL)
        {
            CFRelease(frame->textureRef[0]);
            frame->textureRef[0] = NULL;
        }

        if (frame->textureRef[1] != NULL)
        {
            CFRelease(frame->textureRef[1]);
            frame->textureRef[1] = NULL;
        }
        
        frame->target = 0;

#elif PLATFORM_WINDOWS

		frame->buffer = NULL;

#else

	#error Unsupported platform

#endif
    }
};

struct DecoderConfig
{
    DecoderParameters parameters;

    uint16_t width = 0;
    uint16_t height = 0;

    std::string name = "";

    bool manualVideoTextureUpload = false;

    int32_t outputBufferQueueSize = 0;
    int32_t inputBufferQueueSize = 0;
};

class HWVideoDecoderBase
{
public:

    bool inputEOS = false;
    bool outputEOS = false;

    int64_t numTotalFramesDecoded = 0;

public:

    HWVideoDecoderBase();
    ~HWVideoDecoderBase();

public: // Decoder

    virtual bool initialize(DecoderConfig& config) = 0;
    virtual bool shutdown() = 0;

    virtual bool start() = 0;
    virtual bool stop() = 0;

    virtual bool flush() = 0;

    virtual bool queueVideoInputBuffer(uint8_t* data, size_t bytes, int64_t decodeTimeStamp, int64_t presentationTimeStamp, bool inputEOS = false) = 0;
    virtual bool dequeueOutputBuffer() = 0;

    const DecoderConfig& getConfig() const;

    bool isInputQueueEmpty();
    bool isInputQueueFull();

    bool isOutputQueueEmpty();
    bool isOutputQueueFull();

    size_t getOutputQueueSize();

    CachedFrame* retainCachedFrame();
    bool releaseCachedFrame(CachedFrame* frame);

protected:

    virtual bool uploadTexture(CachedFrame& frame) = 0;

protected:

    DecoderConfig _config;

    int32_t _inputBuffers = 0;

    // Caches frames waiting for upload and usage
    std::queue<CachedFrame*> _outputBuffers;
    std::queue<CachedFrame*> _freeOutputBuffers;

    std::mutex _frameCacheMutex;
};

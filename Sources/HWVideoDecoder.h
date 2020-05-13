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

    uint32_t width = 0;
    uint32_t height = 0;

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
    HEVC::DecoderParameters parameters;

    int32_t width = 0;
    int32_t height = 0;

    std::string name = "";

    bool manualVideoTextureUpload = false;

    int32_t outputBufferQueueSize = 0;
    int32_t inputBufferQueueSize = 0;
};

struct DecoderStats
{
    uint32_t numTotalFrames = 0;
    float averageFPS = 0.0f;
    uint32_t averageFrameDurationMs = 0;
};

class HWVideoDecoderBase
{
public:

    bool inputEOS = false;
    bool outputEOS = false;

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

public:

    const DecoderConfig& getConfig() const;

    bool isValid() const;

    bool isInputQueueEmpty();
    bool isInputQueueFull();

    bool isOutputQueueEmpty();
    bool isOutputQueueFull();

    size_t getOutputQueueSize();

    bool isCachedFrameReady(int64_t presentationTimeStamp);

    CachedFrame* retainCachedFrame(int64_t presentationTimeStamp);
    bool releaseCachedFrame(CachedFrame* frame);

    const DecoderStats& getStats() const;

    void beginStatisticsScope();
    void endStatisticsScope();

    void printStatistics();

    void flushCachedFrames();

protected:

    virtual bool uploadTexture(CachedFrame& frame) = 0;

protected:

    DecoderConfig _config;

    int32_t _inputBuffers = 0;

    // Caches frames waiting for upload and usage
    std::vector<CachedFrame*> _outputBuffers;
    std::queue<CachedFrame*> _freeOutputBuffers;

    std::recursive_mutex _frameCacheMutex;

    int64_t _totalFrameDecodingStartTime = 0;
    int64_t _numTotalFramesDecoded = 0;
    
    DecoderStats _statistics;

    bool _initialized = false;
};

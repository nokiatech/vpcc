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

extern "C"
{
    #include <jni.h>
    #include <android/log.h>

    #include <media/NdkMediaCodec.h>
    #include <media/NdkMediaCrypto.h>
    #include <media/NdkMediaDrm.h>
    #include <media/NdkMediaError.h>
    #include <media/NdkMediaExtractor.h>
    #include <media/NdkMediaFormat.h>
    #include <media/NdkMediaMuxer.h>

    #include <android/rect.h>
    #include <android/window.h>
    #include <android/native_window.h>
    #include <android/native_window_jni.h>
}

#include "HWVideoDecoder.h"
#include "Surface.h"

struct OutputTexture
{
    OutputTexture()
    {
        surface = NULL;
        nativeWindow = NULL;
    }

    ~OutputTexture()
    {
        ANativeWindow_release(nativeWindow);
        nativeWindow = NULL;

        delete surface;
        surface = NULL;
    }

    Surface* surface;
    ANativeWindow* nativeWindow;
};

class HWVideoDecoder
: public HWVideoDecoderBase
{
public:

    HWVideoDecoder();
    ~HWVideoDecoder();

public: // Decoder

    virtual bool initialize(DecoderConfig& config);
    virtual bool shutdown();

    virtual bool start();
    virtual bool stop();

    virtual bool flush();

    virtual bool queueVideoInputBuffer(uint8_t* data, size_t bytes, int64_t decodeTimeStamp, int64_t presentationTimeStamp, bool inputEOS = false);
    virtual bool dequeueOutputBuffer();

protected:

    virtual bool uploadTexture(CachedFrame& frame);

private:

    AMediaFormat* _inputFormat = NULL;
    AMediaCodec* _mediaCodec = NULL;

    OutputTexture* _outputTexture = NULL;
};

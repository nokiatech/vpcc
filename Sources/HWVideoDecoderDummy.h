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

#if 0

#include <cstdlib>

#include "HWVideoDecoder.h"

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
};

#endif

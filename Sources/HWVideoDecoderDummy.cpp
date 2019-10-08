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

#include "HWVideoDecoderDummy.h"

#if 0

HWVideoDecoder::HWVideoDecoder()
{
}

HWVideoDecoder::~HWVideoDecoder()
{
}

bool HWVideoDecoder::initialize(DecoderConfig& config)
{
    return true;
}

bool HWVideoDecoder::shutdown()
{
    return true;
}

bool HWVideoDecoder::start()
{
    return true;
}

bool HWVideoDecoder::stop()
{
    return true;
}

bool HWVideoDecoder::flush()
{
    return true;
}

bool HWVideoDecoder::queueVideoInputBuffer(uint8_t* data, size_t bytes, int64_t decodeTimeStamp, int64_t presentationTimeStamp, bool inputEOS)
{
    return true;
}

bool HWVideoDecoder::dequeueOutputBuffer()
{
    return true;
}

bool HWVideoDecoder::uploadTexture(CachedFrame& frame)
{
    return true;
}

#endif

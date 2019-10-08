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

#include "HWVideoDecoder.h"

HWVideoDecoderBase::HWVideoDecoderBase()
{
}

HWVideoDecoderBase::~HWVideoDecoderBase()
{
}

bool HWVideoDecoderBase::isInputQueueEmpty()
{
    _frameCacheMutex.lock();

    bool result = (_inputBuffers == 0);

    _frameCacheMutex.unlock();

    return result;
}

bool HWVideoDecoderBase::isInputQueueFull()
{
    _frameCacheMutex.lock();

    bool result = (_inputBuffers >= _config.inputBufferQueueSize);

    _frameCacheMutex.unlock();

    return result;
}

bool HWVideoDecoderBase::isOutputQueueEmpty()
{
    _frameCacheMutex.lock();

    // All free frames left -> output queue is empty
    bool result = (_freeOutputBuffers.size() == _config.outputBufferQueueSize);

    _frameCacheMutex.unlock();

    return result;
}

bool HWVideoDecoderBase::isOutputQueueFull()
{
    _frameCacheMutex.lock();

    // No free frames left -> output queue is full
    bool result = (_freeOutputBuffers.size() == 0);

    _frameCacheMutex.unlock();

    return result;
}

size_t HWVideoDecoderBase::getOutputQueueSize()
{
    return _outputBuffers.size();
}

CachedFrame* HWVideoDecoderBase::retainCachedFrame()
{
    _frameCacheMutex.lock();

    CachedFrame* frame = NULL;

    if (_outputBuffers.size() > 0)
    {
        frame = _outputBuffers.front();
        _outputBuffers.pop();

        uploadTexture(*frame);
    }

    _frameCacheMutex.unlock();

    return frame;
}

bool HWVideoDecoderBase::releaseCachedFrame(CachedFrame* frame)
{
    if (frame == NULL)
    {
        return false;
    }

    _frameCacheMutex.lock();

    CachedFrame::reset(frame);

    _freeOutputBuffers.push(frame);

    _frameCacheMutex.unlock();

    return true;
}

const DecoderConfig& HWVideoDecoderBase::getConfig() const
{
    return _config;
}

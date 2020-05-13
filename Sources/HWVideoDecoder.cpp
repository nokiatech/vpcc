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

#include "HighResolutionTimer.h"

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

bool HWVideoDecoderBase::isCachedFrameReady(int64_t presentationTimeStamp)
{
    bool result = false;

    _frameCacheMutex.lock();

    for (size_t i = 0; i < _outputBuffers.size(); ++i)
    {
        CachedFrame* frame = _outputBuffers.at(i);

        if (frame->pts == presentationTimeStamp)
        {
            result = true;

            break;
        }
    }

    _frameCacheMutex.unlock();

    return result;
}

CachedFrame* HWVideoDecoderBase::retainCachedFrame(int64_t presentationTimeStamp)
{
    _frameCacheMutex.lock();

    CachedFrame* frame = NULL;

    for (size_t i = 0; i < _outputBuffers.size(); ++i)
    {
        CachedFrame* cf = _outputBuffers.at(i);

        if (cf->pts == presentationTimeStamp)
        {
            frame = cf;

            _outputBuffers.erase(_outputBuffers.begin() + i);

            uploadTexture(*frame);

            break;
        }
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

const DecoderStats& HWVideoDecoderBase::getStats() const
{
    return _statistics;
}

void HWVideoDecoderBase::beginStatisticsScope()
{
    _numTotalFramesDecoded = 0;
    _totalFrameDecodingStartTime = HighResolutionTimer::getTimeMs();
}

void HWVideoDecoderBase::endStatisticsScope()
{
    _numTotalFramesDecoded = 0;
    _totalFrameDecodingStartTime = 0;
}

void HWVideoDecoderBase::printStatistics()
{
    // Print average stats for whole clip
    LOG_I("---------- DECODING STATS - BEGIN ----------");

    // Calculate average decoding stats
    int64_t totalFrameDecodingEndTime = HighResolutionTimer::getTimeMs();
    int64_t totalFrameDecodingTime = totalFrameDecodingEndTime - _totalFrameDecodingStartTime;

    int32_t averageFrameDuration = (int32_t)(totalFrameDecodingTime / _numTotalFramesDecoded);

    _statistics.numTotalFrames = (uint32_t)_numTotalFramesDecoded;
    _statistics.averageFPS = 1000.0f / (float)averageFrameDuration;
    _statistics.averageFrameDurationMs = averageFrameDuration;

    LOG_I("Total decoding time: %d", totalFrameDecodingTime);

    LOG_I("Total num frames decoded: %d (Decoder: %s)", _statistics.numTotalFrames, _config.name.c_str());
    LOG_I("Average %f fps (Decoder: %s)", _statistics.averageFPS, _config.name.c_str());
    LOG_I("Average %d ms / frame (Decoder: %s)", _statistics.averageFrameDurationMs, _config.name.c_str());

    LOG_I("---------- DECODING STATS - END ----------");
}

bool HWVideoDecoderBase::isValid() const
{
    return _initialized;
}

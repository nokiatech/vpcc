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

#include "Android/HWVideoDecoderMediaCodec.h"
#include "Android/JNIInterface.h"

#include "TGA.h"
#include "FileSystem.h"
#include "Helpers.h"
#include "Logger.h"
#include "HighResolutionTimer.h"
#include "HEVC.h"

#include <string>
#include <vector>

HWVideoDecoder::HWVideoDecoder()
: _inputFormat(NULL)
, _mediaCodec(NULL)
, _outputTexture(NULL)
{
}

HWVideoDecoder::~HWVideoDecoder()
{
}

bool HWVideoDecoder::initialize(DecoderConfig& config)
{
    _config = config;

    if (!_config.manualVideoTextureUpload)
    {
        // Create output texture
        assert(_outputTexture == NULL);

        _outputTexture = new OutputTexture();
        _outputTexture->surface = new Surface();

        jobject jsurface = _outputTexture->surface->getJavaObject();
        JNIEnv* jenv = getJNIEnv();

        _outputTexture->nativeWindow = ANativeWindow_fromSurface(jenv, jsurface);
    }

    // Create cache frames
    for (size_t i = 0; i < _config.outputBufferQueueSize; i++)
    {
        CachedFrame* frame = new CachedFrame();
        frame->width = _config.width;
        frame->height = _config.height;

        if (_config.manualVideoTextureUpload)
        {
            // Create Y texture
            glActiveTexture(GL_TEXTURE0);

            glGenTextures(1, &frame->yTextureHandle);
            glBindTexture(GL_TEXTURE_2D, frame->yTextureHandle);

            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, /*GL_LUMINANCE*/ GL_R8, frame->width, frame->height, 0, /*GL_LUMINANCE*/ GL_RED, GL_UNSIGNED_BYTE, NULL);

            GL_CHECK_ERRORS();

            // Create UV texture
            glGenTextures(1, &frame->uvTextureHandle);
            glBindTexture(GL_TEXTURE_2D, frame->uvTextureHandle);

            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // GL_LINEAR
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // GL_LINEAR
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8/*GL_LUMINANCE_ALPHA*/, frame->width / 2, frame->height / 2, 0, GL_RG/*GL_LUMINANCE_ALPHA*/, GL_UNSIGNED_BYTE, NULL);

            glBindTexture(GL_TEXTURE_2D, 0);

            GL_CHECK_ERRORS();
        }

        _freeOutputBuffers.push(frame);
    }

    // https://developer.android.com/reference/android/media/MediaCodec

    // Create media format
    _inputFormat = AMediaFormat_new();

    const std::string mimeType = "video/hevc";

    AMediaFormat_setString(_inputFormat, AMEDIAFORMAT_KEY_MIME, mimeType.c_str());

    AMediaFormat_setInt32(_inputFormat, AMEDIAFORMAT_KEY_WIDTH, _config.width);
    AMediaFormat_setInt32(_inputFormat, AMEDIAFORMAT_KEY_HEIGHT, _config.height);

    // HEVC configuration

    // Format        CSD buffer #0
    // H.265 HEVC    VPS (Video Parameter Sets*) + SPS (Sequence Parameter Sets*) + PPS (Picture Parameter Sets*)
    //
    // Each parameter set and the codec-specific-data sections marked with (*) must start with a start code of "\x00\x00\x00\x01".

    std::vector<uint8_t> decoderParameters;
    decoderParameters.insert(decoderParameters.end(), _config.parameters.vps.begin(), _config.parameters.vps.end());
    decoderParameters.insert(decoderParameters.end(), _config.parameters.sps.begin(), _config.parameters.sps.end());
    decoderParameters.insert(decoderParameters.end(), _config.parameters.pps.begin(), _config.parameters.pps.end());

    uint8_t* decoderConfig = decoderParameters.data();
    uint64_t decoderConfigSize = decoderParameters.size();

    AMediaFormat_setBuffer(_inputFormat, "csd-0", decoderConfig, decoderConfigSize);

    // Create media codec
    _mediaCodec = AMediaCodec_createDecoderByType(mimeType.c_str());

    // Configure media codec in async mode
    ANativeWindow* nativeWindow = NULL;

    if (_config.manualVideoTextureUpload)
    {
        nativeWindow = NULL;
    }
    else
    {
        nativeWindow = (ANativeWindow*)_outputTexture->nativeWindow;
    }

    media_status_t configureStatus = AMediaCodec_configure(_mediaCodec, _inputFormat, nativeWindow, NULL, 0);

    if (configureStatus != AMEDIA_OK)
    {
        LOG_E("VideoDecoder: %s, AMediaCodec_configure error", _config.name.c_str());

        return false;
    }

    return true;
}

bool HWVideoDecoder::shutdown()
{
    flush();

    // Stop decoding
    AMediaCodec_stop(_mediaCodec);

    // Destroy media format
    AMediaFormat_delete(_inputFormat);
    _inputFormat = NULL;

    // Destroy media codec
    AMediaCodec_delete(_mediaCodec);
    _mediaCodec = NULL;

    _frameCacheMutex.lock();

    while (_freeOutputBuffers.size())
    {
        CachedFrame* frame = _freeOutputBuffers.front();

        if (_config.manualVideoTextureUpload)
        {
            glDeleteTextures(1, &frame->yTextureHandle);
            frame->yTextureHandle = 0;

            glDeleteTextures(1, &frame->uvTextureHandle);
            frame->uvTextureHandle = 0;
        }

        _freeOutputBuffers.pop();
    }

    while (_outputBuffers.size())
    {
        CachedFrame* frame = _outputBuffers.front();

        if (_config.manualVideoTextureUpload)
        {
            glDeleteTextures(1, &frame->yTextureHandle);
            frame->yTextureHandle = 0;

            glDeleteTextures(1, &frame->uvTextureHandle);
            frame->uvTextureHandle = 0;
        }

        _outputBuffers.pop();
    }

    if (!_config.manualVideoTextureUpload)
    {
        delete _outputTexture;
        _outputTexture = NULL;
    }

    _frameCacheMutex.unlock();

    return true;
}

bool HWVideoDecoder::start()
{
    // Start decoding
    media_status_t startStatus = AMediaCodec_start(_mediaCodec);

    if (startStatus != AMEDIA_OK)
    {
        LOG_E("VideoDecoder: %s, AMediaCodec_start error", _config.name.c_str());

        return false;
    }

    return true;
}

bool HWVideoDecoder::stop()
{
    inputEOS = true;
    outputEOS = true;

    // Stop decoding
    AMediaCodec_stop(_mediaCodec);

    return true;
}

bool HWVideoDecoder::flush()
{
    media_status_t status = AMediaCodec_flush(_mediaCodec);

    if (status != AMEDIA_OK)
    {
        LOG_E("VideoDecoder: %s, AMediaCodec_flush error", _config.name.c_str());

        return false;
    }

    return true;
}

bool HWVideoDecoder::queueVideoInputBuffer(uint8_t* data, size_t bytes, int64_t decodeTimeStamp, int64_t presentationTimeStamp, bool inputEOS)
{
    if (data == NULL && bytes == 0)
    {
        return true;
    }
    else
    {
        assert(data != NULL);
        assert(bytes != 0);
    }

    // Fill input buffers
    size_t bytesRemaining = bytes;
    uint8_t* ptr = data;

    while (bytesRemaining > 0)
    {
        ssize_t inputBufferId = AMediaCodec_dequeueInputBuffer(_mediaCodec, 0);

        if (inputBufferId >= 0)
        {
            size_t inputBufferSize = 0;
            uint8_t* inputBuffer = AMediaCodec_getInputBuffer(_mediaCodec, inputBufferId, &inputBufferSize);

            size_t length = Math::min<size_t>(inputBufferSize, bytesRemaining);
            memcpy(inputBuffer, ptr, length);

            bytesRemaining -= length;
            ptr += length;

            // If EOS is reached, stop filling input buffers
            bool eos = (bytesRemaining == 0) ? inputEOS : false;

            media_status_t queueInputStatus = AMediaCodec_queueInputBuffer(_mediaCodec,
                                                                            inputBufferId,
                                                                            0,
                                                                            length,
                                                                            presentationTimeStamp,
                                                                            eos ? AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM : 0);

            LOG_D("VideoDecoder: %s, input buffer found: %zd, timestamp: %lld, length: %d, EOS: %d", _config.name.c_str(), inputBufferId, 0, bytes, (int32_t)eos);

            if (queueInputStatus != AMEDIA_OK)
            {
                LOG_E("VideoDecoder: %s, AMediaCodec_queueInputBuffer error", _config.name.c_str());

                return false;
            }

            _frameCacheMutex.lock();

            _inputBuffers++;

            _frameCacheMutex.unlock();

            return true;
        }
    }

    return false;
}

bool HWVideoDecoder::dequeueOutputBuffer()
{
    AMediaCodecBufferInfo outputBufferInfo;
    ssize_t outputBufferId = AMediaCodec_dequeueOutputBuffer(_mediaCodec, &outputBufferInfo, 0);

    if (outputBufferId >= 0)
    {
        // Get output format
        AMediaFormat* outputFormat = AMediaCodec_getOutputFormat(_mediaCodec);

        int32_t outputWidth = 0;
        int32_t outputHeight = 0;
        int32_t outputStride = 0;
        int32_t outputSliceHeight = 0;
        int32_t outputColorFormat = 0; // YUV420SemiPlanar = 21
        int32_t outputChannelCount = 0;
        int32_t outputCropLeft = 0;
        int32_t outputCropRight = 0;
        int32_t outputCropTop = 0;
        int32_t outputCropottom = 0;

        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_WIDTH, &outputWidth);
        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_HEIGHT, &outputHeight);
        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_STRIDE, &outputStride);
        //AMediaFormat_getInt32(outputFormat, XMEDIAFORMAT_KEY_SLICE, &outputSliceHeight);
        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_COLOR_FORMAT, &outputColorFormat);
        AMediaFormat_getInt32(outputFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, &outputChannelCount);
        //AMediaFormat_getInt32(outputFormat, XMEDIAFORMAT_KEY_CROP_LEFT, &outputCropLeft);
        //AMediaFormat_getInt32(outputFormat, XMEDIAFORMAT_KEY_CROP_RIGHT, &outputCropRight);
        //AMediaFormat_getInt32(outputFormat, XMEDIAFORMAT_KEY_CROP_TOP, &outputCropTop);
        //AMediaFormat_getInt32(outputFormat, XMEDIAFORMAT_KEY_CROP_BOTTOM, &outputCropottom);

        if (outputBufferInfo.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM)
        {
            outputEOS = true;
        }

        numTotalFramesDecoded++;

        _frameCacheMutex.lock();

        assert(_freeOutputBuffers.size() > 0);

        CachedFrame* cachedFrame = _freeOutputBuffers.front();
        cachedFrame->pts = outputBufferInfo.presentationTimeUs;
        cachedFrame->duration = 0; // TODO: Proper duration
        cachedFrame->width = outputWidth;
        cachedFrame->height = outputHeight;
        cachedFrame->outputBufferId = outputBufferId;

        if (_config.manualVideoTextureUpload)
        {
            size_t outputBufferSize = 0;
            uint8_t* outputBuffer = AMediaCodec_getOutputBuffer(_mediaCodec, outputBufferId, &outputBufferSize);

            if (outputBuffer != NULL)
            {
                cachedFrame->buffer = (uint8_t*)malloc(outputBufferSize);
                memcpy(cachedFrame->buffer, outputBuffer, outputBufferSize);
            }
        }

        _freeOutputBuffers.pop();
        _outputBuffers.push(cachedFrame);

        _inputBuffers--;

        _frameCacheMutex.unlock();

        AMediaFormat_delete(outputFormat);

        if (_config.manualVideoTextureUpload)
        {
            AMediaCodec_releaseOutputBuffer(_mediaCodec, outputBufferId, false);
        }

        return true;
    }
    else if (outputBufferId == AMEDIACODEC_INFO_OUTPUT_BUFFERS_CHANGED)
    {
        LOG_D("VideoDecoder: %s, Output buffers changed", _config.name.c_str());
    }
    else if (outputBufferId == AMEDIACODEC_INFO_OUTPUT_FORMAT_CHANGED)
    {
        AMediaFormat* format = AMediaCodec_getOutputFormat(_mediaCodec);
        LOG_D("VideoDecoder: %s, Output format changed to: %s", _config.name.c_str(), AMediaFormat_toString(format));
        AMediaFormat_delete(format);
    }
    else if (outputBufferId == AMEDIACODEC_INFO_TRY_AGAIN_LATER)
    {
        LOG_D("VideoDecoder: %s, No output buffer right now", _config.name.c_str());
    }
    else
    {
        LOG_D("VideoDecoder: %s, Unexpected info code: %zd", _config.name.c_str(), outputBufferId);
    }

    return false;
}

bool HWVideoDecoder::uploadTexture(CachedFrame& frame)
{
    if (frame.uploaded)
    {
        return true;
    }

    frame.uploaded = true;

    if (_config.manualVideoTextureUpload)
    {
        glActiveTexture(GL_TEXTURE0);

        // Create Y texture
        uint8_t* y = frame.buffer;
        size_t ySize = frame.width * frame.height;

        glBindTexture(GL_TEXTURE_2D, frame.yTextureHandle);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width, frame.height, GL_RED, GL_UNSIGNED_BYTE, y);

        // Create UV texture
        uint8_t* uv = frame.buffer + ySize;
        size_t uvSize = frame.width / 2 * frame.height / 2;

        glBindTexture(GL_TEXTURE_2D, frame.uvTextureHandle);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.width / 2, frame.height / 2, GL_RG, GL_UNSIGNED_BYTE, uv);

        glBindTexture(GL_TEXTURE_2D, 0);

        GL_CHECK_ERRORS();

        delete frame.buffer;
        frame.buffer = NULL;

        frame.target = GL_TEXTURE_2D;
    }
    else
    {
        media_status_t status = AMediaCodec_releaseOutputBuffer(_mediaCodec, frame.outputBufferId, true);

        if (status != AMEDIA_OK)
        {
            LOG_D("VideoDecoder: %s, output buffer release failed: %zd, timestamp: %lld", _config.name.c_str(), frame.outputBufferId, frame.pts);

            return false;
        }

        SurfaceTexture* surfaceTexture = _outputTexture->surface->getSurfaceTexture();
        surfaceTexture->updateTextImage();

        frame.target = GL_TEXTURE_EXTERNAL_OES;
        frame.yTextureHandle = surfaceTexture->getTexture();
    }

    return true;
}

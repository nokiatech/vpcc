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

#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <vector>

#include "PCCBitstream60.h"
#include "PCCParser60.h"
#include "PCCRenderer.h"
#include "FileSystem.h"

#if PLATFORM_ANDROID

    #include "Android/HWVideoDecoderMediaCodec.h"

#elif PLATFORM_IOS || PLATFORM_MACOS

    #include "Apple/HWVideoDecoderVideoToolbox.h"

#elif PLATFORM_WINDOWS

	#include "Windows/HWVideoDecoderMediaFoundation.h"

#else

    #include "HWVideoDecoderDummy.h"

#endif

class ARPlayer
{
public:

    struct Result
    {
        enum Enum
        {
            RESULT_ERROR = -1,
            RESULT_OK = 0,

            RESULT_NO_FRAME_AVAILABLE = 1,
            RESULT_OUT_OF_SYNC_FRAME_AVAILABLE = 2,
        };
    };

    struct Config
    {
        bool dualLayerMode = false;
        bool manualVideoTextureUpload = false;
    };

public:

    ARPlayer();
    ~ARPlayer();

    Result::Enum initialize(Config config);
    Result::Enum shutdown();

    Result::Enum open(std::string filename);
    Result::Enum play();
    Result::Enum stop();

    Result::Enum pause();
    Result::Enum resume();

    Result::Enum fetchPresentationFrame(PCCRenderer::PresentationFrame& presentationFrame);

    struct Stats
    {
        uint32_t numTotalFrames = 0;
        float averageFPS = 0.0f;
        uint32_t averageFrameDurationMs = 0;
    };

    struct StatsCollection
    {
        Stats geometry;
        Stats texture;
        Stats occupancy;
    };

    StatsCollection getStatsCollection() const;

private:

    static Config _config;

    std::string _filename;
    std::thread* _thread;

    struct State
    {
        enum Enum
        {
            INVALID = -1,
            INITIALIZED = 0,
            PLAYING = 1,
            STOPPED = 2,
            PAUSED = 3,
            SHUTDOWN = 4,
        };
    };

    State::Enum _state;

    struct PlaybackContext
    {
        std::mutex mutex;

        std::vector<PCC::FrameGroup> frameGroups;

        size_t outputFrameGroupIndex = 0;
        size_t outputFrameIndex = 0;

        size_t inputFrameGroupIndex = 0;
        size_t inputFrameIndex = 0;

        ///

        PCC::FrameGroup* getCurrentRenderFrameGroup();
        PCC::Frame* getCurrentRenderFrame();
        void proceedToNextRenderFrame();
        bool isOutputEOS();

        ///

        PCC::FrameGroup* getCurrentDecoderFrameGroup();
        PCC::Frame* getCurrentDecoderFrame();
        void proceedToNextDecoderFrame();
        bool isInputEOS();

        ///

        void resetPlaybackPosition();
    };

    PlaybackContext _playbackContext;

    struct VideoDecoderContext
    {
        HWVideoDecoder geometryVideoDecoder;
        HWVideoDecoder textureVideoDecoder;
        HWVideoDecoder occupancyVideoDecoder;

        StatsCollection stats;
    };

    VideoDecoderContext _videoDecoderContext;

    PCCRenderer::PresentationFrame _presentationFrame;

private:

    static void threadEntry(ARPlayer* player);

    static void queueInputBuffers(PlaybackContext& playbackContext, VideoDecoderContext& videoDecoderContext);
    static void queueOutputBuffers(VideoDecoderContext& videoDecoderContext);

    static bool queueInputBuffer(uint8_t* buffer, size_t bytes, HWVideoDecoder& videoDecoder, int64_t presentationTimeUs, bool inputEOS);

    static bool shouldQueueInput(VideoDecoderContext& videoDecoderContext);

    void releasePresentationFrame(VideoDecoderContext& videoDecoderContext, PCCRenderer::PresentationFrame& presentationFrame);

    bool isNextPresentationFrameAvailable(VideoDecoderContext& videoDecoderContext);

    bool isPresentationFrameCompleted(PCCRenderer::PresentationFrame& presentationFrame);
    bool isValidPresentationFrame(PCCRenderer::PresentationFrame& presentationFrame);
};

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

#include "VPCC/VPCCParser130.h"

#include "VPCC/VPCCRenderer.h"

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

namespace VPCC = VPCC130;

class VPCCPlayer
{
public:

    struct Result
    {
        enum Enum
        {
            RESULT_ERROR = -1,

            RESULT_OK,

            RESULT_NEW_FRAME,
            RESULT_OLD_FRAME,

            RESULT_NO_FRAME_AVAILABLE,
            RESULT_OUT_OF_SYNC_FRAME_AVAILABLE,

            RESULT_EOS,
        };
    };

    struct Config
    {
        bool manualVideoTextureUpload = false;
    };
    
    struct State
    {
        enum Enum
        {
            INVALID = -1,

            INITIALIZED,
            PLAYING,
            STOPPED,
            PAUSED,
            SHUTDOWN,

            COUNT
        };
    };

public:

    VPCCPlayer();
    ~VPCCPlayer();

    Result::Enum initialize(Config config);
    Result::Enum shutdown();

    Result::Enum open(std::string filename);
    Result::Enum play();
    Result::Enum stop();

    Result::Enum pause();
    Result::Enum resume();

    void restart();
    
    State::Enum GetState();

    Result::Enum fetchPresentationFrame(VPCCRenderer::PresentationFrame& presentationFrame);

private:

    static Config _config;

    std::string _filename;
    std::thread* _thread;

    State::Enum _state = State::INVALID;

    struct PlaybackContext
    {
        std::recursive_mutex mutex;

        std::vector<VPCC::FrameGroup> frameGroups;

        uint64_t currentFramePTS = -1;

        size_t outputFrameGroupIndex = 0;
        size_t outputFrameIndex = 0;

        size_t inputFrameGroupIndex[VPCC::VideoType::COUNT] = { 0 };
        size_t inputPacketIndex[VPCC::VideoType::COUNT] = { 0 };

        static const int64_t MAX_GOP_SIZE = 32;
        int64_t slots[VPCC::VideoType::COUNT][MAX_GOP_SIZE] = { 0 };

        HWVideoDecoder videoDecoders[VPCC::VideoType::COUNT];

        bool inputEOS = false;
        bool outputEOS = false;

        ///

        VPCC::FrameData* getCurrentFrame();

        void proceedToNextFrame();

        bool isOutputEOS();
        bool isInputEOS();

        ///

        VPCC::FrameGroup* getFrameGroup(VPCC::VideoType::Enum type);
        VPCC::VideoFramePacket* getPacket(VPCC::VideoType::Enum type);

        void proceedToNextPacket(VPCC::VideoType::Enum type);
        bool isLastPacket(VPCC::VideoType::Enum type);
        bool isInputEOS(VPCC::VideoType::Enum type);

        ///

        void initializeDecoders();
        void shutdownDecoders();

        void reset();

        HWVideoDecoder& getDecoderByType(VPCC::VideoType::Enum type);

        uint64_t getFramePTS();
        uint64_t getNextFramePTS();
    };

    PlaybackContext _playbackContext;

    VPCCRenderer::PresentationFrame _presentationFrame;

private:

    static void threadEntry(VPCCPlayer* player);

    static void queueInputBuffers(PlaybackContext& playbackContext);
    static void queueOutputBuffers(PlaybackContext& playbackContext);

    static bool queueInputPacket(VPCC::VideoStream& stream, VPCC::VideoFramePacket& packet, HWVideoDecoder& videoDecoder, PlaybackContext& playbackContext, bool inputEOS);

    void releasePresentationFrame(VPCCRenderer::PresentationFrame& presentationFrame);

    bool isNextPresentationFrameReady(PlaybackContext& playbackContext);

    bool isPresentationFrameCompleted(VPCCRenderer::PresentationFrame& presentationFrame);
    bool isValidPresentationFrame(VPCCRenderer::PresentationFrame& presentationFrame);

    void initializeVideoDecoder(VPCC::VideoStream& stream, HWVideoDecoder& decoder);

    bool isEOS(PlaybackContext& playbackContext);
};

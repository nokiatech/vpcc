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

#include "VPCC/VPCCPlayer.h"

#include "TGA.h"
#include "FileSystem.h"
#include "Helpers.h"
#include "HEVC.h"
#include "Logger.h"
#include "HighResolutionTimer.h"

#if PLATFORM_ANDROID

#include "Android/JNIInterface.h"

#endif

#define DEBUG_DUMP_RAW_COMPRESSED_YUV_FRAME 0

namespace
{
    // Limit max values to buffer queue sizes.
#if PLATFORM_ANDROID

    int32_t inputBufferQueueSize = 16; // E.g. Huawei devices need up to 10 input samples to be pre-buffered before output is decoded.
    int32_t outputBufferQueueSize = 8;

#elif PLATFORM_IOS

    // iOS devices are able to decode output in startup without pre-buffering.
    int32_t inputBufferQueueSize = 8;
    int32_t outputBufferQueueSize = 8;

#elif PLATFORM_MACOS

    // macOS devices are able to decode output in startup without pre-buffering.
    int32_t inputBufferQueueSize = 8;
    int32_t outputBufferQueueSize = 8;

#elif PLATFORM_WINDOWS

    int32_t inputBufferQueueSize = 16; // HEVC decoder is not able to decode output without input sample pre-buffering in startup.
    int32_t outputBufferQueueSize = 16;

#endif
}

/////////////////

VPCCPlayer::Config VPCCPlayer::_config;

void VPCCPlayer::threadEntry(VPCCPlayer* player)
{
#if PLATFORM_ANDROID

    attachThread();

#endif

    // Read whole file to memory buffer
    IOBuffer buffer = FileSystem::loadFromDisk(player->_filename);
    assert(buffer.data != NULL);

    if (buffer.size == 0)
    {
        return;
    }
    
    PlaybackContext& playbackContext = player->_playbackContext;

    // Parse all frame groups at once
    bool result = VPCC::parseFrameGroups(buffer, playbackContext.frameGroups, false);
    
    if (!result)
    {
        LOG_E("Frame groups parsing failed!");

        assert(false);
        
        return;
    }
    
    size_t frameGroupCount = 0;
    size_t totalFrameCount = 0;
    
    for (size_t frameGroupIndex = 0; frameGroupIndex < playbackContext.frameGroups.size(); ++frameGroupIndex)
    {
        VPCC::FrameGroup& frameGroup = playbackContext.frameGroups[frameGroupIndex];
        
        frameGroupCount += 1;
        totalFrameCount += frameGroup.frames.size();
    }
    
    LOG_I("Number of frame groups: %d", frameGroupCount);
    LOG_I("Total number of frames: %d", totalFrameCount);

    // Verify that all frame groups and all the frames have same dimensions.
    // Note: If frame size changes between frame groups HW decoders need to be recreated which is slow (> 100ms per instance).
    size_t previousFrameGroupWidth = 0;
    size_t previousFrameGroupHeight = 0;
    
    bool error = false;
    
    for (size_t frameGroupIndex = 0; frameGroupIndex < playbackContext.frameGroups.size(); ++frameGroupIndex)
    {
        VPCC::FrameGroup& frameGroup = playbackContext.frameGroups[frameGroupIndex];
        
        size_t frameGroupWidth = (frameGroup.frames.size() > 0) ? frameGroup.frames[0].width : 0;
        size_t frameGroupHeight = (frameGroup.frames.size() > 0) ? frameGroup.frames[0].height : 0;
        
        if (frameGroupIndex == 0)
        {
            previousFrameGroupWidth = frameGroupWidth;
            previousFrameGroupHeight = frameGroupHeight;
        }
        
        if (frameGroupWidth == previousFrameGroupWidth && frameGroupHeight == previousFrameGroupHeight)
        {
            LOG_D("Frame group #%llu size (width & height): %d x %d", frameGroupIndex, frameGroupWidth, frameGroupHeight);
        }
        else
        {
            LOG_D("Frame group #%llu size (width & height): %d x %d are not consistent!", frameGroupIndex, frameGroupWidth, frameGroupHeight);
            
            error = true;
        }
    }
    
    if (error)
    {
        return;
    }

    // Start playback loop
    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        HWVideoDecoder& decoder = playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            decoder.beginStatisticsScope();
        }
    }

    player->_state = State::PLAYING;

    while (true)
    {
        if (player->_state == State::PLAYING)
        {
            // Queue input buffers
            if (!playbackContext.isInputEOS())
            {
                queueInputBuffers(playbackContext);
            }
            else
            {
                playbackContext.inputEOS = true;
            }

            // Wait until input EOS and output EOS is reached
            if (playbackContext.inputEOS && playbackContext.isOutputEOS())
            {
                playbackContext.outputEOS = true;
            }

            // Query output buffers
            if (!playbackContext.outputEOS)
            {
                queueOutputBuffers(playbackContext);
            }
        }
        else if (player->_state == State::SHUTDOWN)
        {
            break;
        }

        std::this_thread::yield();
    }

#if PLATFORM_ANDROID

    detachThread();

#endif
}

////////////////////////////////////////

VPCCPlayer::VPCCPlayer()
: _thread(NULL)
{
}

VPCCPlayer::~VPCCPlayer()
{
}

VPCCPlayer::Result::Enum VPCCPlayer::initialize(Config config)
{
    _config = config;

    return VPCCPlayer::Result::RESULT_OK;
}

VPCCPlayer::Result::Enum VPCCPlayer::shutdown()
{
    _state = State::SHUTDOWN;

    // Wait for shutdown
    _thread->join();

    delete _thread;
    _thread = NULL;

    // Destroy decoders
    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        HWVideoDecoder& decoder = _playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            decoder.shutdown();
        }
    }

    // Release retained frame
    releasePresentationFrame(_presentationFrame);
    
    _state = State::INVALID;

    return VPCCPlayer::Result::RESULT_OK;
}

void VPCCPlayer::initializeVideoDecoder(VPCC::VideoStream& stream, HWVideoDecoder& decoder)
{
    DecoderConfig decoderConfig;
    decoderConfig.parameters = stream.decoderParameters;
    decoderConfig.width = stream.sps.picWidthInLumaSamples;
    decoderConfig.height = stream.sps.picHeightInLumaSamples;
    decoderConfig.name = VPCC::VideoType::toString(stream.type);
    decoderConfig.inputBufferQueueSize = inputBufferQueueSize;
    decoderConfig.outputBufferQueueSize = outputBufferQueueSize;
    decoderConfig.manualVideoTextureUpload = _config.manualVideoTextureUpload;

    LOG_I("---------- DECODER CONFIG - BEGIN ----------");

    LOG_I("Decoder: %s", decoderConfig.name.c_str());
    LOG_I("Frame width: %d", decoderConfig.width);
    LOG_I("Frame height: %d", decoderConfig.height);

    LOG_I("Profile: %s", HEVC::profileName(stream.vps.profileTierLevel.generalProfileIdc));
    LOG_I("Tier: %s", HEVC::tierName(stream.vps.profileTierLevel.generalTierFlag));
    LOG_I("Level: %d", stream.vps.profileTierLevel.generalLevelIdc / 30);

    LOG_I("---------- DECODER CONFIG - END ----------");

    bool result = decoder.initialize(decoderConfig);
    assert(result);

    decoder.start();
}

VPCCPlayer::Result::Enum VPCCPlayer::open(std::string filename)
{
    // Read file
    IOBuffer buffer = FileSystem::loadFromDisk(filename);
    assert(buffer.data != NULL);

    if (buffer.size == 0)
    {
        LOG_E("File (%s) not found", filename.c_str());

        return VPCCPlayer::Result::RESULT_ERROR;
    }

    LOG_I("File (%s) loaded", filename.c_str());

    _filename = filename;
    
    // Parse first frame group to get decoder config parameters
    std::vector<VPCC::FrameGroup> frameGroups;

    if (VPCC::parseFrameGroups(buffer, frameGroups, true))
    {
        IOBuffer::free(&buffer);

        bool dualLayer = false;
        
        VPCC::FrameGroup& frameGroup = frameGroups.at(0);

        if (frameGroup.videoStream[VPCC::VideoType::GEOMETRY].packets.size() > 0)
        {
            dualLayer = (frameGroup.videoStream[VPCC::VideoType::OCCUPANCY].packets.size() * 2 == frameGroup.videoStream[VPCC::VideoType::GEOMETRY].packets.size());
        }

        if (dualLayer)
        {
            LOG_E("Dual-layer file format is not supported! (%s)", filename.c_str());

            return VPCCPlayer::Result::RESULT_ERROR;
        }

        ////////////////////////////////////////////////////////////////////////////
        ///
        /// Note: E.g. Android AMediaCodec needs to be inialized from UI/rendering thread
        ///
        ////////////////////////////////////////////////////////////////////////////
        for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
        {
            HWVideoDecoder& decoder = _playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);
            VPCC::VideoStream& stream = frameGroup.videoStream[i];

            if (stream.type != VPCC::VideoType::INVALID)
            {
                initializeVideoDecoder(stream, decoder);
            }
        }

        _state = State::INITIALIZED;

        return VPCCPlayer::Result::RESULT_OK;
    }

    IOBuffer::free(&buffer);

    return VPCCPlayer::Result::RESULT_ERROR;
}

VPCCPlayer::Result::Enum VPCCPlayer::play()
{
    // Start thread
    _thread = new std::thread(VPCCPlayer::threadEntry, this);

    return VPCCPlayer::Result::RESULT_OK;
}

VPCCPlayer::Result::Enum VPCCPlayer::stop()
{
    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        HWVideoDecoder& decoder = _playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            decoder.stop();
        }
    }

    _state = State::STOPPED;

    return VPCCPlayer::Result::RESULT_OK;
}

VPCCPlayer::Result::Enum VPCCPlayer::pause()
{
    _state = State::PAUSED;

    return VPCCPlayer::Result::RESULT_OK;
}

VPCCPlayer::Result::Enum VPCCPlayer::resume()
{
    _state = State::PLAYING;

    return VPCCPlayer::Result::RESULT_OK;
}

bool VPCCPlayer::isPresentationFrameCompleted(VPCCRenderer::PresentationFrame& presentationFrame)
{
    static uint64_t frameCounter = 0;

	// TODO: Clock + pts + frame duration based frame index calculation 
#if PLATFORM_ANDROID

    static const uint64_t FRAME_DURATION = 1;

#elif PLATFORM_IOS

    static const uint64_t FRAME_DURATION = 1;

#elif  PLATFORM_MACOS || PLATFORM_WINDOWS

	static const uint64_t FRAME_DURATION = 1;

#endif

    if (frameCounter >= FRAME_DURATION)
    {
        frameCounter = 0;

        return true;
    }

    frameCounter++;

    return false;
}

bool VPCCPlayer::isValidPresentationFrame(VPCCRenderer::PresentationFrame& presentationFrame)
{
    if (presentationFrame.depth0 != NULL &&
        presentationFrame.color0 != NULL &&
        presentationFrame.occupancy != NULL)
    {
        return true;
    }

    return false;
}

bool VPCCPlayer::isNextPresentationFrameReady(PlaybackContext& playbackContext)
{
    bool result = true;

    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        HWVideoDecoder& decoder = playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            result &= decoder.isCachedFrameReady(playbackContext.getNextFramePTS());
        }
    }

    return result;
}

bool VPCCPlayer::isEOS(PlaybackContext& playbackContext)
{
    if (_state == State::PLAYING)
    {
        return (playbackContext.inputEOS && playbackContext.outputEOS);
    }

    return false;
}

void VPCCPlayer::restart()
{
    _playbackContext.mutex.lock();
    {
        for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
        {
            HWVideoDecoder& decoder = _playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

            if (decoder.isValid())
            {
                decoder.printStatistics();
                decoder.endStatisticsScope();

                decoder.flush();
            }
        }

        _playbackContext.reset();

        // Set parser context states
        _playbackContext.inputEOS = false;
        _playbackContext.outputEOS = false;
    }
    _playbackContext.mutex.unlock();
}

VPCCPlayer::State::Enum VPCCPlayer::GetState()
{
    return _state;
}

VPCCPlayer::Result::Enum VPCCPlayer::fetchPresentationFrame(VPCCRenderer::PresentationFrame& presentationFrame)
{
    std::lock_guard<std::recursive_mutex> lock(_playbackContext.mutex);
    {
        if (_state == State::PAUSED)
        {
            if (isValidPresentationFrame(_presentationFrame))
            {
                presentationFrame = _presentationFrame;

                return Result::RESULT_OLD_FRAME;
            }
            else
            {
                return Result::RESULT_NO_FRAME_AVAILABLE;
            }
        }
        else if (_state != State::PLAYING)
        {
            return Result::RESULT_NO_FRAME_AVAILABLE;
        }

        // Check if current presentation frame is valid and frame is not completed (presentation timestamp + duration)
        if (isValidPresentationFrame(_presentationFrame) && !isPresentationFrameCompleted(_presentationFrame))
        {
            presentationFrame = _presentationFrame;

            return Result::RESULT_OLD_FRAME;
        }

        // Check if new presentation frame is available
        bool nextFrameAvailable = isNextPresentationFrameReady(_playbackContext);

        if (!nextFrameAvailable)
        {
            if (isEOS(_playbackContext))
            {
                if (isValidPresentationFrame(_presentationFrame))
                {
                    releasePresentationFrame(_presentationFrame);

                    _playbackContext.proceedToNextFrame();
                }

                return Result::RESULT_EOS;
            }

            if (isValidPresentationFrame(_presentationFrame))
            {
                presentationFrame = _presentationFrame;

                return Result::RESULT_OLD_FRAME;
            }
            else
            {
                return Result::RESULT_NO_FRAME_AVAILABLE;
            }
        }

        // Release old presentation frame and create a new one
        releasePresentationFrame(_presentationFrame);

        uint64_t pts = _playbackContext.getNextFramePTS();

        CachedFrame* geometry0 = _playbackContext.getDecoderByType(VPCC::VideoType::GEOMETRY).retainCachedFrame(pts);
        CachedFrame* texture0 = _playbackContext.getDecoderByType(VPCC::VideoType::TEXTURE).retainCachedFrame(pts);
        CachedFrame* occupancy = _playbackContext.getDecoderByType(VPCC::VideoType::OCCUPANCY).retainCachedFrame(pts);

        // TODO: Current expecting that frame drops does not exist
        bool isSync = true;

        isSync &= (geometry0->pts == texture0->pts);
        isSync &= (geometry0->pts == occupancy->pts);

        if (isSync)
        {
            _presentationFrame.depth0 = geometry0;
            _presentationFrame.color0 = texture0;
            _presentationFrame.occupancy = occupancy;

            VPCC::FrameData* frame = _playbackContext.getCurrentFrame(); // This should always succeed at this point
            assert(frame != NULL);

            _presentationFrame.patches = frame->patches;
            _presentationFrame.blockToPatch = frame->blockToPatch;

            presentationFrame = _presentationFrame;

            _playbackContext.proceedToNextFrame();

            LOG_V("Current frame pts: depth0 %lu, color0 %lu, occupancy %lu", _presentationFrame.depth0->pts, _presentationFrame.color0->pts, _presentationFrame.occupancy->pts);

            return Result::RESULT_NEW_FRAME;
        }
        else
        {
            return Result::RESULT_OUT_OF_SYNC_FRAME_AVAILABLE;
        }
    }
}

void VPCCPlayer::releasePresentationFrame(VPCCRenderer::PresentationFrame& presentationFrame)
{
    _playbackContext.getDecoderByType(VPCC::VideoType::GEOMETRY).releaseCachedFrame(presentationFrame.depth0);
    _playbackContext.getDecoderByType(VPCC::VideoType::TEXTURE).releaseCachedFrame(presentationFrame.color0);
    _playbackContext.getDecoderByType(VPCC::VideoType::OCCUPANCY).releaseCachedFrame(presentationFrame.occupancy);

    presentationFrame.depth0 = NULL;
    presentationFrame.depth1 = NULL;
    presentationFrame.color0 = NULL;
    presentationFrame.color1 = NULL;
    presentationFrame.occupancy = NULL;

    presentationFrame.patches.clear();
    presentationFrame.blockToPatch.clear();
}

void VPCCPlayer::queueInputBuffers(PlaybackContext& playbackContext)
{
    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        VPCC::FrameGroup* frameGroup = playbackContext.getFrameGroup((VPCC::VideoType::Enum)i);

        if (frameGroup != NULL)
        {
            HWVideoDecoder& decoder = playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

            if (decoder.isValid())
            {
                bool queueInput = !decoder.isInputQueueFull() && !decoder.isOutputQueueFull();

                if (queueInput)
                {
                    bool inputEOS = playbackContext.isLastPacket((VPCC::VideoType::Enum)i);

                    VPCC::VideoStream& stream = frameGroup->videoStream[i];
                    VPCC::VideoFramePacket* packet = playbackContext.getPacket((VPCC::VideoType::Enum)i);

                    if (packet == NULL)
                    {
                        return;
                    }

                    bool succeed = queueInputPacket(stream, *packet, decoder, playbackContext, inputEOS);

                    if (succeed)
                    {
                        playbackContext.proceedToNextPacket((VPCC::VideoType::Enum)i);
                    }
                    else
                    {
                        // TODO: What if queue operation fails... flush video decoder?
                    }
                }
            }
        }
    }
}

bool VPCCPlayer::queueInputPacket(VPCC::VideoStream& stream, VPCC::VideoFramePacket& packet, HWVideoDecoder& videoDecoder, PlaybackContext& playbackContext, bool inputEOS)
{
    int64_t decodeTimeStamp = 0;
    int64_t presentationTimeStamp = 0;

    // TODO: Currently just using running number
    decodeTimeStamp = packet.sliceIndex;

    // TODO: Currently just using running number
    int64_t generation = playbackContext.slots[stream.type][packet.pictureOrderCount];

    if (generation != 0)
    {
        presentationTimeStamp = packet.pictureOrderCount + (playbackContext.MAX_GOP_SIZE * generation);
    }
    else
    {
        presentationTimeStamp = packet.pictureOrderCount;
    }

    playbackContext.slots[stream.type][packet.pictureOrderCount] += 1;

    // Queue data
    uint8_t* buffer = stream.buffer.data() + packet.offset;
    size_t bytes = packet.length;

#if DEBUG_DUMP_RAW_COMPRESSED_YUV_FRAME

    std::string outputPath;
    outputPath.append(std::to_string(presentationTimeStamp));
    outputPath.append("_");
    outputPath.append(videoDecoder.getConfig().name);
    outputPath.append("_frame.265");

    FileSystem::saveToDisk(outputPath.c_str(), buffer, bytes);

#endif

    return videoDecoder.queueVideoInputBuffer(buffer, bytes, decodeTimeStamp, presentationTimeStamp, inputEOS);
}

void VPCCPlayer::queueOutputBuffers(PlaybackContext& playbackContext)
{
    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        HWVideoDecoder& decoder = playbackContext.getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            if (!decoder.isOutputQueueFull())
            {
                decoder.dequeueOutputBuffer();
            }
        }
    }
}

///

VPCC::FrameData* VPCCPlayer::PlaybackContext::getCurrentFrame()
{
    if (outputFrameGroupIndex < frameGroups.size())
    {
        VPCC::FrameGroup& frameGroup = frameGroups[outputFrameGroupIndex];
        std::vector<VPCC::FrameData>& frames = frameGroup.frames;

        if (outputFrameIndex < frames.size())
        {
            return &frames[outputFrameIndex];
        }
    }

    return NULL;
}

void VPCCPlayer::PlaybackContext::proceedToNextFrame()
{
    if (outputFrameGroupIndex < frameGroups.size())
    {
        VPCC::FrameGroup& frameGroup = frameGroups[outputFrameGroupIndex];
        std::vector<VPCC::FrameData>& frames = frameGroup.frames;

        currentFramePTS++;

        outputFrameIndex++;

        if (outputFrameIndex >= frames.size())
        {
            outputFrameIndex = 0;
            outputFrameGroupIndex++;
        }
    }
}

bool VPCCPlayer::PlaybackContext::isOutputEOS()
{
    if (frameGroups.size() == 0)
    {
        return false;
    }

    if (outputFrameGroupIndex >= frameGroups.size())
    {
        return true;
    }

    return false;
}

bool VPCCPlayer::PlaybackContext::isInputEOS()
{
    bool result = true;

    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        HWVideoDecoder& decoder = getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            result &= isInputEOS((VPCC::VideoType::Enum)i);
        }
    }

    return result;
}

///

VPCC::FrameGroup* VPCCPlayer::PlaybackContext::getFrameGroup(VPCC::VideoType::Enum type)
{
    size_t frameGroupIndex = inputFrameGroupIndex[type];

    if (frameGroupIndex < frameGroups.size())
    {
        return &frameGroups[frameGroupIndex];
    }

    return NULL;
}

VPCC::VideoFramePacket* VPCCPlayer::PlaybackContext::getPacket(VPCC::VideoType::Enum type)
{
    size_t frameGroupIndex = inputFrameGroupIndex[type];

    if (frameGroupIndex < frameGroups.size())
    {
        VPCC::FrameGroup& frameGroup = frameGroups[frameGroupIndex];
        VPCC::VideoStream& videoStream = frameGroup.videoStream[type];

        size_t packetIndex = inputPacketIndex[type];

        if (packetIndex < videoStream.packets.size())
        {
            return &videoStream.packets[packetIndex];
        }
    }

    return NULL;
}

void VPCCPlayer::PlaybackContext::proceedToNextPacket(VPCC::VideoType::Enum type)
{
    size_t frameGroupIndex = inputFrameGroupIndex[type];

    if (frameGroupIndex < frameGroups.size())
    {
        VPCC::FrameGroup& frameGroup = frameGroups[frameGroupIndex];
        VPCC::VideoStream& videoStream = frameGroup.videoStream[type];

        inputPacketIndex[type]++;

        if (inputPacketIndex[type] >= videoStream.packets.size())
        {
            inputPacketIndex[type] = 0;
            inputFrameGroupIndex[type]++;
        }
    }
}

bool VPCCPlayer::PlaybackContext::isInputEOS(VPCC::VideoType::Enum type)
{
    if (frameGroups.size() == 0)
    {
        return false;
    }

    if (inputFrameGroupIndex[type] >= frameGroups.size())
    {
        return true;
    }

    return false;
}

bool VPCCPlayer::PlaybackContext::isLastPacket(VPCC::VideoType::Enum type)
{
    size_t frameGroupIndex = inputFrameGroupIndex[type];

    if ((frameGroupIndex + 1) >= frameGroups.size())
    {
        VPCC::FrameGroup& frameGroup = frameGroups[frameGroupIndex];
        VPCC::VideoStream& videoStream = frameGroup.videoStream[type];

        if ((inputPacketIndex[type] + 1) >= videoStream.packets.size())
        {
            return true;
        }
    }

    return false;
}

///

void VPCCPlayer::PlaybackContext::reset()
{
    currentFramePTS = -1;

    outputFrameGroupIndex = 0;
    outputFrameIndex = 0;

    memset(slots, 0, sizeof(slots));

    for (size_t i = 0; i < VPCC::VideoType::COUNT; ++i)
    {
        inputFrameGroupIndex[i] = 0;
        inputPacketIndex[i] = 0;

        HWVideoDecoder& decoder = getDecoderByType((VPCC::VideoType::Enum)i);

        if (decoder.isValid())
        {
            decoder.flush();
        }
    }
}

void VPCCPlayer::PlaybackContext::initializeDecoders()
{

}

void VPCCPlayer::PlaybackContext::shutdownDecoders()
{

}

HWVideoDecoder& VPCCPlayer::PlaybackContext::getDecoderByType(VPCC::VideoType::Enum type)
{
    return videoDecoders[type];
}

uint64_t VPCCPlayer::PlaybackContext::getFramePTS()
{
    return currentFramePTS;
}

uint64_t VPCCPlayer::PlaybackContext::getNextFramePTS()
{
    return currentFramePTS + 1;
}

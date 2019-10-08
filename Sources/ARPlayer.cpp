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

#include "ARPlayer.h"

#include "TGA.h"
#include "FileSystem.h"
#include "Helpers.h"
#include "HEVC.h"
#include "Logger.h"
#include "HighResolutionTimer.h"

#include "PCCParser60.h"
#include "PCCBitstream60.h"

#if PLATFORM_ANDROID

#include "Android/JNIInterface.h"

#endif

#define DEBUG_DUMP_RAW_COMPRESSED_YUV_FRAME 0
#define DEBUG_DECODE_FIRST_FRAME_ONLY 0
#define DEBUG_PCC_DECODER 0

#define MIN_OUTPUT_QUEUE_SIZE 1

// Limit max values to buffer queue sizes.
#if PLATFORM_ANDROID

int32_t inputBufferQueueSize = 10; // E.g. Huawei devices need up to 10 input samples to be pre-buffered before output is decoded.
int32_t outputBufferQueueSize = 5;

#elif PLATFORM_IOS

// iOS devices are able to decode output in startup without pre-buffering.
int32_t inputBufferQueueSize = 5;
int32_t outputBufferQueueSize = 5;

#elif PLATFORM_MACOS

// macOS devices are able to decode output in startup without pre-buffering.
int32_t inputBufferQueueSize = 5;
int32_t outputBufferQueueSize = 5;

#elif PLATFORM_WINDOWS

int32_t inputBufferQueueSize = 10; // HEVC decoder is not able to decode output without input sample pre-buffering in startup.
int32_t outputBufferQueueSize = 10;

#endif

/////////////////

#if DEBUG_PCC_DECODER

#include "PCCCommon.h"
#include "PCCChrono.h"
#include "PCCDecoder.h"
#include "PCCContext.h"
#include "PCCFrameContext.h"
#include "PCCBitstream.h"
#include "PCCGroupOfFrames.h"

int decompressVideo(pcc::PCCBitstream& bitstream, const pcc::PCCDecoderParameters& decoderParams/*, const pcc::PCCMetricsParameters& metricsParams*/)
{
    if (!bitstream.readHeader())
    {
        return -1;
    }

    size_t frameNumber = decoderParams.startFrameNumber_;
    std::vector<std::vector<uint8_t>> checksumsRec, checksumsDec;

    pcc::PCCDecoder decoder;
    decoder.setParameters(decoderParams);

    while (bitstream.size() < bitstream.capacity())
    {
        pcc::PCCGroupOfFrames reconstructs;
        pcc::PCCContext context;

        int ret = decoder.decode(bitstream, context, reconstructs);

        if (ret)
        {
            return ret;
        }
    }

    bitstream.getBitStreamStat().trace();

    return 0;
}

int debugDecode(const pcc::PCCDecoderParameters& params, IOBuffer& ioBuffer)
{
    pcc::PCCBitstream bitstream;
    bitstream.initialize(ioBuffer.data, ioBuffer.size);

    pcc::PCCDecoderParameters decoderParams;
    int ret = decompressVideo(bitstream, decoderParams/*, metricsParams, clockUser*/);

    return ret;
}

#endif

///////////////////

ARPlayer::Config ARPlayer::_config;

void ARPlayer::threadEntry(ARPlayer* player)
{
#if PLATFORM_ANDROID

    attachThread();

#endif

    // Read whole file to memory buffer
    IOBuffer ioBuffer = FileSystem::loadFromDisk(player->_filename);
    assert(ioBuffer.data != NULL);

    if (ioBuffer.size == 0)
    {
        return;
    }

    PCC::Bitstream bitstream(ioBuffer.data, ioBuffer.size);

    PCC::TMC2Header header;

    if (!PCC::parseContainerHeader(bitstream, header))
    {
        return;
    }

    // Parse all frame groups at once
    PlaybackContext& playbackContext = player->_playbackContext;

    size_t frameGroupCount = 0;
    size_t totalFrameCount = 0;

    while (size_t bytesLeft = PCC::BitstreamReader::bytesAvailable(bitstream))
    {
        PCC::FrameGroup frameGroup;
        PCC::parse(bitstream, frameGroup);

        // Generate pts manually
        for (size_t i = 0; i < frameGroup.frames.size(); i++)
        {
            PCC::Frame& frame = frameGroup.frames[i];
            frame.presentationTimeUs = ((totalFrameCount / 32) * 100) + i % 32;

            totalFrameCount++;
        }

        playbackContext.frameGroups.push_back(frameGroup);

        frameGroupCount++;
    }

    LOG_I("Number of frame groups: %d", frameGroupCount);
    LOG_I("Total number of frames: %d", totalFrameCount);

    for (size_t i = 0; i < playbackContext.frameGroups.size(); ++i)
    {
        PCC::FrameGroup& current = playbackContext.frameGroups[i];

        LOG_E("Frame group size (width & height): %d x %d", current.sps.frameWidth, current.sps.frameHeight);
    }

    // Verify that all frame groups and all the frames have same dimensions.
    // Note: If frame size changes between frame groups HW decoders need to be recreated which is slow (> 100ms per instance).
    for (size_t i = 1; i < playbackContext.frameGroups.size(); ++i)
    {
        PCC::FrameGroup& previous = playbackContext.frameGroups[i - 1];
        PCC::FrameGroup& current = playbackContext.frameGroups[i];

        if ((previous.sps.frameWidth != 0 && (previous.sps.frameWidth != current.sps.frameWidth)) ||
            (previous.sps.frameHeight != 0 && (previous.sps.frameHeight != current.sps.frameHeight)))
        {
            LOG_E("Frame group size (width & height) are not consistent!");

            assert(false);
        }
    }

    // Start playback loop
    VideoDecoderContext& videoDecoderContext = player->_videoDecoderContext;
    videoDecoderContext.geometryVideoDecoder.numTotalFramesDecoded = 0;
    videoDecoderContext.textureVideoDecoder.numTotalFramesDecoded = 0;
    videoDecoderContext.occupancyVideoDecoder.numTotalFramesDecoded = 0;

    int64_t totalFrameDecodingStartTime = HighResolutionTimer::getTimeMs();

    bool inputEOS = false;
    bool outputEOS = false;

    while (true)
    {
        if (player->_state == State::PLAYING)
        {
#if DEBUG_DECODE_FIRST_FRAME_ONLY

            static bool once = false;

            if (!once)
            {
                queueInputBuffers(playbackContext, videoDecoderContext);

                once = true;
            }

            queueOutputBuffers(videoDecoderContext);

#else

            // Queue input buffers
            if (!playbackContext.isInputEOS())
            {
                queueInputBuffers(playbackContext, videoDecoderContext);
            }
            else
            {
                inputEOS = true;
            }

            // Wait until input EOS and output EOS is reached
            if (inputEOS && playbackContext.isOutputEOS())
            {
                outputEOS = true;
            }

            // Query output buffers
            if (!outputEOS)
            {
                queueOutputBuffers(videoDecoderContext);
            }

            // Force auto-loop with hacks
            if (outputEOS)
            {
                // Calculate average decoding stats
                int64_t totalFrameDecodingEndTime = HighResolutionTimer::getTimeMs();
                int64_t totalFrameDecodingTime = totalFrameDecodingEndTime - totalFrameDecodingStartTime;

                int32_t averageFrameDurationGeometry = (int32_t)(totalFrameDecodingTime / videoDecoderContext.geometryVideoDecoder.numTotalFramesDecoded);
                int32_t averageFrameDurationTexture = (int32_t)(totalFrameDecodingTime / videoDecoderContext.textureVideoDecoder.numTotalFramesDecoded);
                int32_t averageFrameDurationOccupancy = (int32_t)(totalFrameDecodingTime / videoDecoderContext.occupancyVideoDecoder.numTotalFramesDecoded);

                videoDecoderContext.stats.geometry.numTotalFrames = (uint32_t)videoDecoderContext.geometryVideoDecoder.numTotalFramesDecoded;
                videoDecoderContext.stats.geometry.averageFPS = 1000.0f / (float)averageFrameDurationGeometry;
                videoDecoderContext.stats.geometry.averageFrameDurationMs = averageFrameDurationGeometry;

                videoDecoderContext.stats.texture.numTotalFrames = (uint32_t)videoDecoderContext.textureVideoDecoder.numTotalFramesDecoded;
                videoDecoderContext.stats.texture.averageFPS = 1000.0f / (float)averageFrameDurationTexture;
                videoDecoderContext.stats.texture.averageFrameDurationMs = averageFrameDurationTexture;

                videoDecoderContext.stats.occupancy.numTotalFrames = (uint32_t)videoDecoderContext.occupancyVideoDecoder.numTotalFramesDecoded;
                videoDecoderContext.stats.occupancy.averageFPS = 1000.0f / (float)averageFrameDurationOccupancy;
                videoDecoderContext.stats.occupancy.averageFrameDurationMs = averageFrameDurationOccupancy;

                // Print average stats for whole clip
                LOG_I("---------- DECODING STATS - BEGINS ----------");

                LOG_I("Total decoding time: %d", totalFrameDecodingTime);

                LOG_I("Total num frames decoded: %d (Decoder: geometry)", videoDecoderContext.stats.geometry.numTotalFrames);
                LOG_I("Average %f fps (Decoder: geometry)", videoDecoderContext.stats.geometry.averageFPS);
                LOG_I("Average %d ms / frame (Decoder: geometry)", videoDecoderContext.stats.geometry.averageFrameDurationMs);

                LOG_I("Total num frames decoded: %d (Decoder: texture)", videoDecoderContext.stats.texture.numTotalFrames);
                LOG_I("Average %f fps (Decoder: texture)", videoDecoderContext.stats.texture.averageFPS);
                LOG_I("Average %d ms / frame (Decoder: texture)", videoDecoderContext.stats.texture.averageFrameDurationMs);

                LOG_I("Total num frames decoded: %d (Decoder: occupancy)", videoDecoderContext.stats.occupancy.numTotalFrames);
                LOG_I("Average %f fps (Decoder: occupancy)", videoDecoderContext.stats.occupancy.averageFPS);
                LOG_I("Average %d ms / frame (Decoder: occupancy)", videoDecoderContext.stats.occupancy.averageFrameDurationMs);

                LOG_I("---------- DECODING STATS - ENDS ----------");

                // Flush video decoders
                videoDecoderContext.geometryVideoDecoder.flush();
                videoDecoderContext.textureVideoDecoder.flush();
                videoDecoderContext.occupancyVideoDecoder.flush();

                // Set parser context states
                inputEOS = false;
                outputEOS = false;

                // Reset playback position in frame group
                playbackContext.resetPlaybackPosition();

                totalFrameDecodingStartTime = HighResolutionTimer::getTimeMs();

                videoDecoderContext.geometryVideoDecoder.numTotalFramesDecoded = 0;
                videoDecoderContext.textureVideoDecoder.numTotalFramesDecoded = 0;
                videoDecoderContext.occupancyVideoDecoder.numTotalFramesDecoded = 0;
            }

#endif
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

ARPlayer::ARPlayer()
: _thread(NULL)
{
}

ARPlayer::~ARPlayer()
{
}

ARPlayer::Result::Enum ARPlayer::initialize(Config config)
{
    _config = config;

    return ARPlayer::Result::RESULT_OK;
}

ARPlayer::Result::Enum ARPlayer::shutdown()
{
    _state = State::SHUTDOWN;

    // Wait for shutdown
    _thread->join();

    delete _thread;
    _thread = NULL;

    // Destroy decoders
    _videoDecoderContext.geometryVideoDecoder.shutdown();
    _videoDecoderContext.textureVideoDecoder.shutdown();
    _videoDecoderContext.occupancyVideoDecoder.shutdown();

    // Release retained frame
    releasePresentationFrame(_videoDecoderContext, _presentationFrame);

    return ARPlayer::Result::RESULT_OK;
}

ARPlayer::Result::Enum ARPlayer::open(std::string filename)
{
    // Read file
    IOBuffer ioBuffer = FileSystem::loadFromDisk(filename);
    assert(ioBuffer.data != NULL);

    if (ioBuffer.size == 0)
    {
        LOG_E("File (%s) not found", filename.c_str());

        return ARPlayer::Result::RESULT_ERROR;
    }

    LOG_I("File (%s) loaded", filename.c_str());

#if DEBUG_PCC_DECODER

    pcc::PCCDecoderParameters params;
    debugDecode(params, ioBuffer);

#endif

    _filename = filename;

    // Parse first frame group to get decoder config parameters
    PCC::Bitstream bitstream(ioBuffer.data, ioBuffer.size);

    PCC::TMC2Header header;

    if (!PCC::parseContainerHeader(bitstream, header))
    {
        return ARPlayer::Result::RESULT_ERROR;
    }

    PCC::FrameGroup firstFrameGroup;

    if (!PCC::parse(bitstream, firstFrameGroup))
    {
        return ARPlayer::Result::RESULT_ERROR;
    }

    IOBuffer::free(&ioBuffer);

    ////////////////////////////////////////////////////////////////////////////
    ///
    /// Note: E.g. Android AMediaCodec needs to be inialized from UI/rendering thread
    ///
    ////////////////////////////////////////////////////////////////////////////

    {
        HEVC::Bitstream videoBitstream(firstFrameGroup.geometry.data(), firstFrameGroup.geometry.size());
        DecoderParameters decoderParameters;
        HEVC::parseDecoderParameters(videoBitstream, decoderParameters);

        HEVC::Bitstream vpsBitstream(decoderParameters.vps.data(), decoderParameters.vps.size());
        HEVC::VPS vps;
        HEVC::parseVPS(vpsBitstream, vps);

        HEVC::Bitstream spsBitstream(decoderParameters.sps.data(), decoderParameters.sps.size());
        HEVC::SPS sps;
        HEVC::parseSPS(spsBitstream, sps);

        DecoderConfig decoderConfig;
        decoderConfig.parameters = decoderParameters;
        decoderConfig.width = sps.picWidthInLumaSamples;
        decoderConfig.height = sps.picHeightInLumaSamples;
        decoderConfig.name = "Geometry";
		decoderConfig.inputBufferQueueSize = inputBufferQueueSize;
        decoderConfig.outputBufferQueueSize = outputBufferQueueSize;
        decoderConfig.manualVideoTextureUpload = _config.manualVideoTextureUpload;

        LOG_I("---------- DECODER CONFIG - BEGINS ----------");

        LOG_I("Decoder: %s", decoderConfig.name.c_str());
        LOG_I("Frame width: %d", decoderConfig.width);
        LOG_I("Frame height: %d", decoderConfig.height);

        LOG_I("Profile: %s", HEVC::profileName(vps.profileTierLevel.generalProfileIdc));
        LOG_I("Tier: %s", HEVC::tierName(vps.profileTierLevel.generalTierFlag));
        LOG_I("Level: %d", vps.profileTierLevel.generalLevelIdc / 30);

        LOG_I("---------- DECODER CONFIG - ENDS ----------");

		bool result = _videoDecoderContext.geometryVideoDecoder.initialize(decoderConfig);
		assert(result);

        _videoDecoderContext.geometryVideoDecoder.start();
    }

    {
        HEVC::Bitstream videoBitstream(firstFrameGroup.texture.data(), firstFrameGroup.texture.size());
        DecoderParameters decoderParameters;
        HEVC::parseDecoderParameters(videoBitstream, decoderParameters);

        HEVC::Bitstream vpsBitstream(decoderParameters.vps.data(), decoderParameters.vps.size());
        HEVC::VPS vps;
        HEVC::parseVPS(vpsBitstream, vps);

        HEVC::Bitstream spsBitstream(decoderParameters.sps.data(), decoderParameters.sps.size());
        HEVC::SPS sps;
        HEVC::parseSPS(spsBitstream, sps);

        DecoderConfig decoderConfig;
        decoderConfig.parameters = decoderParameters;
        decoderConfig.width = sps.picWidthInLumaSamples;
        decoderConfig.height = sps.picHeightInLumaSamples;
        decoderConfig.name = "Texture";
        decoderConfig.inputBufferQueueSize = inputBufferQueueSize;
        decoderConfig.outputBufferQueueSize = outputBufferQueueSize;
        decoderConfig.manualVideoTextureUpload = _config.manualVideoTextureUpload;

        LOG_I("---------- DECODER CONFIG - BEGINS ----------");

        LOG_I("Decoder: %s", decoderConfig.name.c_str());
        LOG_I("Frame width: %d", decoderConfig.width);
        LOG_I("Frame height: %d", decoderConfig.height);

        LOG_I("Profile: %s", HEVC::profileName(vps.profileTierLevel.generalProfileIdc));
        LOG_I("Tier: %s", HEVC::tierName(vps.profileTierLevel.generalTierFlag));
        LOG_I("Level: %d", vps.profileTierLevel.generalLevelIdc / 30);

        LOG_I("---------- DECODER CONFIG - ENDS ----------");

		bool result = _videoDecoderContext.textureVideoDecoder.initialize(decoderConfig);
		assert(result);

        _videoDecoderContext.textureVideoDecoder.start();
    }

    {
        HEVC::Bitstream videoBitstream(firstFrameGroup.occupancy.data(), firstFrameGroup.occupancy.size());
        DecoderParameters decoderParameters;
        HEVC::parseDecoderParameters(videoBitstream, decoderParameters);

        HEVC::Bitstream vpsBitstream(decoderParameters.vps.data(), decoderParameters.vps.size());
        HEVC::VPS vps;
        HEVC::parseVPS(vpsBitstream, vps);

        HEVC::Bitstream spsBitstream(decoderParameters.sps.data(), decoderParameters.sps.size());
        HEVC::SPS sps;
        HEVC::parseSPS(spsBitstream, sps);

        DecoderConfig decoderConfig;
        decoderConfig.parameters = decoderParameters;
        decoderConfig.width = sps.picWidthInLumaSamples;
        decoderConfig.height = sps.picHeightInLumaSamples;
        decoderConfig.name = "Occupancy";
        decoderConfig.inputBufferQueueSize = inputBufferQueueSize;
        decoderConfig.outputBufferQueueSize = outputBufferQueueSize;
        decoderConfig.manualVideoTextureUpload = _config.manualVideoTextureUpload;

        LOG_I("---------- DECODER CONFIG - BEGINS ----------");

        LOG_I("Decoder: %s", decoderConfig.name.c_str());
        LOG_I("Frame width: %d", decoderConfig.width);
        LOG_I("Frame height: %d", decoderConfig.height);

        LOG_I("Profile: %s", HEVC::profileName(vps.profileTierLevel.generalProfileIdc));
        LOG_I("Tier: %s", HEVC::tierName(vps.profileTierLevel.generalTierFlag));
        LOG_I("Level: %d", vps.profileTierLevel.generalLevelIdc / 30);

        LOG_I("---------- DECODER CONFIG - ENDS ----------");

		bool result = _videoDecoderContext.occupancyVideoDecoder.initialize(decoderConfig);
		assert(result);

        _videoDecoderContext.occupancyVideoDecoder.start();
    }

    // Start thread
    _thread = new std::thread(ARPlayer::threadEntry, this);

    return ARPlayer::Result::RESULT_OK;
}

ARPlayer::Result::Enum ARPlayer::play()
{
    _state = State::PLAYING;

    return ARPlayer::Result::RESULT_OK;
}

ARPlayer::Result::Enum ARPlayer::stop()
{
    _videoDecoderContext.geometryVideoDecoder.stop();
    _videoDecoderContext.textureVideoDecoder.stop();
    _videoDecoderContext.occupancyVideoDecoder.stop();

    _state = State::STOPPED;

    return ARPlayer::Result::RESULT_OK;
}

ARPlayer::Result::Enum ARPlayer::pause()
{
    _state = State::PAUSED;

    return ARPlayer::Result::RESULT_OK;
}

ARPlayer::Result::Enum ARPlayer::resume()
{
    _state = State::PLAYING;

    return ARPlayer::Result::RESULT_OK;
}

bool ARPlayer::isPresentationFrameCompleted(PCCRenderer::PresentationFrame& presentationFrame)
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

bool ARPlayer::isValidPresentationFrame(PCCRenderer::PresentationFrame& presentationFrame)
{
    if (presentationFrame.depth0 != NULL && presentationFrame.color0 != NULL && presentationFrame.occupancy != NULL)
    {
        return true;
    }

    return false;
}

bool ARPlayer::isNextPresentationFrameAvailable(VideoDecoderContext& videoDecoderContext)
{
    bool result = true;

    result &= videoDecoderContext.geometryVideoDecoder.getOutputQueueSize() >= MIN_OUTPUT_QUEUE_SIZE;
    result &= videoDecoderContext.textureVideoDecoder.getOutputQueueSize() >= MIN_OUTPUT_QUEUE_SIZE;
    result &= videoDecoderContext.occupancyVideoDecoder.getOutputQueueSize() >= MIN_OUTPUT_QUEUE_SIZE;

    return result;
}

ARPlayer::Result::Enum ARPlayer::fetchPresentationFrame(PCCRenderer::PresentationFrame& presentationFrame)
{
    if (_state == State::PAUSED)
    {
        if (isValidPresentationFrame(_presentationFrame))
        {
            presentationFrame = _presentationFrame;

            return Result::RESULT_OK;
        }
        else
        {
            return Result::RESULT_NO_FRAME_AVAILABLE;
        }
    }

    // Check if current presentation frame is valid and frame is not completed (presentation timestamp + duration)
    if (isValidPresentationFrame(_presentationFrame) && !isPresentationFrameCompleted(_presentationFrame))
    {
        presentationFrame = _presentationFrame;

        return Result::RESULT_OK;
    }

    // Check if new presentation frame is available
    bool nextFrameAvailable = isNextPresentationFrameAvailable(_videoDecoderContext);

    if (!nextFrameAvailable)
    {
        if (isValidPresentationFrame(_presentationFrame))
        {
            presentationFrame = _presentationFrame;

            return Result::RESULT_OK;
        }
        else
        {
            return Result::RESULT_NO_FRAME_AVAILABLE;
        }
    }

    // Release old presentation frame and create a new one
    releasePresentationFrame(_videoDecoderContext, _presentationFrame);

    CachedFrame* geometry0Frame = _videoDecoderContext.geometryVideoDecoder.retainCachedFrame();
    CachedFrame* texture0Frame = _videoDecoderContext.textureVideoDecoder.retainCachedFrame();
    CachedFrame* occupancyFrame = _videoDecoderContext.occupancyVideoDecoder.retainCachedFrame();

    // TODO: Current expecting that frame drops does not exist
    bool isSync = true;

    isSync &= (geometry0Frame->pts == texture0Frame->pts);
    isSync &= (geometry0Frame->pts == occupancyFrame->pts);

    if (isSync)
    {
        _presentationFrame.depth0 = geometry0Frame;
        _presentationFrame.color0 = texture0Frame;
        _presentationFrame.occupancy = occupancyFrame;

        _playbackContext.mutex.lock();
        {
            PCC::Frame* frame = _playbackContext.getCurrentRenderFrame(); // This should always succeed at this point
            assert(frame != NULL);

            _presentationFrame.patches = frame->patches;
            _presentationFrame.blockToPatch = frame->blockToPatch;

            presentationFrame = _presentationFrame;

            _playbackContext.proceedToNextRenderFrame();
        }
        _playbackContext.mutex.unlock();

        return Result::RESULT_OK;
    }
    else
    {
        return Result::RESULT_OUT_OF_SYNC_FRAME_AVAILABLE;
    }
}

void ARPlayer::releasePresentationFrame(VideoDecoderContext& videoDecoderContext, PCCRenderer::PresentationFrame& presentationFrame)
{
    videoDecoderContext.geometryVideoDecoder.releaseCachedFrame(presentationFrame.depth0);
    videoDecoderContext.textureVideoDecoder.releaseCachedFrame(presentationFrame.color0);
    videoDecoderContext.occupancyVideoDecoder.releaseCachedFrame(presentationFrame.occupancy);

    presentationFrame.depth0 = NULL;
    presentationFrame.depth1 = NULL;
    presentationFrame.color0 = NULL;
    presentationFrame.color1 = NULL;
    presentationFrame.occupancy = NULL;

    presentationFrame.patches.clear();
    presentationFrame.blockToPatch.clear();
}

void ARPlayer::queueInputBuffers(PlaybackContext& playbackContext, VideoDecoderContext& videoDecoderContext)
{
    bool queueInput = shouldQueueInput(videoDecoderContext);

    if (queueInput)
    {
        PCC::FrameGroup* frameGroup = playbackContext.getCurrentDecoderFrameGroup();
        PCC::Frame* frame = playbackContext.getCurrentDecoderFrame();

        if (frameGroup && frame)
        {
            // Decode new frame
            int64_t presentationTimeUs = frame->presentationTimeUs;
            bool inputEOS = playbackContext.isInputEOS();

            bool succeed = true;

            {
                uint8_t* buffer = frameGroup->geometry.data() + frame->geometry.offset;
                size_t bytes = frame->geometry.length;

                succeed &= queueInputBuffer(buffer, bytes, videoDecoderContext.geometryVideoDecoder, presentationTimeUs, inputEOS);
            }

            {
                uint8_t* buffer = frameGroup->texture.data() + frame->texture.offset;
                size_t bytes = frame->texture.length;

                succeed &= queueInputBuffer(buffer, bytes, videoDecoderContext.textureVideoDecoder, presentationTimeUs, inputEOS);
            }

            {
                uint8_t* buffer = frameGroup->occupancy.data() + frame->occupancy.offset;
                size_t bytes = frame->occupancy.length;

                succeed &= queueInputBuffer(buffer, bytes, videoDecoderContext.occupancyVideoDecoder, presentationTimeUs, inputEOS);
            }

            if (succeed)
            {
                playbackContext.proceedToNextDecoderFrame();
            }
        }
    }
}

bool ARPlayer::queueInputBuffer(uint8_t* buffer, size_t bytes, HWVideoDecoder& videoDecoder, int64_t presentationTimeUs, bool inputEOS)
{
#if DEBUG_DUMP_RAW_COMPRESSED_YUV_FRAME

    std::string outputPath;
    outputPath.append(std::to_string(presentationTimeUs));
    outputPath.append("_");
    outputPath.append(videoDecoder.getConfig().name);
    outputPath.append("_frame.265");

    FileSystem::saveToDisk(outputPath.c_str(), buffer, bytes);

#endif

    return videoDecoder.queueVideoInputBuffer(buffer, bytes, presentationTimeUs, inputEOS);
}

void ARPlayer::queueOutputBuffers(VideoDecoderContext& videoDecoderContext)
{
    if (!videoDecoderContext.geometryVideoDecoder.isOutputQueueFull())
    {
        videoDecoderContext.geometryVideoDecoder.dequeueOutputBuffer();
    }

    if (!videoDecoderContext.textureVideoDecoder.isOutputQueueFull())
    {
        videoDecoderContext.textureVideoDecoder.dequeueOutputBuffer();
    }

    if (!videoDecoderContext.occupancyVideoDecoder.isOutputQueueFull())
    {
        videoDecoderContext.occupancyVideoDecoder.dequeueOutputBuffer();
    }
}

bool ARPlayer::shouldQueueInput(VideoDecoderContext& videoDecoderContext)
{
    bool depthQueueInput = !videoDecoderContext.geometryVideoDecoder.isInputQueueFull() && !videoDecoderContext.geometryVideoDecoder.isOutputQueueFull();
    bool colorQueueInput = !videoDecoderContext.textureVideoDecoder.isInputQueueFull() && !videoDecoderContext.textureVideoDecoder.isOutputQueueFull();
    bool occupancyQueueInput = !videoDecoderContext.occupancyVideoDecoder.isInputQueueFull() && !videoDecoderContext.occupancyVideoDecoder.isOutputQueueFull();

    bool result = (depthQueueInput & colorQueueInput & occupancyQueueInput);

    return result;
}

ARPlayer::StatsCollection ARPlayer::getStatsCollection() const
{
    return _videoDecoderContext.stats;
}

///

PCC::FrameGroup* ARPlayer::PlaybackContext::getCurrentRenderFrameGroup()
{
    if (outputFrameGroupIndex < frameGroups.size())
    {
        return &frameGroups[outputFrameGroupIndex];
    }

    return NULL;
}

PCC::Frame* ARPlayer::PlaybackContext::getCurrentRenderFrame()
{
    if (outputFrameGroupIndex < frameGroups.size())
    {
        PCC::FrameGroup& frameGroup = frameGroups[outputFrameGroupIndex];
        PCC::FrameStream& frames = frameGroup.frames;

        if (outputFrameIndex < frames.size())
        {
            return &frames[outputFrameIndex];
        }
    }

    return NULL;
}

void ARPlayer::PlaybackContext::proceedToNextRenderFrame()
{
#if DEBUG_DECODE_FIRST_FRAME_ONLY

    return;

#else

    if (outputFrameGroupIndex < frameGroups.size())
    {
        PCC::FrameGroup& frameGroup = frameGroups[outputFrameGroupIndex];
        PCC::FrameStream& frames = frameGroup.frames;

        outputFrameIndex++;

        if (outputFrameIndex >= frames.size())
        {
            outputFrameIndex = 0;
            outputFrameGroupIndex++;
        }
    }

#endif
}

bool ARPlayer::PlaybackContext::isOutputEOS()
{
    if (frameGroups.size() == 0)
    {
        return true;
    }

    if (outputFrameGroupIndex >= frameGroups.size())
    {
        return true;
    }

    return false;
}

///

PCC::FrameGroup* ARPlayer::PlaybackContext::getCurrentDecoderFrameGroup()
{
    if (inputFrameGroupIndex < frameGroups.size())
    {
        return &frameGroups[inputFrameGroupIndex];
    }

    return NULL;
}

PCC::Frame* ARPlayer::PlaybackContext::getCurrentDecoderFrame()
{
    if (inputFrameGroupIndex < frameGroups.size())
    {
        PCC::FrameGroup& frameGroup = frameGroups[inputFrameGroupIndex];
        PCC::FrameStream& frames = frameGroup.frames;

        if (inputFrameIndex < frames.size())
        {
            return &frames[inputFrameIndex];
        }
    }

    return NULL;
}

void ARPlayer::PlaybackContext::proceedToNextDecoderFrame()
{
    if (inputFrameGroupIndex < frameGroups.size())
    {
        PCC::FrameGroup& frameGroup = frameGroups[inputFrameGroupIndex];
        PCC::FrameStream& frames = frameGroup.frames;

        inputFrameIndex++;

        if (inputFrameIndex >= frames.size())
        {
            inputFrameIndex = 0;
            inputFrameGroupIndex++;
        }
    }
}

bool ARPlayer::PlaybackContext::isInputEOS()
{
    if (frameGroups.size() == 0)
    {
        return true;
    }

    if (inputFrameGroupIndex >= frameGroups.size())
    {
        return true;
    }

    return false;
}

///

void ARPlayer::PlaybackContext::resetPlaybackPosition()
{
    mutex.lock();

    outputFrameGroupIndex = 0;
    outputFrameIndex = 0;

    inputFrameGroupIndex = 0;
    inputFrameIndex = 0;

    mutex.unlock();
}

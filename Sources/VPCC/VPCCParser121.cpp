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

#include "VPCC/VPCCParser121.h"
#include "VPCC/VPCCDecoder121.h"
#include "VPCC/VPCCDatatypes121.h"

#include <cmath>
#include <assert.h>

#include "Helpers.h"
#include "HEVC.h"

#include "Logger.h"
#include "FileSystem.h"

#include "PCCCommon.h"
#include "PCCChrono.h"
#include "PCCContext.h"
#include "PCCFrameContext.h"
#include "PCCBitstream.h"
#include "PCCGroupOfFrames.h"
#include "PCCBitstreamReader.h"

namespace VPCC121
{
    void dumpVideoStream(uint8_t* buffer, size_t bufferSize, VideoType::Enum type)
    {
#if 0 // DEBUG

        // Dump encoded YUV sequence
        std::string outputPath;

        static size_t counter = 0;

        outputPath.append(std::to_string(counter));
        outputPath.append("_");
        outputPath.append(VideoType::toString(type));
        outputPath.append(".265");

        counter++;

        FileSystem::saveToDisk(outputPath.c_str(), buffer, bufferSize);

#endif
    }

    void createVideoPackets(uint8_t* buffer, size_t bufferSize, std::vector<VideoFramePacket>& packets, VideoType::Enum type)
    {
        LOG_V("---------- VIDEO STREAM - BEGIN ----------");

        LOG_V("Video stream type: %s", VideoType::toString(type));
        LOG_V("Video stream size: %llu bytes", bufferSize);

        std::vector<HEVC::NALUnit> nalUnits;
        HEVC::readNALUnits(buffer, bufferSize, nalUnits);

        std::vector<HEVC::Slice> slices;
        HEVC::parseSlices(buffer, bufferSize, nalUnits, slices);

        // Find POCs
        size_t sliceIndex = 0;

        for (size_t startIndex = 0; startIndex < nalUnits.size();)
        {
            const HEVC::NALUnit& startNalUnit = nalUnits.at(startIndex);

            size_t endIndex = HEVC::findFrameEnd(startIndex, nalUnits);
            const HEVC::NALUnit& endNalUnit = nalUnits.at(endIndex);

            const HEVC::Slice& slice = slices.at(sliceIndex);

            VideoFramePacket packet;
            packet.offset = startNalUnit.offset;
            packet.length = (endNalUnit.offset - startNalUnit.offset) + endNalUnit.length;
            packet.sliceIndex = sliceIndex;
            packet.pictureOrderCount = slice.slicePicOrderCntLsb;

            packets.push_back(packet);

#if 0 // DEBUG

            LOG_V("Slice (#%d) POC: %d type: %s", sliceIndex, slice.slicePicOrderCntLsb, HEVC::SliceType::toString((HEVC::SliceType::Enum)slice.sliceType));

#endif

            startIndex = endIndex + 1;
            sliceIndex++;
        }

        LOG_V("Slice count: %d", sliceIndex);

#if 0 // DEBUG

        LOG_V("---------- NAL UNITS - BEGIN ----------");

        LOG_V("Video stream type: %s", VideoType::toString(type));

        for (uint32_t i = 0; i< nalUnits.size(); i++)
        {
            HEVC::NALUnit nalUnit = nalUnits[i];

            const char* string = HEVC::NALUnitType::toString(nalUnit.type);
            LOG_D("0x%zx (%zu) = %s, length = %lu, header length = %lu", nalUnit.offset, nalUnit.offset, string, nalUnit.length, nalUnit.headerLength);
        }

        LOG_V("---------- NAL UNITS - END ----------");

        dumpVideoStream(buffer, bufferSize, type);

#endif

        LOG_V("---------- VIDEO STREAM - END ----------");
    }

    VideoType::Enum convertVideoType(pcc::PCCVideoType type)
    {
        if (type == pcc::VIDEO_OCCUPANCY) return VideoType::OCCUPANCY;
        else if (type == pcc::VIDEO_GEOMETRY) return VideoType::GEOMETRY;
        else if (type == pcc::VIDEO_TEXTURE) return VideoType::TEXTURE;
        
        return VideoType::INVALID;
    }

    void parseVideoStream(pcc::PCCVideoBitstream& videoBitstream, FrameGroup& frameGroup)
    {
        uint8_t* buffer = videoBitstream.buffer();
        size_t bufferSize = videoBitstream.size();
        
        VideoType::Enum videoType = convertVideoType(videoBitstream.type());
        
        VideoStream& videoStream = frameGroup.videoStream[videoType];
        videoStream.type = videoType;
        videoStream.buffer.insert(videoStream.buffer.begin(), &buffer[0], &buffer[bufferSize]);
        
        std::vector<VideoFramePacket>* packets = &frameGroup.videoStream[videoType].packets;
        
        // Parse HEVC video stream
        createVideoPackets(buffer, bufferSize, *packets, videoType);

        // Parse decoder parameters
        HEVC::Bitstream hevcBitstream(buffer, bufferSize);
        HEVC::parseDecoderParameters(hevcBitstream, videoStream.decoderParameters);

        // Parse VPS
        HEVC::Bitstream vpsBitstream(videoStream.decoderParameters.vps.data(), videoStream.decoderParameters.vps.size());
        HEVC::parseVPS(vpsBitstream, videoStream.vps);

        // Parse SPS
        HEVC::Bitstream spsBitstream(videoStream.decoderParameters.sps.data(), videoStream.decoderParameters.sps.size());
        HEVC::parseSPS(spsBitstream, videoStream.sps);
    }

    void preprocess(pcc::PCCContext& context, FrameGroup& frameGroup)
    {
        //
        // Populate rendering data structures
        //
        std::vector<pcc::PCCAtlasFrameContext>& atlasFrameContexts = context.getFrames();
        size_t numFrames = atlasFrameContexts.size();
        
        frameGroup.frames.resize(numFrames);
        
        for (size_t frameIndex = 0; frameIndex < numFrames; frameIndex++)
        {
            pcc::PCCAtlasFrameContext& atlasFrameContext = atlasFrameContexts.at(frameIndex);
            pcc::PCCFrameContext& frameContext = atlasFrameContext.getTitleFrameContext();
            
            FrameData& frame = frameGroup.frames[frameIndex];
            frame.afOrderCnt = frameIndex;
            frame.index = frameIndex;
            frame.width = atlasFrameContext.getAtlasFrameWidth();
            frame.height = atlasFrameContext.getAtlasFrameHeight();
            
            std::vector<size_t>& blockToPatches = frameContext.getBlockToPatch();
            
            frame.blockToPatch.resize(blockToPatches.size());
            
            for (size_t blockIndex = 0; blockIndex < blockToPatches.size(); ++blockIndex)
            {
                frame.blockToPatch[blockIndex] = blockToPatches.at(blockIndex);
            }
            
            std::vector<pcc::PCCPatch>& patches = frameContext.getPatches();
            
            frame.patches.resize(patches.size());
            
            for (size_t patchesIndex = 0; patchesIndex < patches.size(); ++patchesIndex)
            {
                pcc::PCCPatch& p = patches.at(patchesIndex);
                
                Patch& patch = frame.patches.at(patchesIndex);
                patch.u1 = (uint32_t)p.getU1();
                patch.v1 = (uint32_t)p.getV1();
                patch.d1 = (uint32_t)p.getD1();
                patch.u0 = (uint32_t)p.getU0();
                patch.v0 = (uint32_t)p.getV0();
                patch.sizeU0 = (uint32_t)p.getSizeU0();
                patch.sizeV0 = (uint32_t)p.getSizeV0();
                patch.occupancyResolution = (uint32_t)p.getOccupancyResolution();
                patch.normalAxis = (uint32_t)p.getNormalAxis();
                patch.tangentAxis = (uint32_t)p.getTangentAxis();
                patch.bitangentAxis = (uint32_t)p.getBitangentAxis();
                patch.patchOrientation = (uint32_t)p.getPatchOrientation();
                patch.projectionMode = (uint32_t)p.getProjectionMode();
            }
        }
        
        //
        // Parse occupancy video stream
        //
        {
            pcc::PCCVideoBitstream& videoBitstream = context.getVideoBitstream(pcc::VIDEO_OCCUPANCY);
            parseVideoStream(videoBitstream, frameGroup);
        }
        
        //
        // Parse geometry stream
        //
        {
            pcc::PCCVideoBitstream& videoBitstream = context.getVideoBitstream(pcc::VIDEO_GEOMETRY);
            parseVideoStream(videoBitstream, frameGroup);
        }
        
        //
        // Parse texture stream
        //
        {
            pcc::PCCVideoBitstream& videoBitstream = context.getVideoBitstream(pcc::VIDEO_TEXTURE);
            parseVideoStream(videoBitstream, frameGroup);
        }
    }

    bool parseFrameGroups(IOBuffer& buffer, std::vector<FrameGroup>& frameGroups, bool firstOnly)
    {
        pcc::PCCBitstream bitstream;
        
        std::vector<uint8_t>& data = bitstream.vector();
        data.clear();
        data.insert(data.begin(), &buffer.data[0], &buffer.data[buffer.size]);
        
        pcc::PCCBitstreamStat bitstreamStat;
        bitstreamStat.setHeader(bitstream.size());
        
        pcc::SampleStreamV3CUnit ssvu;
        pcc::PCCBitstreamReader bitstreamReader;
        size_t headerSize = pcc::PCCBitstreamReader::read(bitstream, ssvu);
        
        bitstreamStat.incrHeader(headerSize);
        
        //
        // Calculate number of frame groups based on VPS count.
        //
        size_t frameGroupCount = 0;
        
        std::vector<pcc::V3CUnit>& units = ssvu.getV3CUnit();
        
        for (size_t unitIndex = 0; unitIndex < units.size(); ++unitIndex)
        {
            pcc::V3CUnit& unit = units.at(unitIndex);
            
            if (unit.getType() == pcc::V3C_VPS)
            {
                frameGroupCount++;
            }
        }
        
        frameGroups.resize(frameGroupCount);
        
        bool bytesAvailable = true;
        int32_t frameGroupIndex = 0;
        
        while (bytesAvailable)
        {
            pcc::PCCContext context;
            context.setBitstreamStat(bitstreamStat);
            
            pcc::PCCBitstreamReader bitstreamReader;
            
            if (bitstreamReader.decode(ssvu, context) == 0)
            {
                return false;
            }
            
            // Allocate atlas structures
            context.resizeAtlas(context.getVps().getAtlasCountMinus1() + 1);
            
            for (uint32_t atlasIndex = 0; atlasIndex < context.getVps().getAtlasCountMinus1() + 1; atlasIndex++)
            {
                context.getAtlas(atlasIndex).allocateVideoFrames(context, 0);
                context.setAtlasIndex(atlasIndex);
                
                int32_t result = decode(&context, atlasIndex);
                
                if (result != 0)
                {
                    return false;
                }
                
                // Pre-process data for rendering
                FrameGroup& frameGroup = frameGroups.at(frameGroupIndex);
                preprocess(context, frameGroup);

                bytesAvailable = (ssvu.getV3CUnitCount() > 0);
                
                frameGroupIndex++;
                
                if (firstOnly)
                {
                    bytesAvailable = false;
                    
                    break;
                }
            }
        }
        
        bitstreamStat.trace();
        
        return true;
    }
}

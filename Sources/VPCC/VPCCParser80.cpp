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

#include "VPCC/VPCCParser80.h"

#include <cmath>

#include "Logger.h"
#include "FileSystem.h"

#undef min
#undef max

namespace VPCC
{
    // 7.3.2.2 V-PCC unit header syntax
    bool vpccUnitHeader(Bitstream& bitstream, ParserContext& context, VPCCUnitType::Enum& vpccUnitType);

    // 7.3.2.3 V-PCC unit payload syntax
    bool vpccUnitPayload(Bitstream& bitstream, ParserContext& context, FrameGroup& frameGroup, VPCCUnitType::Enum& vpccUnitType);

    // 7.3.4.1 General V-PCC parameter set syntax
    void vpccParameterSet(Bitstream& bitstream, ParserContext& context);

    void atlasSubStream(Bitstream& bitstream, ParserContext& context);

    // 7.3.4.1 General V-PCC parameter set syntax
    void vpccParameterSet(Bitstream& bitstream, ParserContext& context);

    // 7.3.4.3 Occupancy parameter set syntax
    void occupancyInformation(OccupancyInformation& oi, Bitstream& bitstream);

    // 7.3.4.4 Geometry parameter set syntax
    void geometryInformation(GeometryInformation& gi, VpccParameterSet& vps, Bitstream& bitstream);

    // 7.3.4.5 Attribute information
    void attributeInformation(AttributeInformation& ai, VpccParameterSet& vps, Bitstream& bitstream);

    // C.2 Sample stream NAL unit syntax and semantics
    // C.2.1 Sample stream NAL header syntax
    void sampleStreamNalHeader(Bitstream& bitstream, SampleStreamNalUnit& ssnu);

    // C.2.2 Sample stream NAL unit syntax
    void sampleStreamNalUnit(Bitstream& bitstream, ParserContext& context, SampleStreamNalUnit& ssnu, size_t index);

    // 7.3.6.1 Atlas sequence parameter set RBSP
    void atlasSequenceParameterSetRBSP(Bitstream& bitstream, ParserContext& context, AtlasSequenceParameterSetRBSP& asps);

    // 7.3.6.3  Atlas frame parameter set RBSP syntax
    void atlasFrameParameterSetRbsp(Bitstream& bitstream, ParserContext& context, AtlasFrameParameterSetRbsp& afps);

    // 7.3.5.2 NAL unit header syntax
    void nalUnitHeader(Bitstream& bitstream, NalUnit& nalUnit);

    // 7.3.6.10  Atlas tile group layer RBSP syntax
    void atlasTileGroupLayerRbsp(Bitstream& bitstream, ParserContext& context, AtlasTileGroupLayerRbsp& atgl);

    // 7.3.6.5  Supplemental enhancement information RBSP syntax
    void seiRbsp(Bitstream& bitstream, ParserContext& context, NalUnitType::Enum nalUnitType);

    // 7.3.6.12  Reference list structure syntax
    void refListStruct(Bitstream& bitstream, RefListStruct& rls, AtlasSequenceParameterSetRBSP& asps);

    // 7.3.6.2 Point local reconstruction information syntax
    void pointLocalReconstructionInformation(Bitstream& bitstream, ParserContext& context, AtlasSequenceParameterSetRBSP& asps);

    // F.2  VUI syntax
    // F.2.1  VUI parameters syntax
    void vuiParameters(Bitstream& bitstream, VUIParameters& vp);

    // F.2.2  HRD parameters syntax
    void hrdParameters(Bitstream& bitstream, HrdParameters& hp);

    // 7.3.6.4  Atlas frame tile information syntax
    void atlasFrameTileInformation(Bitstream& bitstream, VpccParameterSet& vps, AtlasFrameTileInformation& afti);

    // 7.3.6.11  Atlas tile group header syntax
    void atlasTileGroupHeader(Bitstream& bitstream, ParserContext& context, AtlasTileGroupHeader& atgh);

    // 7.3.7.1  General atlas tile group data unit syntax
    void atlasTileGroupDataUnit(Bitstream& bitstream, ParserContext& context, AtlasTileGroupDataUnit& atgdu, AtlasTileGroupHeader& atgh);

    // 7.3.7.2  Patch information data syntax
    void patchInformationData(Bitstream& bitstream, ParserContext& context, PatchInformationData& pid, size_t patchMode, AtlasTileGroupHeader& atgh);

    // 7.3.7.3  Patch data unit syntax
    void patchDataUnit(Bitstream& bitstream, ParserContext& context, PatchDataUnit& pdu, AtlasTileGroupHeader& atgh);

    // 7.3.7.5  Merge patch data unit syntax
    void mergePatchDataUnit(Bitstream& bitstream, ParserContext& context, MergePatchDataUnit& mpdu, AtlasTileGroupHeader& atgh);

    // 7.3.7.6  Inter patch data unit syntax
    void interPatchDataUnit(Bitstream& bitstream, ParserContext& context, InterPatchDataUnit& ipdu, AtlasTileGroupHeader& atgh);

    // 7.3.7.7  Raw patch data unit syntax
    void rawPatchDataUnit(Bitstream& bitstream, ParserContext& context, RawPatchDataUnit& ppdu, AtlasTileGroupHeader& atgh);

    // 7.3.6.x EOM patch data unit syntax
    void eomPatchDataUnit(Bitstream& bitstream, ParserContext& context, EOMPatchDataUnit& epdu, AtlasTileGroupHeader& atgh);

    // 7.3.8 Supplemental enhancement information message syntax
    void seiMessage(Bitstream& bitstream, ParserContext& context, NalUnitType::Enum nalUnitType);

    // E.2  SEI payload syntax
    // E.2.1  General SEI message syntax
    void seiPayload(Bitstream& bitstream, ParserContext& context, NalUnitType::Enum nalUnitType, SeiPayloadType::Enum payloadType, size_t payloadSize);

    // E.2.2  Filler payload SEI message syntax
    void fillerPayload(Bitstream& bitstream, SEI& sei, size_t payloadSize);

    // E.2.3  User data registered by Recommendation ITU-T T.35 SEI message syntax
    void userDataRegisteredItuTT35(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.4  User data unregistered SEI message syntax
    void userDataUnregistered(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.5  Recovery point SEI message syntax
    void recoveryPoint(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.6  No display SEI message syntax
    void noDisplay(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.7  Reserved SEI message syntax
    void reservedSeiMessage(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.8  SEI manifest SEI message syntax
    void seiManifest(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.9  SEI prefix indication SEI message syntax
    void seiPrefixIndication(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.10  Geometry transformation parameters SEI message syntax
    void geometryTransformationParams(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.11  Attribute transformation parameters SEI message syntax
    void attributeTransformationParams(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.12  Active substreams SEI message syntax
    void activeSubstreams(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.13  Component codec mapping SEI message syntax
    void componentCodecMapping(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.13  Component codec mapping SEI message syntax
    void componentCodecMapping(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.14  Volumetric Tiling SEI message syntax
    // E.2.14.1  General
    void volumetricTilingInfo(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.15  Buffering period SEI message syntax
    void bufferingPeriod(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize, bool NalHrdBpPresentFlag, bool AclHrdBpPresentFlag, std::vector<uint8_t> hrdCabCntMinus1);

    // E.2.16  Atlas frame timing SEI message syntax
    void atlasFrameTiming(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize, bool CabDabDelaysPresentFlag);

    // E.2.17  Presentation inforomation SEI message syntax
    void presentationInformation(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.18  Smoothing parameters SEI message syntax
    void smoothingParameters(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize);

    // E.2.14.2  Volumetric Tiling Info Labels
    void volumetricTilingInfoLabels(Bitstream& bitstream, SEIVolumetricTilingInfo& sei);

    // E.2.14.3  Volumetric Tiling Info Objects
    void volumetricTilingInfoObjects(Bitstream& bitstream, SEIVolumetricTilingInfo& sei);

    // F.2.3  Sub-layer HRD parameters syntax
    void hrdSubLayerParameters(Bitstream& bitstream, HrdSubLayerParameters& hlsp, size_t cabCnt);


    // Helpers

    void sampleStreamVpccHeader(Bitstream& bitstream, uint32_t& ssvhUnitSizePrecisionBytesMinus1);
    SEI& addSei(ParserContext& context, NalUnitType::Enum nalUnitType, SeiPayloadType::Enum payloadType);

    void byteAlignment(Bitstream& bitstream)
    {
        BitstreamReader::readBits(bitstream, 1);

        while (!BitstreamReader::isAligned(bitstream))
        {
            BitstreamReader::readBits(bitstream, 1);
        }
    }

    uint32_t fixedLengthCodeBitsCount(uint32_t range)
    {
        uint32_t count = 0;

        if (range > 0)
        {
            range -= 1;

            while (range > 0)
            {
                count++;
                range >>= 1;
            }
        }

        return count;
    }

    void parseVideoStream(std::vector<uint8_t>& stream, std::vector<VideoFramePacket>& packets, VideoType::Enum type)
    {
        LOG_V("---------- VIDEO STREAM - BEGIN ----------");

        LOG_V("Video stream type: %s", VideoType::toString(type));
        LOG_V("Video stream size: %llu bytes", stream.size());

        std::vector<HEVC::NALUnit> nalUnits;
        HEVC::readNALUnits(stream.data(), stream.size(), nalUnits);

        std::vector<HEVC::Slice> slices;
        HEVC::parseSlices(stream.data(), stream.size(), nalUnits, slices);

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

        dumpVideoStream(stream, type);

#endif

        LOG_V("---------- VIDEO STREAM - END ----------");
    }

    void dumpVideoStream(std::vector<uint8_t>& stream, VideoType::Enum type)
    {
#if 0 // DEBUG

        // Dump encoded YUV sequence
        std::string outputPath;

        static size_t counter = 0;

        outputPath.append(std::to_string(counter));
        outputPath.append("_");
        outputPath.append(VideoType.toString(type));
        outputPath.append(".265");

        counter++;

        uint8_t* buffer = stream.data();
        size_t bufferSize = stream.size();

        FileSystem::saveToDisk(outputPath.c_str(), buffer, bufferSize);

#endif
    }

    bool parseContainerHeader(Bitstream& bitstream, TMC2Header& header)
    {
        uint32_t containerMagicNumber = BitstreamReader::readUInt32(bitstream);

        if (containerMagicNumber != TMC2ContainerMagicNumber)
        {
            return false;
        }

        uint32_t containerVersion = BitstreamReader::readUInt32(bitstream);

        if (containerVersion != TMC2ContainerVersion)
        {
            return false;
        }

        uint64_t totalSize = BitstreamReader::readUInt64(bitstream);

        header.magic = containerMagicNumber;
        header.version = containerVersion;
        header.totalSize = totalSize;

        return true;
    }

	SEI& addSei(ParserContext& context, NalUnitType::Enum nalUnitType, SeiPayloadType::Enum payloadType)
    {
        std::shared_ptr<SEI> sharedPtr;
        
        switch (payloadType)
        {
            case SeiPayloadType::Enum::BUFFERING_PERIOD:
                sharedPtr = std::make_shared<SEIBufferingPeriod>();
                break;

            case SeiPayloadType::Enum::ATLAS_FRAME_TIMING:
                sharedPtr = std::make_shared<SEIAtlasFrameTiming>();
                break;

            case SeiPayloadType::Enum::FILLER_PAYLOAD:
                break;

            case SeiPayloadType::Enum::USER_DATAREGISTERED_ITUTT35:
                sharedPtr = std::make_shared<SEIUserDataRegisteredItuTT35>();
                break;

            case SeiPayloadType::Enum::USER_DATA_UNREGISTERED:
                sharedPtr = std::make_shared<SEIUserDataUnregistered>();
                break;

            case SeiPayloadType::Enum::RECOVERY_POINT:
                sharedPtr = std::make_shared<SEIRecoveryPoint>();
                break;

            case SeiPayloadType::Enum::NO_DISPLAY:
                sharedPtr = std::make_shared<SEINoDisplay>();
                break;

            case SeiPayloadType::Enum::TIME_CODE:
                break;

            case SeiPayloadType::Enum::REGIONAL_NESTING:
                break;

            case SeiPayloadType::Enum::SEI_MANIFEST:
                sharedPtr = std::make_shared<SEIManifest>();
                break;

            case SeiPayloadType::Enum::SEI_PREFIX_INDICATION:
                sharedPtr = std::make_shared<SEIPrefixIndication>();
                break;

            case SeiPayloadType::Enum::GEOMETRY_TRANSFORMATION_PARAMS:
                sharedPtr = std::make_shared<SEIGeometryTransformationParams>();
                break;

            case SeiPayloadType::Enum::ATTRIBUTE_TRANSFORMATION_PARAMS:
                sharedPtr = std::make_shared<SEIAttributeTransformationParams>();
                break;

            case SeiPayloadType::Enum::ACTIVE_SUBSTREAMS:
                sharedPtr = std::make_shared<SEIActiveSubstreams>();
                break;

            case SeiPayloadType::Enum::COMPONENT_CODEC_MAPPING:
                sharedPtr = std::make_shared<SEIComponentCodecMapping>();
                break;

            case SeiPayloadType::Enum::VOLUMETRIC_TILING_INFO:
                sharedPtr = std::make_shared<SEIVolumetricTilingInfo>();
                break;

            case SeiPayloadType::Enum::PRESENTATION_INFORMATION:
                sharedPtr = std::make_shared<SEIPresentationInformation>();
                break;

            case SeiPayloadType::Enum::SMOOTHING_PARAMETERS:
                sharedPtr = std::make_shared<SEISmoothingParameters>();
                break;

            case SeiPayloadType::Enum::RESERVED_SEI_MESSAGE:
                sharedPtr = std::make_shared<SEIReservedSeiMessage>();
                break;

            default:
                assert(false);
                break;
        }

        if (nalUnitType == NalUnitType::Enum::PREFIX_SEI)
        {
            context.seiPrefix.push_back(sharedPtr);

            return *(context.seiPrefix.back().get());
        }
        else if (nalUnitType == NalUnitType::Enum::SUFFIX_SEI)
        {
            context.seiSuffix.push_back(sharedPtr);

            return *(context.seiSuffix.back().get());
        }
        else
        {
            assert(false);
        }

        return *(sharedPtr);
    }

    void parseVideoBitstream(Bitstream& bitstream, ParserContext& context, FrameGroup& frameGroup, VideoType::Enum videoType)
    {
        uint32_t size = BitstreamReader::readBits(bitstream, 32);

        // Fetch correct buffer
        
        VideoStream& videoStream = frameGroup.videoStream[videoType];
        videoStream.type = videoType;

        std::vector<uint8_t>* buffer = &videoStream.buffer;
        buffer->resize(size);

        std::vector<VideoFramePacket>* packets = &frameGroup.videoStream[videoType].packets;

        // Read data
        size_t bytesRead = BitstreamReader::readBytes(bitstream, buffer->data(), size);
        assert(size == bytesRead);

        // Parse HEVC video stream
        parseVideoStream(*buffer, *packets, videoType);

        // Parse decoder parameters
        HEVC::Bitstream videoBitstream(buffer->data(), buffer->size());
        HEVC::parseDecoderParameters(videoBitstream, videoStream.decoderParameters);

        // Parse VPS
        HEVC::Bitstream vpsBitstream(videoStream.decoderParameters.vps.data(), videoStream.decoderParameters.vps.size());
        HEVC::parseVPS(vpsBitstream, videoStream.vps);

        // Parse SPS
        HEVC::Bitstream spsBitstream(videoStream.decoderParameters.sps.data(), videoStream.decoderParameters.sps.size());
        HEVC::parseSPS(spsBitstream, videoStream.sps);
    }

    // VPCC bitstream

	void videoSubStream(Bitstream& bitstream, ParserContext& context, FrameGroup& frameGroup, VPCCUnitType::Enum vpccUnitType)
    {
        size_t atlasIndex = 0;
        
        if (vpccUnitType == VPCCUnitType::OVD)
        {
            parseVideoBitstream(bitstream, context, frameGroup, VideoType::OCCUPANCY);

        }
        else if (vpccUnitType == VPCCUnitType::GVD)
        {
            VpccUnitHeader& vpccUnitHeader = context.vpccUnitHeader[(size_t)VPCCUnitType::GVD];
            
            if (vpccUnitHeader.rawVideoFlag)
            {
                parseVideoBitstream(bitstream, context, frameGroup, VideoType::GEOMETRY_RAW);
            }
            else
            {
                VpccParameterSet& vps = context.getActiveVps();
                
                if (vps.mapCountMinus1[atlasIndex] > 0 && vps.multipleMapStreamsPresentFlag[atlasIndex])
                {
                    if (vpccUnitHeader.mapIndex == 0)
                    {
                        parseVideoBitstream(bitstream, context, frameGroup, VideoType::GEOMETRY_D0);
                    }
                    else if (vpccUnitHeader.mapIndex == 1)
                    {
                        parseVideoBitstream(bitstream, context, frameGroup, VideoType::GEOMETRY_D1);
                    }
                }
                else
                {
                    parseVideoBitstream(bitstream, context, frameGroup, VideoType::GEOMETRY);
                }
            }
        }
        else if (vpccUnitType == VPCCUnitType::AVD)
        {
            VpccUnitHeader& vpccUnitHeader = context.vpccUnitHeader[(size_t)VPCCUnitType::AVD];
            VpccParameterSet& vps = context.getActiveVps();
            
            if (vps.attributeInformation[atlasIndex].attributeCount > 0)
            {
                if (vpccUnitHeader.rawVideoFlag)
                {
                    parseVideoBitstream(bitstream, context, frameGroup, VideoType::TEXTURE_RAW);
                }
                else
                {
                    if (vps.mapCountMinus1[atlasIndex] > 0 && vps.multipleMapStreamsPresentFlag[atlasIndex])
                    {
                        if (vpccUnitHeader.mapIndex == 0)
                        {
                            parseVideoBitstream(bitstream, context, frameGroup, VideoType::TEXTURE_T0);
                        }
                        else if (vpccUnitHeader.mapIndex == 1)
                        {
                            parseVideoBitstream(bitstream, context, frameGroup, VideoType::TEXTURE_T1);
                        }
                    }
                    else
                    {
                        parseVideoBitstream(bitstream, context, frameGroup, VideoType::TEXTURE);
                    }
                }
            }
        }
    }

    void profileTierLevel(Bitstream& bitstream, ParserContext& context, ProfileTierLevel& ptl)
    {
        ptl.tierFlag = BitstreamReader::readBits(bitstream, 1);
        ptl.profileCodecGroupIdc = BitstreamReader::readBits(bitstream, 7);
        ptl.profilePccToolsetIdc = BitstreamReader::readBits(bitstream, 8);
        ptl.profileReconctructionIdc = BitstreamReader::readBits(bitstream, 8);

        BitstreamReader::readBits(bitstream, 32);

        ptl.levelIdc = BitstreamReader::readBits(bitstream, 8);
    }

    void pointLocalReconstructionInformation(Bitstream& bitstream, ParserContext& context, PointLocalReconstructionInformation& plri)
    {
        plri.numberOfModesMinus1 = BitstreamReader::readBits(bitstream, 4);

        plri.minimumDepth.resize(plri.numberOfModesMinus1 + 1, 0);
        plri.neighbourMinus1.resize(plri.numberOfModesMinus1 + 1, 0);
        plri.interpolateFlag.resize(plri.numberOfModesMinus1 + 1, false);
        plri.fillingFlag.resize(plri.numberOfModesMinus1 + 1, false);

        for (size_t i = 0; i <= plri.numberOfModesMinus1; i++)
        {
            bool interpolateFlag = BitstreamReader::readBits(bitstream, 1);
            bool fillingFlag = BitstreamReader::readBits(bitstream, 1);
            
            uint8_t minimumDepth = BitstreamReader::readBits(bitstream, 2);
            uint8_t neighbourMinus1 = BitstreamReader::readBits(bitstream, 2);

            plri.interpolateFlag[i] = interpolateFlag;
            plri.fillingFlag[i] = fillingFlag;
            plri.minimumDepth[i] = minimumDepth;
            plri.neighbourMinus1[i] = neighbourMinus1;
        }

        uint32_t uvlc = BitstreamReader::readUVLC(bitstream);
        plri.blockThresholdPerPatchMinus1 = uvlc;
    }

    bool vpccUnit(Bitstream& bitstream, ParserContext& context, FrameGroup& frameGroup, VPCCUnitType::Enum& vpccUnitType)
    {
        if (vpccUnitHeader(bitstream, context, vpccUnitType))
        {
            return vpccUnitPayload(bitstream, context, frameGroup, vpccUnitType);
        }

        return false;
    }

    bool vpccUnitHeader(Bitstream& bitstream, ParserContext& context, VPCCUnitType::Enum& vpccUnitType)
    {
        size_t vpccUnitSize = BitstreamReader::readBits(bitstream, (8 * (context.ssvhUnitSizePrecisionBytesMinus1 + 1)));
        size_t start = bitstream.position;

        vpccUnitType = (VPCCUnitType::Enum) BitstreamReader::readBits(bitstream, 5);

        VpccUnitHeader& vpccUnitHeader = context.vpccUnitHeader[(size_t)vpccUnitType];
        vpccUnitHeader.unitType = vpccUnitType;
        vpccUnitHeader.unitSize = vpccUnitSize;
        vpccUnitHeader.unitPos = start;

		if (vpccUnitType == VPCCUnitType::AVD || vpccUnitType == VPCCUnitType::GVD || vpccUnitType == VPCCUnitType::OVD || vpccUnitType == VPCCUnitType::AD)
        {
			vpccUnitHeader.sequenceParamterSetId = BitstreamReader::readBits(bitstream, 4);
            vpccUnitHeader.atlasId = BitstreamReader::readBits(bitstream, 6);
            
            context.setActiveVps(vpccUnitHeader.sequenceParamterSetId);
        }

        if (vpccUnitType == VPCCUnitType::AVD)
        {
            vpccUnitHeader.attributeIndex = BitstreamReader::readBits(bitstream, 7);
            vpccUnitHeader.attributeDimensionIndex = BitstreamReader::readBits(bitstream, 5);
            vpccUnitHeader.mapIndex = BitstreamReader::readBits(bitstream, 4);
            vpccUnitHeader.rawVideoFlag = BitstreamReader::readBits(bitstream, 1);
        }
        else if (vpccUnitType == VPCCUnitType::GVD)
        {
            vpccUnitHeader.mapIndex = BitstreamReader::readBits(bitstream, 4);
            vpccUnitHeader.rawVideoFlag = BitstreamReader::readBits(bitstream, 1);

            BitstreamReader::readBits(bitstream, 12);
        }
        else if (vpccUnitType == VPCCUnitType::OVD || vpccUnitType == VPCCUnitType::AD)
        {
            BitstreamReader::readBits(bitstream, 17);
        }
        else
        {
            BitstreamReader::readBits(bitstream, 27);
        }

        return true;
    }

    bool vpccUnitPayload(Bitstream& bitstream, ParserContext& context, FrameGroup& frameGroup, VPCCUnitType::Enum& vpccUnitType)
    {
        if (vpccUnitType == VPCCUnitType::VPS)
        {
            vpccParameterSet(bitstream, context);
        }
        else if (vpccUnitType == VPCCUnitType::AD)
        {
            atlasSubStream(bitstream, context);
        }
        else if (vpccUnitType == VPCCUnitType::OVD || vpccUnitType == VPCCUnitType::GVD || vpccUnitType == VPCCUnitType::AVD)
        {
            videoSubStream(bitstream, context, frameGroup, vpccUnitType);
        }

        return true;
    }

    void vpccParameterSet(Bitstream& bitstream, ParserContext& context)
    {
        VpccParameterSet& vps = context.addVpccParameterSet();

        profileTierLevel(bitstream, context, vps.profileTierLevel);

        vps.vpccParameterSetId = BitstreamReader::readBits(bitstream, 4);
        vps.atlasCountMinus1 = BitstreamReader::readBits(bitstream, 6);
        
        vps.allocateAtlas();

        for (uint32_t j = 0; j < vps.atlasCountMinus1 + 1; j++)
        {
            vps.frameWidth[j]  = BitstreamReader::readBits(bitstream, 16);
            vps.frameHeight[j] = BitstreamReader::readBits(bitstream, 16);

            vps.mapCountMinus1[j] = BitstreamReader::readBits(bitstream, 4);
            
            vps.allocateMap(j);
            
            if (vps.mapCountMinus1[j] > 0)
            {
                vps.multipleMapStreamsPresentFlag[j] = BitstreamReader::readBits(bitstream, 1);
            }
            
            vps.mapAbsoluteCodingEnableFlag[j][0] = 1;
            
            for (size_t i = 1; i <= vps.mapCountMinus1[j]; i++)
            {
                if (vps.multipleMapStreamsPresentFlag[j])
                {
                    vps.mapAbsoluteCodingEnableFlag[j][i] = BitstreamReader::readBits(bitstream, 1);
                }
                else
                {
                    vps.mapAbsoluteCodingEnableFlag[j][i] = 1;
                }
                
                if (vps.mapAbsoluteCodingEnableFlag[j][i] == 0)
                {
                    if (i > 0)
                    {
                        uint32_t uvlc = BitstreamReader::readUVLC(bitstream);
                        vps.mapPredictorIndexDiff[j][i] = uvlc;
                    }
                    else
                    {
                        vps.mapPredictorIndexDiff[j][i] = 0;
                    }
                }
            }
            
            vps.rawPatchEnabledFlag[j] = BitstreamReader::readBits(bitstream, 1);
            
            if (vps.rawPatchEnabledFlag[j])
            {
                vps.rawSeparateVideoPresentFlag[j] = BitstreamReader::readBits(bitstream, 1);
            }

            occupancyInformation(vps.occupancyInformation[j], bitstream);
            geometryInformation(vps.geometryInformation[j], vps, bitstream);
            attributeInformation(vps.attributeInformation[j], vps, bitstream);
        }

        vps.extensionPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (vps.extensionPresentFlag)
        {
            uint32_t uvlc = BitstreamReader::readUVLC(bitstream);
            vps.extensionLength = uvlc;

            vps.extensionDataByte.resize(vps.extensionLength);
            
            for (size_t i = 0; i < vps.extensionLength; i++)
            {
                vps.extensionDataByte[i] = BitstreamReader::readBits(bitstream, 8);
            }
        }

        // THE NEXT PARAMETERS ARE NOT IN THE VPCC CD SYNTAX DOCUMENTS AND WILL BE REMOVE
        vps.losslessGeo444 = BitstreamReader::readBits(bitstream, 1);
        vps.losslessGeo = BitstreamReader::readBits(bitstream, 1);
        vps.minLevel = BitstreamReader::readBits(bitstream, 8);
        // THE NEXT PARAMETERS ARE NOT IN THE VPCC CD SYNTAX DOCUMENTS AND WILL BE REMOVE

        byteAlignment(bitstream);
    }

    // 7.3.4.3 Occupancy parameter set syntax
    void occupancyInformation(OccupancyInformation& oi, Bitstream& bitstream)
    {
        oi.occupancyCodecId = BitstreamReader::readBits(bitstream, 8);
        oi.lossyOccupancyMapCompressionThreshold = BitstreamReader::readBits(bitstream, 8);
        oi.occupancyNominal2DBitdepthMinus1 = BitstreamReader::readBits(bitstream, 5);
        oi.occupancyMSBAlignFlag = BitstreamReader::readBits(bitstream, 1);
    }

    // 7.3.4.4 Geometry parameter set syntax
    void geometryInformation(GeometryInformation& gi, VpccParameterSet& vps, Bitstream& bitstream)
    {
        size_t atlasIndex = 0;
        gi.geometryCodecId = BitstreamReader::readBits(bitstream, 8);
        gi.geometryNominal2dBitdepthMinus1 = BitstreamReader::readBits(bitstream, 5);
        gi.geometryMSBAlignFlag = BitstreamReader::readBits(bitstream, 1);
        gi.geometry3dCoordinatesBitdepthMinus1 = BitstreamReader::readBits(bitstream, 5);
        
        if (vps.rawSeparateVideoPresentFlag[atlasIndex])
        {
            gi.rawGeometryCodecId = BitstreamReader::readBits(bitstream, 8);
        }
    }

    // 7.3.4.5 Attribute information
    void attributeInformation(AttributeInformation& ai, VpccParameterSet& vps, Bitstream& bitstream)
    {
        ai.attributeCount = BitstreamReader::readBits(bitstream, 7);
        
        ai.allocate();
        
        size_t atlasIndex = 0;
        
        for (uint32_t i = 0; i < ai.attributeCount; i++)
        {
            ai.attributeTypeId[i]  = BitstreamReader::readBits(bitstream, 4);
            ai.attributeCodecId[i] = BitstreamReader::readBits(bitstream, 8);
            
            if (vps.rawSeparateVideoPresentFlag[atlasIndex])
            {
                ai.rawAttributeCodecId[i] = BitstreamReader::readBits(bitstream, 8);
            }
            
            ai.addAttributeMapAbsoluteCodingEnabledFlag(i, true);
            
            for (int32_t j = 0; j < vps.mapCountMinus1[atlasIndex]; j++)
            {
                if (vps.mapAbsoluteCodingEnableFlag[atlasIndex][j] == 0)
                {
                    ai.addAttributeMapAbsoluteCodingEnabledFlag(i, BitstreamReader::readBits(bitstream, 1));
                }
                else
                {
                    ai.addAttributeMapAbsoluteCodingEnabledFlag(i, true);
                }
            }
            
            ai.attributeDimensionMinus1[i] = BitstreamReader::readBits(bitstream, 6);
            
            if (ai.attributeDimensionMinus1[i] > 0)
            {
                ai.attributeDimensionPartitionsMinus1[i] = BitstreamReader::readBits(bitstream, 6);
                
                int32_t remainingDimensions = ai.attributeDimensionMinus1[i];
                int32_t k = ai.attributeDimensionPartitionsMinus1[i];
                
                for (int32_t j = 0; j < k; j++)
                {
                    if (k - j == remainingDimensions)
                    {
                        ai.setAttributePartitionChannelsMinus1(i, j, 0);
                    }
                    else
                    {
                        uint32_t uvlc = BitstreamReader::readUVLC(bitstream);
                        ai.setAttributePartitionChannelsMinus1(i, j, uvlc);
                    }
                    
                    remainingDimensions -= ai.attributePartitionChannelsMinus1[i][j] + 1;
                }
                
                ai.setAttributePartitionChannelsMinus1(i, k, remainingDimensions);
            }
            
            ai.attributeNominal2dBitdepthMinus1[i] = BitstreamReader::readBits(bitstream, 5);
        }
        
        if (ai.attributeCount > 0)
        {
            ai.attributeMSBAlignFlag = BitstreamReader::readBits(bitstream, 1);
        }
    }
		
	void atlasSubStream(Bitstream& bitstream, ParserContext& context)
    {
        VpccUnitHeader& vpccUnitHeader = context.vpccUnitHeader[(size_t)VPCCUnitType::AD];
        int64_t sizeBitstream = vpccUnitHeader.unitPos + vpccUnitHeader.unitSize;

        SampleStreamNalUnit ssnu;
        sampleStreamNalHeader(bitstream, ssnu);
        
        while (bitstream.position < sizeBitstream)
        {
            ssnu.addNalUnit();
            
            sampleStreamNalUnit(bitstream, context, ssnu, ssnu.nalUnit.size() - 1);
        }
    }

    void sampleStreamNalHeader(Bitstream& bitstream, SampleStreamNalUnit& ssnu)
    {
        ssnu.unitSizePrecisionBytesMinus1 = BitstreamReader::readBits(bitstream, 3);
        BitstreamReader::readBits(bitstream, 5);
    }

    void sampleStreamNalUnit(Bitstream& bitstream, ParserContext& context, SampleStreamNalUnit& ssnu, size_t index)
    {
        NalUnit& nu = ssnu.nalUnit[index];
        nu.nalUnitSize = BitstreamReader::readBits(bitstream, (8 * (ssnu.unitSizePrecisionBytesMinus1 + 1)));
        
        nalUnitHeader(bitstream, nu);

        switch (nu.nalUnitType)
        {
            case NalUnitType::ASPS:
                atlasSequenceParameterSetRBSP(bitstream, context, context.addAtlasSequenceParameterSet());
                break;
                    
            case NalUnitType::AFPS:
                atlasFrameParameterSetRbsp(bitstream, context, context.addAtlasFrameParameterSet());
                break;
                    
            case NalUnitType::TRAIL:
            case NalUnitType::TSA:
            case NalUnitType::STSA:
            case NalUnitType::RADL:
            case NalUnitType::RASL:
            case NalUnitType::SKIP:
                atlasTileGroupLayerRbsp(bitstream, context, context.addAtlasTileGroupLayer());
                break;
                    
            case NalUnitType::SUFFIX_SEI:
                seiRbsp(bitstream, context, nu.nalUnitType);
                break;
                    
            case NalUnitType::PREFIX_SEI:
                seiRbsp(bitstream, context, nu.nalUnitType);
                break;
                    
            default:
                assert(0);
        }
    }
	
    void nalUnitHeader(Bitstream& bitstream, NalUnit& nalUnit)
    {
        BitstreamReader::readBits(bitstream, 1);
        
        nalUnit.nalUnitType = (NalUnitType::Enum) BitstreamReader::readBits(bitstream, 6);
        nalUnit.layerId = BitstreamReader::readBits(bitstream, 6);
        nalUnit.temporalyIdPlus1 = BitstreamReader::readBits(bitstream, 3);
    }

    void atlasSequenceParameterSetRBSP(Bitstream& bitstream, ParserContext& context, AtlasSequenceParameterSetRBSP& asps)
    {
        asps.altasSequenceParameterSetId = BitstreamReader::readUVLC(bitstream);
        asps.frameWidth = BitstreamReader::readBits(bitstream, 16);
        asps.frameHeight = BitstreamReader::readBits(bitstream, 16);
        asps.log2PatchPackingBlockSize = BitstreamReader::readBits(bitstream, 3);
        asps.log2MaxAtlasFrameOrderCntLsbMinus4 = BitstreamReader::readUVLC(bitstream);
        asps.maxDecAtlasFrameBufferingMinus1 = BitstreamReader::readUVLC(bitstream);
        asps.longTermRefAtlasFramesFlag = BitstreamReader::readBits(bitstream, 1);
        asps.numRefAtlasFrameListsInAsps = BitstreamReader::readUVLC(bitstream);

        asps.allocateRefListStruct();
        
        for (size_t i = 0; i < asps.numRefAtlasFrameListsInAsps; i++)
        {
            refListStruct(bitstream, asps.refListStruct[i], asps);
        }
        
        asps.longTermRefAtlasFramesFlag = BitstreamReader::readBits(bitstream, 1);
        asps.degree45ProjectionPatchPresentFlag = BitstreamReader::readBits(bitstream, 1);
        asps.normalAxisLimitsQuantizationEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        asps.normalAxisMaxDeltaValueEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        asps.removeDuplicatePointEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        asps.pixelDeinterleavingFlag = BitstreamReader::readBits(bitstream, 1);
        asps.patchPrecedenceOrderFlag = BitstreamReader::readBits(bitstream, 1);
        asps.patchSizeQuantizerPresentFlag = BitstreamReader::readBits(bitstream, 1);
        asps.enhancedOccupancyMapForDepthFlag = BitstreamReader::readBits(bitstream, 1);
        asps.pointLocalReconstructionEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        asps.mapCountMinus1 = BitstreamReader::readBits(bitstream, 4);
        
        if (asps.enhancedOccupancyMapForDepthFlag && asps.mapCountMinus1 == 0)
        {
            asps.enhancedOccupancyMapFixBitCountMinus1 = BitstreamReader::readBits(bitstream, 4);
        }
        
        if (asps.pointLocalReconstructionEnabledFlag)
        {
            pointLocalReconstructionInformation(bitstream, context, asps);
        }
        
        if (asps.pixelDeinterleavingFlag || asps.pointLocalReconstructionEnabledFlag)
        {
            asps.surfaceThicknessMinus1 = BitstreamReader::readBits(bitstream, 8);
        }
        
        asps.vuiParametersPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (asps.vuiParametersPresentFlag)
        {
            vuiParameters(bitstream, asps.vuiParameters);
        }
        
        asps.extensionPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (asps.extensionPresentFlag)
        {
            while (false /*moreRbspData(bitstream)*/)
            {
                asps.extensionPresentFlag = BitstreamReader::readBits(bitstream, 1);
            }
        }

        byteAlignment(bitstream);
    }

    void pointLocalReconstructionInformation(Bitstream& bitstream, ParserContext& context, AtlasSequenceParameterSetRBSP& asps)
    {
        asps.allocatePointLocalReconstructionInformation();
        
        for (size_t j = 0; j < asps.mapCountMinus1 + 1; j++)
        {
            PointLocalReconstructionInformation& plri = asps.pointLocalReconstructionInformation[j];
            plri.mapEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (plri.mapEnabledFlag)
            {
                plri.numberOfModesMinus1 = BitstreamReader::readBits(bitstream, 4);
                
                plri.allocate();
                
                for (size_t i = 0; i < plri.numberOfModesMinus1; i++)
                {
                    plri.interpolateFlag[i] = BitstreamReader::readBits(bitstream, 1);
                    plri.fillingFlag[i] = BitstreamReader::readBits(bitstream, 1);
                    plri.minimumDepth[i] = BitstreamReader::readBits(bitstream, 2);
                    plri.neighbourMinus1[i] = BitstreamReader::readBits(bitstream, 2);
                }
                
                plri.blockThresholdPerPatchMinus1 = BitstreamReader::readBits(bitstream, 6);
            }
        }
    }

    void refListStruct(Bitstream& bitstream, RefListStruct& rls, AtlasSequenceParameterSetRBSP& asps)
    {
        rls.numRefEntries = BitstreamReader::readUVLC(bitstream);
        
        rls.allocate();
        
        for (size_t i = 0; i < rls.numRefEntries; i++)
        {
            if (asps.longTermRefAtlasFramesFlag)
            {
                rls.stRefAtlasFrameFlag[i] = BitstreamReader::readBits(bitstream, 1);
            }
            else
            {
                rls.stRefAtlasFrameFlag[i] = 1;
            }
            
            if (rls.stRefAtlasFrameFlag[i])
            {
                rls.absDeltaAfocSt[i] = BitstreamReader::readUVLC(bitstream);
                
                if (rls.absDeltaAfocSt[i] > 0)
                {
                    rls.strpfEntrySignFlag[i] = BitstreamReader::readBits(bitstream, 1);
                }
                else
                {
                    rls.strpfEntrySignFlag[i] = 1;
                }
            }
            else
            {
                uint8_t bitCount = asps.log2MaxAtlasFrameOrderCntLsbMinus4 + 4;
                rls.afocLsbLt[i] = BitstreamReader::readBits(bitstream, bitCount);
            }
        }
    }

    void atlasFrameParameterSetRbsp(Bitstream& bitstream, ParserContext& context, AtlasFrameParameterSetRbsp& afps)
    {
        afps.afpsAtlasFrameParameterSetId = BitstreamReader::readUVLC(bitstream);
        afps.afpsAtlasSequenceParameterSetId = BitstreamReader::readUVLC(bitstream);

        atlasFrameTileInformation(bitstream, context.getActiveVps(), afps.atlasFrameTileInformation);
        
        afps.afpsNumRefIdxDefaultActiveMinus1 = BitstreamReader::readUVLC(bitstream);
        afps.afpsAdditionalLtAfocLsbLen  = BitstreamReader::readUVLC(bitstream);
        afps.afps2dPosXBitCountMinus1 = BitstreamReader::readBits(bitstream, 4);
        afps.afps2dPosYBitCountMinus1 = BitstreamReader::readBits(bitstream, 4);
        afps.afps3dPosXBitCountMinus1 = BitstreamReader::readBits(bitstream, 5);
        afps.afps3dPosYBitCountMinus1 = BitstreamReader::readBits(bitstream, 5);
        afps.afpsOverrideEomForDepthFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (afps.afpsOverrideEomForDepthFlag)
        {
            afps.afpsEomNumberOfPatchBitCountMinus1 = BitstreamReader::readBits(bitstream, 4);
            afps.afpsEomMaxBitCountMinus1 = BitstreamReader::readBits(bitstream, 4);
        }
        
        afps.afpsRaw3dPosBitCountExplicitModeFlag = BitstreamReader::readBits(bitstream, 1);
        afps.afpsExtensionPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (afps.afpsExtensionPresentFlag)
        {
            while (false /*moreRbspData(bitstream)*/)
            {
                afps.afpsExtensionDataFlag = BitstreamReader::readBits(bitstream, 1);
            }
        }
        
        byteAlignment(bitstream);
    }
	
    void atlasFrameTileInformation(Bitstream& bitstream, VpccParameterSet& vps, AtlasFrameTileInformation& afti)
    {
        afti.singleTileInAtlasFrameFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (!afti.singleTileInAtlasFrameFlag)
        {
            afti.uniformTileSpacingFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (afti.uniformTileSpacingFlag)
            {
                afti.tileColumnWidthMinus1[0] = BitstreamReader::readUVLC(bitstream);
                afti.tileRowHeightMinus1[0] = BitstreamReader::readUVLC(bitstream);
            }
            else
            {
                afti.numTileColumnsMinus1 = BitstreamReader::readUVLC(bitstream);
                afti.numTileRowsMinus1 = BitstreamReader::readUVLC(bitstream);
                
                for (size_t i = 0; i < afti.numTileColumnsMinus1; i++)
                {
                    afti.tileColumnWidthMinus1[i] = BitstreamReader::readUVLC(bitstream);
                }
                
                for (size_t i = 0; i < afti.numTileRowsMinus1; i++)
                {
                    afti.tileRowHeightMinus1[i] = BitstreamReader::readUVLC(bitstream);
                }
            }
        }

        afti.singleTilePerTileGroupFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (!afti.singleTilePerTileGroupFlag)
        {
            uint32_t NumTilesInPatchFrame = (afti.numTileColumnsMinus1 + 1) * (afti.numTileRowsMinus1 + 1);

            afti.numTileGroupsInAtlasFrameMinus1 = BitstreamReader::readUVLC(bitstream);
            
            for (size_t i = 0; i <= afti.numTileGroupsInAtlasFrameMinus1; i++)
            {
                uint8_t bitCount = fixedLengthCodeBitsCount(NumTilesInPatchFrame + 1);
                
                if (i > 0)
                {
                    afti.topLeftTileIdx[i] = BitstreamReader::readBits(bitstream, bitCount);
                }
                
                bitCount = fixedLengthCodeBitsCount(NumTilesInPatchFrame - afti.topLeftTileIdx[i] + 1);
                afti.bottomRightTileIdxDelta[i] = BitstreamReader::readBits(bitstream, bitCount);
            }
        }
        
        afti.signalledTileGroupIdFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (afti.signalledTileGroupIdFlag)
        {
            afti.signalledTileGroupIdLengthMinus1 = BitstreamReader::readUVLC(bitstream);
            
            for (size_t i = 0; i <= afti.signalledTileGroupIdLengthMinus1; i++)
            {
                uint8_t bitCount = afti.signalledTileGroupIdLengthMinus1 + 1;
                afti.tileGroupId[i] = BitstreamReader::readBits(bitstream, bitCount);
            }
        }
    }
	
    void pointLocalReconstructionData(Bitstream& bitstream, ParserContext& context, PointLocalReconstructionData& plrd, AtlasSequenceParameterSetRBSP& asps)
    {
        PointLocalReconstructionInformation& plri = asps.pointLocalReconstructionInformation[0];
        
        const size_t blockCount = plrd.blockToPatchMapWidth * plrd.blockToPatchMapHeight;
        const uint8_t bitCountMode = uint8_t(fixedLengthCodeBitsCount(uint32_t(plri.numberOfModesMinus1)));
        
        if (blockCount > plri.blockThresholdPerPatchMinus1 + 1)
        {
            plrd.levelFlag = BitstreamReader::readBits(bitstream, 1);
        }
        else
        {
            plrd.levelFlag = true;
        }
        
        if (plrd.levelFlag)
        {
            plrd.presentFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (plrd.presentFlag)
            {
                plrd.modeMinus1 = BitstreamReader::readBits(bitstream, bitCountMode);
            }
        }
        else
        {
            for (size_t i = 0; i < blockCount; i++)
            {
                plrd.blockPresentFlag[i] = BitstreamReader::readBits(bitstream, 1);
                
                if (plrd.blockPresentFlag[i])
                {
                    plrd.blockModeMinus1[i] = BitstreamReader::readBits(bitstream, bitCountMode);
                }
            }
        }
    }

    void atlasTileGroupLayerRbsp(Bitstream& bitstream, ParserContext& context, AtlasTileGroupLayerRbsp& atgl)
    {
        atlasTileGroupHeader(bitstream, context, atgl.atlasTileGroupHeader);
        
        if (atgl.atlasTileGroupHeader.atghType != TileGroup::Enum::SKIP)
        {
            atlasTileGroupDataUnit(bitstream, context, atgl.atlasTileGroupDataUnit, atgl.atlasTileGroupHeader);
        }
        
        byteAlignment(bitstream);
    }

    void atlasTileGroupHeader(Bitstream& bitstream, ParserContext& context, AtlasTileGroupHeader& atgh)
    {
        atgh.atghAtlasFrameParameterSetId = BitstreamReader::readUVLC(bitstream);
        
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        size_t aspsId = afps.afpsAtlasSequenceParameterSetId;
        AtlasSequenceParameterSetRBSP& asps = context.atlasSequenceParameterSet[aspsId];
        
        AtlasFrameTileInformation& afti = afps.atlasFrameTileInformation;

        atgh.atghAddress = BitstreamReader::readBits(bitstream, afti.signalledTileGroupIdLengthMinus1 + 1);
        atgh.atghType = TileGroup::Enum(BitstreamReader::readUVLC(bitstream));
        atgh.atghAtlasFrmOrderCntLsb = BitstreamReader::readBits(bitstream, asps.log2MaxAtlasFrameOrderCntLsbMinus4 + 4);
        
        if (asps.numRefAtlasFrameListsInAsps > 0)
        {
            atgh.atghRefAtlasFrameListSpsFlag = BitstreamReader::readBits(bitstream, 1);
        }
        else
        {
            atgh.atghRefAtlasFrameListSpsFlag = 0;
        }
        
        atgh.atghRefAtlasFrameListIdx = 0;
        
        if (atgh.atghRefAtlasFrameListSpsFlag == 0)
        {
            refListStruct(bitstream, atgh.refListStruct, asps);
        }
        else if (asps.numRefAtlasFrameListsInAsps > 1)
        {
            size_t bitCount = fixedLengthCodeBitsCount(asps.numRefAtlasFrameListsInAsps + 1);
            atgh.atghRefAtlasFrameListIdx = BitstreamReader::readUVLC(bitstream);
        }
        
        if (atgh.atghRefAtlasFrameListSpsFlag)
        {
            atgh.refListStruct = asps.refListStruct[atgh.atghRefAtlasFrameListIdx];
        }
        
        uint8_t rlsIdx = atgh.atghRefAtlasFrameListIdx;
        RefListStruct& refList = atgh.atghRefAtlasFrameListSpsFlag ? asps.refListStruct[rlsIdx] : atgh.refListStruct;
        
		size_t numLtrAtlasFrmEntries = 0;
        
        for (size_t i = 0; i < refList.numRefEntries; i++)
        {
            if (!refList.stRefAtlasFrameFlag[i])
            {
                numLtrAtlasFrmEntries++;
            }
        }
        
        for (size_t j = 0; j < numLtrAtlasFrmEntries; j++)
        {
            atgh.atghAdditionalAfocLsbPresentFlag[j] = BitstreamReader::readBits(bitstream, 1);
            
            if (atgh.atghAdditionalAfocLsbPresentFlag[j])
            {
                uint8_t bitCount = afps.afpsAdditionalLtAfocLsbLen;
                atgh.atghAdditionalAfocLsbVal[j] = BitstreamReader::readBits(bitstream, bitCount);
            }
        }
        
        if (atgh.atghType != TileGroup::Enum::SKIP)
        {
            if (asps.normalAxisLimitsQuantizationEnabledFlag)
            {
                atgh.atghPosMinZQuantizer = BitstreamReader::readBits(bitstream, 5);
                
                if (asps.normalAxisMaxDeltaValueEnabledFlag)
                {
                    atgh.atghPosDeltaMaxZQuantizer = BitstreamReader::readBits(bitstream, 5);
                }
            }
            
            if (asps.patchSizeQuantizerPresentFlag)
            {
                atgh.atghPatchSizeXinfoQuantizer = BitstreamReader::readBits(bitstream, 3);
                atgh.atghPatchSizeYinfoQuantizer = BitstreamReader::readBits(bitstream, 3);
            }
			
			VpccUnitHeader& vpccUnitHeader = context.vpccUnitHeader[(size_t)VPCCUnitType::AD];
            GeometryInformation& gi = context.getActiveVps().geometryInformation[0];
            
            if (afps.afpsRaw3dPosBitCountExplicitModeFlag)
            {
                size_t bitCount = fixedLengthCodeBitsCount(gi.geometry3dCoordinatesBitdepthMinus1 + 1);
                atgh.atghRaw3dPosAxisBitCountMinus1 = BitstreamReader::readBits(bitstream, bitCount);
            }
            else
            {
                atgh.atghRaw3dPosAxisBitCountMinus1 = gi.geometry3dCoordinatesBitdepthMinus1 - gi.geometryNominal2dBitdepthMinus1 - 1;
            }
            
            if (atgh.atghType == TileGroup::Enum::P && refList.numRefEntries > 1)
            {
                atgh.atghNumRefIdxActiveOverrideFlag = BitstreamReader::readBits(bitstream, 1);
                
                if (atgh.atghNumRefIdxActiveOverrideFlag)
                {
                    atgh.atghNumRefIdxActiveMinus1 = BitstreamReader::readUVLC(bitstream);
                }
            }
        }
        
        byteAlignment(bitstream);
    }

    // TODO: Move to context
	int32_t prevPatchSizeU;
    int32_t prevPatchSizeV;
    int32_t predPatchIndex;
    int32_t prevFrameIndex;
	
    void atlasTileGroupDataUnit(Bitstream& bitstream, ParserContext& context, AtlasTileGroupDataUnit& atgdu, AtlasTileGroupHeader& atgh)
    {
        size_t patchIndex = 0;
        TileGroup::Enum tileGroupType = atgh.atghType;
        uint8_t patchMode = BitstreamReader::readUVLC(bitstream);
        
        prevPatchSizeU = 0;
        prevPatchSizeV = 0;
        predPatchIndex = 0;
        
        while (!(((TileGroup::Enum(tileGroupType) == TileGroup::Enum::I) && (patchMode == PatchModeI::Enum::END)) || ((TileGroup::Enum(tileGroupType) == TileGroup::Enum::P) && (patchMode == PatchModeP::Enum::END))))
        {
            auto& pid = atgdu.addPatchInformationData(patchMode);
            pid.frameIndex = atgdu.frameIndex;
            pid.patchIndex = patchIndex;
            
            patchIndex++;
            
            patchInformationData(bitstream, context, pid, patchMode, atgh);
            
            patchMode = BitstreamReader::readUVLC(bitstream);
        }
        
        prevFrameIndex = atgdu.frameIndex;
        
        byteAlignment(bitstream);
    }

    void patchInformationData(Bitstream& bitstream, ParserContext& context, PatchInformationData& pid, size_t patchMode, AtlasTileGroupHeader& atgh)
    {
        if ((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::P && patchMode == PatchModeP::Enum::SKIP)
        {
        }
        else if ((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::P && patchMode == PatchModeP::Enum::MERGE)
        {
            MergePatchDataUnit& mpdu = pid.mergePatchDataUnit;
            mpdu.mpduFrameIndex = pid.frameIndex;
            mpdu.mpduPatchIndex = pid.patchIndex;
            
            mergePatchDataUnit(bitstream, context, mpdu, atgh);
        }
        else if ((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::P && patchMode == PatchModeP::Enum::INTER)
        {
            InterPatchDataUnit& ipdu = pid.interPatchDataUnit;
            ipdu.ipduFrameIndex = pid.frameIndex;
            ipdu.ipduPatchIndex = pid.patchIndex;
            
            interPatchDataUnit(bitstream, context, ipdu, atgh);
        }
        else if (((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::I && patchMode == PatchModeI::Enum::INTRA) || ((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::P && patchMode == PatchModeP::Enum::INTRA))
        {
            PatchDataUnit& pdu = pid.patchDataUnit;
            pdu.pduFrameIndex = pid.frameIndex;
            pdu.pduPatchIndex = pid.patchIndex;
            
            patchDataUnit(bitstream, context, pdu, atgh);
        }
        else if (((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::I && patchMode == PatchModeI::Enum::RAW) || ((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::P && patchMode == PatchModeP::Enum::RAW))
        {
            RawPatchDataUnit& rpdu = pid.rawPatchDataUnit;
            rpdu.rpduFrameIndex = pid.frameIndex;
            rpdu.rpduPatchIndex = pid.patchIndex;
            
            rawPatchDataUnit(bitstream, context, rpdu, atgh);
        }
        else if (((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::I && patchMode == PatchModeI::Enum::EOM) || ((TileGroup::Enum(atgh.atghType)) == TileGroup::Enum::P && patchMode == PatchModeP::Enum::EOM))
        {
            EOMPatchDataUnit& epdu = pid.eomPatchDataUnit;
            epdu.epduFrameIndex = pid.frameIndex;
            epdu.epduPatchIndex = pid.patchIndex;
            
            eomPatchDataUnit(bitstream, context, epdu, atgh);
        }
    }

	size_t getNumRefIdxActive(ParserContext& context, AtlasTileGroupHeader& atgh)
    {
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        size_t numRefIdxActive = 0;
        
        if (TileGroup::Enum(atgh.atghType) == TileGroup::Enum::P || TileGroup::Enum(atgh.atghType) == TileGroup::Enum::SKIP)
        {
            if (atgh.atghNumRefIdxActiveOverrideFlag)
            {
                numRefIdxActive = atgh.atghNumRefIdxActiveMinus1 + 1;
            }
            else
            {
                RefListStruct& refList = atgh.refListStruct;
                numRefIdxActive = (size_t)Math::min((int) refList.numRefEntries, (int)afps.afpsNumRefIdxDefaultActiveMinus1 + 1);
            }
        }
        
        return numRefIdxActive;
    }

    void patchDataUnit(Bitstream& bitstream, ParserContext& context, PatchDataUnit& pdu, AtlasTileGroupHeader& atgh)
    {
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        size_t aspsId = afps.afpsAtlasSequenceParameterSetId;
        AtlasSequenceParameterSetRBSP& asps = context.atlasSequenceParameterSet[aspsId];

        pdu.pdu2dPosX = BitstreamReader::readBits(bitstream, afps.afps2dPosXBitCountMinus1 + 1);
        pdu.pdu2dPosY = BitstreamReader::readBits(bitstream, afps.afps2dPosYBitCountMinus1 + 1);

		pdu.pdu2dDeltaSizeX = BitstreamReader::readSVLC(bitstream);
        pdu.pdu2dDeltaSizeY = BitstreamReader::readSVLC(bitstream);

		pdu.pdu3dPosX = BitstreamReader::readBits(bitstream, afps.afps3dPosXBitCountMinus1 + 1);
        pdu.pdu3dPosY = BitstreamReader::readBits(bitstream, afps.afps3dPosYBitCountMinus1 + 1);

        const uint8_t bitCountForMinDepth = context.getActiveVps().geometryInformation[0].geometry3dCoordinatesBitdepthMinus1 - atgh.atghPosMinZQuantizer + (pdu.pduProjectionId > 5 ? 2 : 1);

		pdu.pdu3dPosMinZ = BitstreamReader::readBits(bitstream, bitCountForMinDepth);
        
        if (asps.normalAxisMaxDeltaValueEnabledFlag)
        {
            uint8_t bitCountForMaxDepth = context.getActiveVps().geometryInformation[0].geometry3dCoordinatesBitdepthMinus1 - atgh.atghPosDeltaMaxZQuantizer + (pdu.pduProjectionId > 5 ? 2 : 1);
            
            if (asps.degree45ProjectionPatchPresentFlag)
            {
                bitCountForMaxDepth++;
            }
            
            pdu.pdu3dPosDeltaMaxZ = BitstreamReader::readBits(bitstream, bitCountForMaxDepth);
        }
        
        pdu.pduProjectionId = BitstreamReader::readBits(bitstream, asps.degree45ProjectionPatchPresentFlag ? 5 : 3);
        pdu.pduOrientationIndex = BitstreamReader::readBits(bitstream, asps.useEightOrientationsFlag ? 3 : 1);
        
        if (afps.afpsLodModeEnableFlag)
        {
            pdu.pduLodEnableFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (pdu.pduLodEnableFlag)
            {
                pdu.pduLodScaleXminus1 = (uint8_t(BitstreamReader::readUVLC(bitstream)));
                pdu.pduLodScaleY = uint8_t(BitstreamReader::readUVLC(bitstream));
            }
        }
        else
        {
            pdu.pduLodEnableFlag = 0;
            pdu.pduLodScaleXminus1 = 0;
            pdu.pduLodScaleY = 0;
        }

        if (asps.pointLocalReconstructionEnabledFlag)
        {
            PointLocalReconstructionData& plrd = pdu.pointLocalReconstructionData;
            plrd.allocate(prevPatchSizeU + pdu.pdu2dDeltaSizeX, prevPatchSizeV + pdu.pdu2dDeltaSizeY);
            
            pointLocalReconstructionData(bitstream, context, plrd, asps);
            prevPatchSizeU += pdu.pdu2dDeltaSizeX;
            prevPatchSizeV += pdu.pdu2dDeltaSizeY;
        }
    }

    void mergePatchDataUnit(Bitstream& bitstream, ParserContext& context, MergePatchDataUnit& mpdu, AtlasTileGroupHeader& atgh)
    {
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        size_t aspsId = afps.afpsAtlasSequenceParameterSetId;
        AtlasSequenceParameterSetRBSP& asps = context.atlasSequenceParameterSet[aspsId];
        
        bool overridePlrFlag = false;
        size_t numRefIdxActive = getNumRefIdxActive(context, atgh);
        
        if (numRefIdxActive > 1)
        {
            mpdu.mpduRefIndex = BitstreamReader::readUVLC(bitstream);
        }
        else
        {
            mpdu.mpduRefIndex = 0;
        }
        
        mpdu.mpduOverride2dParamsFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (mpdu.mpduOverride2dParamsFlag)
        {
            mpdu.mpdu2dPosX = BitstreamReader::readSVLC(bitstream);
            mpdu.mpdu2dPosY = BitstreamReader::readSVLC(bitstream);
            mpdu.mpdu2dDeltaSizeX = BitstreamReader::readSVLC(bitstream);
            mpdu.mpdu2dDeltaSizeY = BitstreamReader::readSVLC(bitstream);
            
            if (asps.pointLocalReconstructionEnabledFlag)
            {
                overridePlrFlag = true;
            }
        }
        else
        {
            mpdu.mpduOverride3dParamsFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (mpdu.mpduOverride3dParamsFlag)
            {
                mpdu.mpdu3dPosX = BitstreamReader::readSVLC(bitstream);
                mpdu.mpdu3dPosY = BitstreamReader::readSVLC(bitstream);
                mpdu.mpdu3dPosMinZ = BitstreamReader::readSVLC(bitstream);
                
                if (asps.normalAxisMaxDeltaValueEnabledFlag)
                {
                    mpdu.mpdu3dPosDeltaMaxZ = BitstreamReader::readSVLC(bitstream);
                }
                
                if (asps.pointLocalReconstructionEnabledFlag)
                {
                    overridePlrFlag = BitstreamReader::readBits(bitstream, 1);
                    mpdu.mpduOverridePlrFlag = overridePlrFlag;
                }
            }
        }
        
        if (overridePlrFlag && asps.pointLocalReconstructionEnabledFlag)
        {
            PointLocalReconstructionData& plrd = mpdu.pointLocalReconstructionData;
            plrd.allocate(prevPatchSizeU + mpdu.mpdu2dDeltaSizeX, prevPatchSizeV + mpdu.mpdu2dDeltaSizeY);
            
            pointLocalReconstructionData(bitstream, context, plrd, asps);
            
            prevPatchSizeU += mpdu.mpdu2dDeltaSizeX;
            prevPatchSizeV += mpdu.mpdu2dDeltaSizeY;
        }
    }

    void interPatchDataUnit(Bitstream& bitstream, ParserContext& context, InterPatchDataUnit& ipdu, AtlasTileGroupHeader& atgh)
    {
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        size_t aspsId = afps.afpsAtlasSequenceParameterSetId;
        AtlasSequenceParameterSetRBSP& asps = context.atlasSequenceParameterSet[aspsId];
        
        size_t numRefIdxActive = getNumRefIdxActive(context, atgh);
        
        if (numRefIdxActive > 1)
        {
            ipdu.ipduRefIndex = BitstreamReader::readUVLC(bitstream);
        }
        else
        {
            ipdu.ipduRefIndex = 0;
        }
        
        ipdu.ipduRefPatchIndex = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu2dPosX = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu2dPosY = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu2dDeltaSizeX = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu2dDeltaSizeY = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu3dPosX = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu3dPosY = BitstreamReader::readSVLC(bitstream);
        ipdu.ipdu3dPosMinZ = BitstreamReader::readSVLC(bitstream);
        
        if (asps.normalAxisMaxDeltaValueEnabledFlag)
        {
            ipdu.ipdu3dPosDeltaMaxZ = BitstreamReader::readSVLC(bitstream);
        }
        
        if (asps.pointLocalReconstructionEnabledFlag)
        {
            AtlasTileGroupLayerRbsp& atglPrev = context.atlasTileGroupLayer[prevFrameIndex];
            AtlasTileGroupHeader& atghPrev = atglPrev.atlasTileGroupHeader;
            AtlasTileGroupDataUnit& atgdPrev = atglPrev.atlasTileGroupDataUnit;
            PatchInformationData& pidPrev = atgdPrev.patchInformationData[ipdu.ipduRefPatchIndex + predPatchIndex];
            
            uint8_t patchModePrev = pidPrev.patchMode;
            
            int64_t sizeU = ipdu.ipdu2dDeltaSizeX;
            int64_t sizeV = ipdu.ipdu2dDeltaSizeY;
            
            if ((TileGroup::Enum(atghPrev.atghType)) == TileGroup::Enum::P && patchModePrev == PatchModeP::Enum::SKIP)
            {
            }
            else if ((TileGroup::Enum(atghPrev.atghType)) == TileGroup::Enum::P && patchModePrev == PatchModeP::MERGE)
            {
                PointLocalReconstructionData& plrdPrev = pidPrev.mergePatchDataUnit.pointLocalReconstructionData;
                
                sizeU += plrdPrev.blockToPatchMapWidth;
                sizeV += plrdPrev.blockToPatchMapHeight;
            }
            else if ((TileGroup::Enum(atghPrev.atghType)) == TileGroup::Enum::P && patchModePrev == PatchModeP::INTER)
            {
                PointLocalReconstructionData& plrdPrev = pidPrev.interPatchDataUnit.pointLocalReconstructionData;
                
                sizeU += plrdPrev.blockToPatchMapWidth;
                sizeV += plrdPrev.blockToPatchMapHeight;
            }
            else if (((TileGroup::Enum(atghPrev.atghType)) == TileGroup::Enum::I && patchModePrev == PatchModeI::INTRA) ||
                     ((TileGroup::Enum(atghPrev.atghType)) == TileGroup::Enum::P && patchModePrev == PatchModeP::INTRA))
            {
                PointLocalReconstructionData& plrdPrev = pidPrev.patchDataUnit.pointLocalReconstructionData;
                
                sizeU += plrdPrev.blockToPatchMapWidth;
                sizeV += plrdPrev.blockToPatchMapHeight;
            }
            
            auto& plrd = ipdu.pointLocalReconstructionData;
            plrd.allocate(sizeU, sizeV);
            
            pointLocalReconstructionData(bitstream, context, plrd, asps);
            
            prevPatchSizeU = sizeU;
            prevPatchSizeV = sizeV;
            
            predPatchIndex += ipdu.ipduRefPatchIndex + 1;
        }
    }

    void rawPatchDataUnit(Bitstream& bitstream, ParserContext& context, RawPatchDataUnit& ppdu, AtlasTileGroupHeader& atgh)
    {
        VpccParameterSet& vps = context.getActiveVps();
        
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        size_t atlasIndex = 0;
        
        if (vps.rawSeparateVideoPresentFlag[atlasIndex])
        {
            ppdu.rpduPatchInRawVideoFlag = BitstreamReader::readBits(bitstream, 1);
        }
        
        ppdu.rpdu2dPosX = BitstreamReader::readBits(bitstream, afps.afps2dPosXBitCountMinus1 + 1);
        ppdu.rpdu2dPosY = BitstreamReader::readBits(bitstream, afps.afps2dPosYBitCountMinus1 + 1);
        ppdu.rpdu2dDeltaSizeX = BitstreamReader::readSVLC(bitstream);
        ppdu.rpdu2dDeltaSizeY = BitstreamReader::readSVLC(bitstream);
        ppdu.rpdu3dPosX = BitstreamReader::readBits(bitstream, atgh.atghRaw3dPosAxisBitCountMinus1 + 1);
        ppdu.rpdu3dPosY = BitstreamReader::readBits(bitstream, atgh.atghRaw3dPosAxisBitCountMinus1 + 1);
        ppdu.rpdu3dPosZ = BitstreamReader::readBits(bitstream, atgh.atghRaw3dPosAxisBitCountMinus1 + 1);
        ppdu.rpduRawPoints = BitstreamReader::readSVLC(bitstream);
    }

    void eomPatchDataUnit(Bitstream& bitstream, ParserContext& context, EOMPatchDataUnit& epdu, AtlasTileGroupHeader& atgh)
    {
        size_t afpsId = atgh.atghAtlasFrameParameterSetId;
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet[afpsId];
        
        epdu.epdu2dPosX = BitstreamReader::readBits(bitstream, afps.afps2dPosXBitCountMinus1 + 1);
        epdu.epdu2dPosY = BitstreamReader::readBits(bitstream, afps.afps2dPosYBitCountMinus1 + 1);
        
        epdu.epdu2dDeltaSizeX = BitstreamReader::readSVLC(bitstream);
        epdu.epdu2dDeltaSizeY = BitstreamReader::readSVLC(bitstream);

		epdu.epduAssociatedPatcheCountMinus1 = BitstreamReader::readBits(bitstream, 8);
        
        epdu.epduAssociatedPatches.resize(epdu.epduAssociatedPatcheCountMinus1 + 1);
        epdu.epduEomPointsPerPatch.resize(epdu.epduAssociatedPatcheCountMinus1 + 1);
        
		for (size_t cnt = 0; cnt < epdu.epduAssociatedPatcheCountMinus1 + 1; cnt++)
        {
            size_t pos = BitstreamReader::readBits(bitstream, 8);
            epdu.epduAssociatedPatches[pos] = cnt;
            
            pos = BitstreamReader::readUVLC(bitstream);
            epdu.epduEomPointsPerPatch[pos] = cnt;
        }
    }
	
    void seiRbsp(Bitstream& bitstream, ParserContext& context, NalUnitType::Enum nalUnitType)
    {
        seiMessage(bitstream, context, nalUnitType);
    }

    void seiMessage(Bitstream& bitstream, ParserContext& context, NalUnitType::Enum nalUnitType)
    {
        int32_t payloadType = 0;
        int32_t payloadSize = 0;
        
        int32_t byte = 0;
        
        do
        {
            byte = BitstreamReader::readBits(bitstream, 8);
            payloadType += byte;
        } while (byte == 0xff);
        
        do
        {
            byte = BitstreamReader::readBits(bitstream, 8);
            payloadSize += byte;
        } while (byte == 0xff);
        
        seiPayload(bitstream, context, nalUnitType, (SeiPayloadType::Enum)payloadType, payloadSize);
    }

    void seiPayload(Bitstream& bitstream, ParserContext& context, NalUnitType::Enum nalUnitType, SeiPayloadType::Enum payloadType, size_t payloadSize)
    {
        SEI& sei = addSei(context, nalUnitType, payloadType);
        
        if (nalUnitType == NalUnitType::Enum::PREFIX_SEI)
        {
            if (payloadType == 0)
            {
                bool NalHrdBpPresentFlag = false;
                bool AclHrdBpPresentFlag = false;
                
                std::vector<uint8_t> hrdCabCntMinus1;
                
                bufferingPeriod(bitstream, sei, payloadSize, NalHrdBpPresentFlag, AclHrdBpPresentFlag, hrdCabCntMinus1);
            }
            else if (payloadType == 1)
            {
                atlasFrameTiming(bitstream, sei, payloadSize, false);
            }
            else if (payloadType == 2)
            {
                fillerPayload(bitstream, sei, payloadSize);
            }
            else if (payloadType == 3)
            {
                userDataRegisteredItuTT35(bitstream, sei, payloadSize);
            }
            else if (payloadType == 4)
            {
                userDataUnregistered(bitstream, sei, payloadSize);
            }
            else if (payloadType == 5)
            {
                recoveryPoint(bitstream, sei, payloadSize);
            }
            else if (payloadType == 6)
            {
                noDisplay(bitstream, sei, payloadSize);
            }
            else if (payloadType == 7)
            {
                // timeCode( bitstream, sei, payloadSize );
            }
            else if (payloadType == 8)
            {
                // regionalNesting( bitstream, sei, payloadSize );
            }
            else if (payloadType == 9)
            {
                seiManifest(bitstream, sei, payloadSize);
            }
            else if (payloadType == 10)
            {
                seiPrefixIndication(bitstream, sei, payloadSize);
            }
            else if (payloadType == 11)
            {
                geometryTransformationParams(bitstream, sei, payloadSize);
            }
            else if (payloadType == 12)
            {
                attributeTransformationParams(bitstream, sei, payloadSize);
            }
            else if (payloadType == 13)
            {
                activeSubstreams(bitstream, sei, payloadSize);
            }
            else if (payloadType == 14)
            {
                componentCodecMapping(bitstream, sei, payloadSize);
            }
            else if (payloadType == 15)
            {
                volumetricTilingInfo(bitstream, sei, payloadSize);
            }
            else if (payloadType == 16)
            {
                presentationInformation(bitstream, sei, payloadSize);
            }
            else if (payloadType == 17)
            {
                smoothingParameters(bitstream, sei, payloadSize);
            }
            else
            {
                reservedSeiMessage(bitstream, sei, payloadSize);
            }
        }
        else
        {
            if (payloadType == 2)
            {
                fillerPayload(bitstream, sei, payloadSize);
            }
            else if (payloadType == 3)
            {
                userDataRegisteredItuTT35(bitstream, sei, payloadSize);
            }
            else if (payloadType == 4)
            {
                userDataUnregistered(bitstream, sei, payloadSize);
            }
            else
            {
                reservedSeiMessage(bitstream, sei, payloadSize);
            }
        }

        byteAlignment(bitstream);
    }

    void bufferingPeriod(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize, bool NalHrdBpPresentFlag, bool AclHrdBpPresentFlag, std::vector<uint8_t> hrdCabCntMinus1)
    {
        const int32_t fixedBitcount = 16;
        
        SEIBufferingPeriod& sei = static_cast<SEIBufferingPeriod&>(seiAbstract);
        sei.bpAtlasSequenceParameterSetId = BitstreamReader::readUVLC(bitstream);
        sei.bpIrapCabParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (sei.bpIrapCabParamsPresentFlag)
        {
            sei.bpCabDelayOffset = BitstreamReader::readBits(bitstream, fixedBitcount);
            sei.bpDabDelayOffset = BitstreamReader::readBits(bitstream, fixedBitcount);
        }
        
        sei.bpConcatenationFlag = BitstreamReader::readBits(bitstream, 1);
        sei.bpAtlasCabRemovalDelayDeltaMinus1 = BitstreamReader::readBits(bitstream, fixedBitcount);
        sei.bpMaxSubLayersMinus1 = BitstreamReader::readBits(bitstream, 3);
        
        sei.allocate();
        
        for (size_t i = 0; i <= sei.bpMaxSubLayersMinus1; i++)
        {
            if (NalHrdBpPresentFlag)
            {
                for (size_t j = 0; j < hrdCabCntMinus1[i] + 1; j++)
                {
                    sei.bpNalInitialCabRemovalDelay[i][j] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    sei.bpNalInitialCabRemovalOffset[i][j] = BitstreamReader::readBits(bitstream, fixedBitcount);
                }
                
                if (sei.bpIrapCabParamsPresentFlag)
                {
                    sei.bpNalInitialAltCabRemovalDelay[i] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    sei.bpNalInitialAltCabRemovalOffset[i] = BitstreamReader::readBits(bitstream, fixedBitcount);
                }
            }
            
            if (AclHrdBpPresentFlag)
            {
                for (size_t j = 0; j < hrdCabCntMinus1[i] + 1; j++)
                {
                    sei.bpAclInitialCabRemovalDelay[i][j] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    sei.bpAclInitialCabRemovalOffset[i][j] = BitstreamReader::readBits(bitstream, fixedBitcount);
                }
                
                if (sei.bpIrapCabParamsPresentFlag)
                {
                    sei.bpAclInitialAltCabRemovalDelay[i] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    sei.bpAclInitialAltCabRemovalOffset[i] = BitstreamReader::readBits(bitstream, fixedBitcount);
                }
            }
        }
    }

    void atlasFrameTiming(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize, bool CabDabDelaysPresentFlag)
    {
        const int32_t fixedBitcount = 16;
        
        SEIAtlasFrameTiming& sei = static_cast<SEIAtlasFrameTiming&>(seiAbstract);
        
        if (CabDabDelaysPresentFlag)
        {
            sei.aftCabRemovalDelayMinus1 = BitstreamReader::readBits(bitstream, fixedBitcount);
            sei.aftDabOutputDelay = BitstreamReader::readBits(bitstream, fixedBitcount);
        }
    }

    void presentationInformation(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIPresentationInformation& sei = static_cast<SEIPresentationInformation&>(seiAbstract);
        sei.piUnitOfLengthFlag = BitstreamReader::readBits(bitstream, 1);
        sei.piOrientationPresentFlag = BitstreamReader::readBits(bitstream, 1);
        sei.piPivotPresentFlag = BitstreamReader::readBits(bitstream, 1);
        sei.piDimensionPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (sei.piOrientationPresentFlag)
        {
            for (size_t d = 0; d < 3; d++)
            {
                sei.piUp[d]    = BitstreamReader::readBitsS(bitstream, 32);
                sei.piFront[d] = BitstreamReader::readBitsS(bitstream, 32);
            }
        }
        
        if (sei.piPivotPresentFlag)
        {
            for (size_t d = 0; d < 3; d++)
            {
                int64_t a = BitstreamReader::readBitsS(bitstream, 32);
                int64_t b = BitstreamReader::readBits(bitstream, 32);
                
                sei.piPivot[d] =  (a << 32) & b;
            }
        }
        
        if (sei.piDimensionPresentFlag)
        {
            for (size_t d = 0; d < 3; d++)
            {
                uint64_t a = BitstreamReader::readBitsS(bitstream, 32);
                uint64_t b = BitstreamReader::readBits(bitstream, 32);
                
                sei.piDimension[d] = (a << 32) & b;
            }
        }
    }

    void smoothingParameters(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEISmoothingParameters& sei = static_cast<SEISmoothingParameters&>(seiAbstract);
        sei.spGeometryCancelFlag = BitstreamReader::readBits(bitstream, 1);
        sei.spAttributeCancelFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (!sei.spGeometryCancelFlag)
        {
            sei.spGeometrySmoothingEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (sei.spGeometrySmoothingEnabledFlag == 1)
            {
                sei.spGeometrySmoothingId = BitstreamReader::readBits(bitstream, 8);
                
                if (sei.spGeometrySmoothingId == 0)
                {
                    sei.spGeometrySmoothingGridSizeMinus2 = BitstreamReader::readBits(bitstream, 7);
                    sei.spGeometrySmoothingThreshold = BitstreamReader::readBits(bitstream, 8);
                }
                else if (sei.spGeometrySmoothingId == 1)
                {
                    sei.spGeometryPatchBlockFilteringLog2ThresholdMinus1 = BitstreamReader::readBits(bitstream, 2);
                    sei.spGeometryPatchBlockFilteringPassesCountMinus1 = BitstreamReader::readBits(bitstream, 2);
                    sei.spGeometryPatchBlockFilteringFilterSizeMinus1 = BitstreamReader::readBits(bitstream, 3);
                }
            }
        }
        
        if (!sei.spAttributeCancelFlag)
        {
            sei.spNumAttributeUpdates = BitstreamReader::readUVLC(bitstream);
            
            sei.allocate();
            
            for (size_t j = 0; j < sei.spNumAttributeUpdates; j++)
            {
                sei.spAttributeIdx[j] = BitstreamReader::readBits(bitstream, 8);
                
                size_t index = sei.spAttributeIdx[j];
                size_t dimention = BitstreamReader::readBits(bitstream, 8);
                
                sei.allocate(index + 1, dimention + 1);
                
                sei.spDimensionMinus1[index] = dimention + 1;
                
                for (size_t i = 0; i < sei.spDimensionMinus1[index] + 1; i++)
                {
                    sei.spAttrSmoothingParamsEnabledFlag[index][i] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (sei.spAttrSmoothingParamsEnabledFlag[index][i])
                    {
                        sei.spAttrSmoothingGridSizeMinus2[index][i] = BitstreamReader::readBits(bitstream, 8);
                        sei.spAttrSmoothingThreshold[index][i] = BitstreamReader::readBits(bitstream, 8);
                        sei.spAttrSmoothingLocalEntropyThreshold[index][i] = BitstreamReader::readBits(bitstream, 8);
                        sei.spAttrSmoothingThresholdVariation[index][i] = BitstreamReader::readBits(bitstream, 8);
                        sei.spAttrSmoothingThresholdDifference[index][i] = BitstreamReader::readBits(bitstream, 8);
                    }
                }
            }
        }
    }
	
    void fillerPayload(Bitstream& bitstream, SEI& sei, size_t payloadSize)
    {
        for (size_t k = 0; k < payloadSize; k++)
        {
            BitstreamReader::readBits(bitstream, 8);
        }
    }

    void userDataRegisteredItuTT35(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIUserDataRegisteredItuTT35& sei = static_cast<SEIUserDataRegisteredItuTT35&>(seiAbstract);
        sei.ituTT35CountryCode = BitstreamReader::readBits(bitstream, 8);
        
        payloadSize--;
        
        if (sei.ituTT35CountryCode == 0xFF)
        {
            sei.ituTT35CountryCodeExtensionByte = BitstreamReader::readBits(bitstream, 8);
            payloadSize--;
        }
        
        std::vector<uint8_t>& payload = sei.ituTT35PayloadByte;
        payload.resize(payloadSize);
        
        for (auto& element : payload)
        {
            element = BitstreamReader::readBits(bitstream, 8);
        }
    }

    void userDataUnregistered(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIUserDataUnregistered& sei = static_cast<SEIUserDataUnregistered&>(seiAbstract);
        
        for (size_t i = 0; i < 16; i++)
        {
            sei.uuidIsoIec11578[i] = BitstreamReader::readBits(bitstream, 8);
        }
        
        payloadSize -= 16;
        
        sei.userDataPayloadByte.resize(payloadSize);
        
        for (size_t i = 0; i < payloadSize; i++)
        {
            sei.userDataPayloadByte[i] = BitstreamReader::readBits(bitstream, 8);
        }
    }

    void recoveryPoint(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIRecoveryPoint& sei = static_cast<SEIRecoveryPoint&>(seiAbstract);
        sei.recoveryAfocCnt = BitstreamReader::readSVLC(bitstream);
        sei.exactMatchFlag = BitstreamReader::readBits(bitstream, 1);
        sei.brokenLinkFlag = BitstreamReader::readBits(bitstream, 1);
    }

    void noDisplay(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
    }

    void reservedSeiMessage(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIReservedSeiMessage& sei = static_cast<SEIReservedSeiMessage&>(seiAbstract);
        sei.reservedSeiMessagePayloadByte.resize(payloadSize);
        
        for (size_t i = 0; i < payloadSize; i++)
        {
            sei.reservedSeiMessagePayloadByte[i] = BitstreamReader::readBits(bitstream, 8);
        }
    }

    void seiManifest(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIManifest& sei = static_cast<SEIManifest&>(seiAbstract);
        sei.manifestNumSeiMsgTypes = BitstreamReader::readBits(bitstream, 16);
        
        sei.allocate();
        
        for (size_t i = 0; i < sei.manifestNumSeiMsgTypes; i++)
        {
            sei.manifestSeiPayloadType[i] = BitstreamReader::readBits(bitstream, 16);
            sei.manifestSeiDescription[i] = BitstreamReader::readBits(bitstream, 8);
        }
    }

    void seiPrefixIndication(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIPrefixIndication& sei = static_cast<SEIPrefixIndication&>(seiAbstract);
        sei.prefixSeiPayloadType = BitstreamReader::readBits(bitstream, 16);
        sei.numSeiPrefixIndicationsMinus1 = BitstreamReader::readBits(bitstream, 8);
        
        sei.numBitsInPrefixIndicationMinus1.resize(sei.numSeiPrefixIndicationsMinus1 + 1, 0);
        sei.seiPrefixDataBit.resize(sei.numSeiPrefixIndicationsMinus1 + 1);
        
        for (size_t i = 0; i <= sei.numSeiPrefixIndicationsMinus1; i++)
        {
            sei.numBitsInPrefixIndicationMinus1[i] = BitstreamReader::readBits(bitstream, 16);
            sei.seiPrefixDataBit[i].resize(sei.numBitsInPrefixIndicationMinus1[i], false);
            
            for (size_t j = 0; j <= sei.numBitsInPrefixIndicationMinus1[i]; j++)
            {
                sei.seiPrefixDataBit[i][j] = BitstreamReader::readBits(bitstream, 1);
            }
            
            while (!BitstreamReader::isAligned(bitstream))
            {
                BitstreamReader::readBits(bitstream, 1);
            }
        }
    }

    void geometryTransformationParams(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIGeometryTransformationParams& sei = static_cast<SEIGeometryTransformationParams&>(seiAbstract);
        sei.gtpCancelFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (!sei.gtpCancelFlag)
        {
            sei.gtpScaleEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            sei.gtpOffsetEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            sei.gtpRotationEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (sei.gtpScaleEnabledFlag)
            {
                for (size_t d = 0; d < 3; d++)
                {
                    sei.gtpGeometryScaleOnAxis[d] = BitstreamReader::readBits(bitstream, 32);
                }
            }
            
            if (sei.gtpOffsetEnabledFlag)
            {
                for (size_t d = 0; d < 3; d++)
                {
                    sei.gtpGeometryOffsetOnAxis[d] = BitstreamReader::readBitsS(bitstream, 32);
                }
            }
            
            if (sei.gtpRotationEnabledFlag)
            {
                sei.gtpRotationQx = BitstreamReader::readBitsS(bitstream, 16);
                sei.gtpRotationQy = BitstreamReader::readBitsS(bitstream, 16);
                sei.gtpRotationQz = BitstreamReader::readBitsS(bitstream, 16);
            }
        }
    }

    void attributeTransformationParams(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIAttributeTransformationParams& sei = static_cast<SEIAttributeTransformationParams&>(seiAbstract);
        sei.atpCancelFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (!sei.atpCancelFlag)
        {
            sei.atpNumAttributeUpdates = BitstreamReader::readUVLC(bitstream);
            
            sei.allocate();
            
            for (size_t j = 0; j < sei.atpNumAttributeUpdates; j++)
            {
                sei.atpAttributeIdx[j] = BitstreamReader::readBits(bitstream, 8);
                
                size_t index = sei.atpAttributeIdx[j];
                sei.atpDimensionMinus1[index] = BitstreamReader::readBits(bitstream, 8);
                
                sei.allocate(index);
                
                for (size_t i = 0; i < sei.atpDimensionMinus1[index]; i++)
                {
                    sei.atpScaleParamsEnabledFlag[index][i] = BitstreamReader::readBits(bitstream, 1);
                    sei.atpOffsetParamsEnabledFlag[index][i] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (sei.atpScaleParamsEnabledFlag[index][i])
                    {
                        sei.atpAttributeScale[index][i] = BitstreamReader::readBits(bitstream, 32);
                    }
                    
                    if (sei.atpOffsetParamsEnabledFlag[index][i])
                    {
                        sei.atpAttributeOffset[index][i] = BitstreamReader::readBitsS(bitstream, 32);
                    }
                }
            }
        }
    }

    void activeSubstreams(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIActiveSubstreams& sei = static_cast<SEIActiveSubstreams&>(seiAbstract);
        sei.activeAttributesChangesFlag = BitstreamReader::readBits(bitstream, 1);
        sei.activeMapsChangesFlag = BitstreamReader::readBits(bitstream, 1);
        sei.rawPointsSubstreamsActiveFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (sei.activeAttributesChangesFlag)
        {
            sei.allAttributesActiveFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (!sei.allAttributesActiveFlag)
            {
                sei.activeAttributeCountMinus1 = BitstreamReader::readBits(bitstream, 7);
                sei.activeAttributeIdx.resize(sei.activeAttributeCountMinus1 + 1, 0);
                
                for (size_t i = 0; i <= sei.activeAttributeCountMinus1; i++)
                {
                    sei.activeAttributeIdx[i] = BitstreamReader::readBits(bitstream, 7);
                }
            }
        }
        
        if (sei.activeMapsChangesFlag)
        {
            sei.allMapsActiveFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (!sei.allMapsActiveFlag)
            {
                sei.activeMapCountMinus1 = BitstreamReader::readBits(bitstream, 4);
                sei.activeMapIdx.resize(sei.activeMapCountMinus1 + 1, 0);
                
                for (size_t i = 0; i <= sei.activeMapCountMinus1; i++)
                {
                    sei.activeMapIdx[i] = BitstreamReader::readBits(bitstream, 4);
                }
            }
        }
    }

    void componentCodecMapping(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIComponentCodecMapping& sei = static_cast<SEIComponentCodecMapping&>(seiAbstract);
        sei.ccmCodecMappingsCountMinus1 = BitstreamReader::readBits(bitstream, 8);
        
        sei.allocate();
        
        for (size_t i = 0; i <= sei.ccmCodecMappingsCountMinus1; i++)
        {
            sei.ccmCodecId[i] = BitstreamReader::readBits(bitstream, 8);
            sei.ccmCodec4cc[sei.ccmCodecId[i]] = BitstreamReader::readString(bitstream);
        }
    }

	// E.2.14  Volumetric Tiling SEI message syntax
    // E.2.14.1  General
    void volumetricTilingInfo(Bitstream& bitstream, SEI& seiAbstract, size_t payloadSize)
    {
        SEIVolumetricTilingInfo& sei = static_cast<SEIVolumetricTilingInfo&>(seiAbstract);
        sei.vtiCancelFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (!sei.vtiCancelFlag)
        {
            sei.vtiObjectLabelPresentFlag = BitstreamReader::readBits(bitstream, 1);
            sei.vti3dBoundingBoxPresentFlag = BitstreamReader::readBits(bitstream, 1);
            sei.vtiObjectPriorityPresentFlag = BitstreamReader::readBits(bitstream, 1);
            sei.vtiObjectHiddenPresentFlag = BitstreamReader::readBits(bitstream, 1);
            sei.vtiObjectCollisionShapePresentFlag = BitstreamReader::readBits(bitstream, 1);
            sei.vtiObjectDependencyPresentFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (sei.vtiObjectLabelPresentFlag)
            {
                volumetricTilingInfoLabels(bitstream, sei);
            }
            
            if (sei.vti3dBoundingBoxPresentFlag)
            {
                sei.vtiBoundingBoxScaleLog2 = BitstreamReader::readBits(bitstream, 5);
                sei.vti3dBoundingBoxScaleLog2 = BitstreamReader::readBits(bitstream, 5);
                sei.vti3dBoundingBoxPrecisionMinus8 = BitstreamReader::readBits(bitstream, 1);
            }
            
            volumetricTilingInfoObjects(bitstream, sei);
        }
    }

    void volumetricTilingInfoLabels(Bitstream& bitstream, SEIVolumetricTilingInfo& sei)
    {
        VolumetricTilingInfoLabels& vtil = sei.volumetricTilingInfoLabels;
        vtil.vtiObjectLabelLanguagePresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (vtil.vtiObjectLabelLanguagePresentFlag)
        {
            while (!BitstreamReader::isAligned(bitstream))
            {
                uint32_t val = BitstreamReader::readBits(bitstream, 1);
                assert(val == 0);
            }
            
            vtil.vtiObjectLabelLanguage = BitstreamReader::readString(bitstream);
        }
        
        vtil.vtiNumObjectLabelUpdates = BitstreamReader::readUVLC(bitstream);
        
        vtil.allocate();
        
        for (size_t i = 0; i < vtil.vtiNumObjectLabelUpdates; i++)
        {
            vtil.vtiLabelIdx[i] = BitstreamReader::readUVLC(bitstream);
            
            bool cancelFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (!cancelFlag)
            {
                while (!BitstreamReader::isAligned(bitstream))
                {
                    uint32_t val = BitstreamReader::readBits(bitstream, 1);
                    assert(val == 0);
                }
                
                vtil.vtiLabel[vtil.vtiLabelIdx[i]] = BitstreamReader::readString(bitstream);
            }
        }
    }

    void volumetricTilingInfoObjects(Bitstream& bitstream, SEIVolumetricTilingInfo& sei)
    {
        const int32_t fixedBitcount = 16;
        
        VolumetricTilingInfoObjects& vtio = sei.volumetricTilingInfoObjects;
        vtio.vtiNumObjectUpdates = BitstreamReader::readUVLC(bitstream);
        
        vtio.allocate();
        
        for (size_t i = 0; i <= vtio.vtiNumObjectUpdates; i++)
        {
            vtio.vtiObjectIdx[i]= BitstreamReader::readUVLC(bitstream);
            
            size_t index = vtio.vtiObjectIdx[i];
            
            vtio.allocate(index + 1);
            
            vtio.vtiObjectCancelFlag[index] = BitstreamReader::readBits(bitstream, 1);

			if (!vtio.vtiObjectCancelFlag[index])
            {
                vtio.vtiBoundingBoxUpdateFlag[index] = BitstreamReader::readBits(bitstream, 1);
                
                if (vtio.vtiBoundingBoxUpdateFlag[index])
                {
                    vtio.vtiBoundingBoxTop[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    vtio.vtiBoundingBoxLeft[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    vtio.vtiBoundingBoxWidth[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    vtio.vtiBoundingBoxHeight[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                }
                
                if (sei.vti3dBoundingBoxPresentFlag)
                {
                    vtio.vti3dBoundingBoxUpdateFlag[index] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (vtio.vti3dBoundingBoxUpdateFlag[index])
                    {
                        vtio.vti3dBoundingBoxX[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                        vtio.vti3dBoundingBoxY[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                        vtio.vti3dBoundingBoxZ[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                        vtio.vti3dBoundingBoxDeltaX[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                        vtio.vti3dBoundingBoxDeltaY[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                        vtio.vti3dBoundingBoxDeltaZ[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    }
                }
                
                if (sei.vtiObjectPriorityPresentFlag)
                {
                    vtio.vtiObjectPriorityUpdateFlag[index] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (vtio.vtiObjectPriorityUpdateFlag[index])
                    {
                        vtio.vtiObjectPriorityValue[index] = BitstreamReader::readBits(bitstream, 4);
                    }
                }
                
                if (sei.vtiObjectHiddenPresentFlag)
                {
                    vtio.vtiObjectHiddenFlag[index] = BitstreamReader::readBits(bitstream, 1);
                }
                
                if (sei.vtiObjectLabelPresentFlag)
                {
                    vtio.vtiObjectLabelUpdateFlag[index] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (vtio.vtiObjectLabelUpdateFlag[index])
                    {
                        vtio.vtiObjectLabelIdx[index] = BitstreamReader::readBits(bitstream, fixedBitcount);
                    }
                }
                
                if (sei.vtiObjectCollisionShapePresentFlag)
                {
                    vtio.vtiObjectCollisionShapeUpdateFlag[index] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (vtio.vtiObjectCollisionShapeUpdateFlag[index])
                    {
                        vtio.vtiObjectCollisionShapeId[index] = BitstreamReader::readBits(bitstream, 16);
                    }
                }
                
                if (sei.vtiObjectDependencyPresentFlag)
                {
                    vtio.vtiObjectDependencyUpdateFlag[index] = BitstreamReader::readBits(bitstream, 1);
                    
                    if (vtio.vtiObjectDependencyUpdateFlag[index])
                    {
                        vtio.vtiObjectNumDependencies[index] = BitstreamReader::readBits(bitstream, 4);
                        
                        for (size_t j = 0; j < vtio.vtiObjectNumDependencies[index]; j++)
                        {
                            vtio.vtiObjectDependencyIdx[index][j] = BitstreamReader::readBits(bitstream, 8);
                        }
                    }
                }
            }
        }
    }

    void vuiParameters(Bitstream& bitstream, VUIParameters& vp)
    {
        vp.vuiTimingInfoPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (vp.vuiTimingInfoPresentFlag)
        {
            vp.vuiNumUnitsInTick = BitstreamReader::readBits(bitstream, 32);
            vp.vuiTimeScale = BitstreamReader::readBits(bitstream, 32);
            vp.vuiPocProportionalToTimingFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (vp.vuiPocProportionalToTimingFlag)
            {
                vp.vuiNumTicksPocDiffOneMinus1 = BitstreamReader::readUVLC(bitstream);
            }
            
            vp.vuiHrdParametersPresentFlag = BitstreamReader::readBits(bitstream, 1);
            
            if (vp.vuiHrdParametersPresentFlag)
            {
                hrdParameters(bitstream, vp.hrdParameters);
            }
        }
    }

    void sampleStreamVpccHeader(Bitstream& bitstream, uint32_t& ssvhUnitSizePrecisionBytesMinus1)
    {
        ssvhUnitSizePrecisionBytesMinus1 = BitstreamReader::readBits(bitstream, 3);
        
        BitstreamReader::readBits(bitstream, 5);
    }

    void hrdParameters(Bitstream& bitstream, HrdParameters& hp)
    {
        hp.hrdNalParametersPresentFlag = BitstreamReader::readBits(bitstream, 1);
        hp.hrdAclParametersPresentFlag = BitstreamReader::readBits(bitstream, 1);
        
        if (hp.hrdNalParametersPresentFlag || hp.hrdAclParametersPresentFlag)
        {
            hp.hrdBitRateScale = BitstreamReader::readBits(bitstream, 4);
            hp.hrdCabSizeScale = BitstreamReader::readBits(bitstream, 4);
            hp.hrdInitialCabRemovalDelayLengthMinus1 = BitstreamReader::readBits(bitstream, 5);
            hp.hrdAuCabRemovalDelayLengthMinus1 = BitstreamReader::readBits(bitstream, 5);
            hp.hrdDabOutputDelayLengthMinus1 = BitstreamReader::readBits(bitstream, 5);
        }
        
        for (size_t i = 0; i <= hp.maxNumSubLayersMinus1; i++)
        {
            hp.hrdFixedAtlasRateGeneralFlag[i] = BitstreamReader::readBits(bitstream, 1);
            
            if (!hp.hrdFixedAtlasRateGeneralFlag[i])
            {
                hp.hrdFixedAtlasRateWithinCasFlag[i] = BitstreamReader::readBits(bitstream, 1);
            }
            
            if (hp.hrdFixedAtlasRateWithinCasFlag[i])
            {
                hp.hrdFixedAtlasRateWithinCasFlag[i] = BitstreamReader::readBits(bitstream, 1);
            }
            else
            {
                hp.hrdLowDelayFlag[i] = BitstreamReader::readBits(bitstream, 1);
            }
            
            if (!hp.hrdLowDelayFlag[i])
            {
                hp.hrdLowDelayFlag[i] = BitstreamReader::readBits(bitstream, 1);
            }
            
            if (hp.hrdNalParametersPresentFlag)
            {
                hrdSubLayerParameters(bitstream, hp.hrdSubLayerParameters[0][i], hp.hrdLowDelayFlag[i]);
            }
            
            if (hp.hrdAclParametersPresentFlag)
            {
                hrdSubLayerParameters(bitstream, hp.hrdSubLayerParameters[1][i], hp.hrdLowDelayFlag[i]);
            }
        }
    }

    void hrdSubLayerParameters(Bitstream& bitstream, HrdSubLayerParameters& hlsp, size_t cabCnt)
    {
        hlsp.allocate(cabCnt + 1);
        
        for (size_t i = 0; i <= cabCnt; i++)
        {
            hlsp.hrdBitRateValueMinus1[i] = BitstreamReader::readUVLC(bitstream);
            hlsp.hrdCabSizeValueMinus1[i] = BitstreamReader::readUVLC(bitstream);
            hlsp.hrdCbrFlag[i] = BitstreamReader::readBits(bitstream, 1);
        }
    }

    // VPCC codec

PatchType::Enum getCurrPatchType(TileGroup::Enum tileGroupType, uint8_t patchMode);
void constructRefList(ParserContext& context, size_t aspsIdx, size_t afpsIdx);

    void createPatchFrameDataStructure(ParserContext& context, FrameGroup& frameGroup, FrameData& frame, size_t frameIndex);
    void createBlockToPatchFromBoundaryBox(ParserContext& context, FrameData& frame, size_t occupancyResolution);

    PatchType::Enum getCurrPatchType(TileGroup::Enum tileGroupType, uint8_t patchMode)
    {
        if (((tileGroupType == TileGroup::I) && patchMode == (uint8_t)PatchModeI::INTRA) ||
            ((tileGroupType == TileGroup::TileGroup::P) && patchMode == (uint8_t)PatchModeP::INTRA))
        {
            return PatchType::INTRA;
        }
        else if ((tileGroupType == TileGroup::P && patchMode == (uint8_t)PatchModeP::INTER))
        {
            return PatchType::INTER;
        }
        else if ((tileGroupType == TileGroup::I && patchMode == (uint8_t)PatchModeI::RAW) ||
                 (tileGroupType == TileGroup::P && patchMode == (uint8_t)PatchModeP::RAW))
        {
            return PatchType::RAW;
        }
        else if ((tileGroupType == TileGroup::I && patchMode == (uint8_t)PatchModeI::EOM) ||
                 (tileGroupType == TileGroup::P && patchMode == (uint8_t)PatchModeP::EOM))
        {
            return PatchType::EOM;
        }
        else if ((tileGroupType == TileGroup::P && patchMode == (uint8_t)PatchModeP::MERGE))
        {
            return PatchType::MERGE;
        }
        else if ((tileGroupType == TileGroup::P && patchMode == (uint8_t)PatchModeP::SKIP))
        {
            return PatchType::SKIP;
        }
        else if ((tileGroupType == TileGroup::I && patchMode == (uint8_t)PatchModeI::END) ||
                 (tileGroupType == TileGroup::P && patchMode == (uint8_t)PatchModeP::END))
        {
            return PatchType::END;
        }
        else
        {
            return PatchType::ERROR;
        }
    }

    void constructRefList(ParserContext& context, size_t aspsIdx, size_t afpsIdx)
    {
        AtlasSequenceParameterSetRBSP& asps = context.atlasSequenceParameterSet[aspsIdx];

        context.refAtlasFrameList.resize(asps.numRefAtlasFrameListsInAsps);

        for (size_t list = 0; list < context.refAtlasFrameList.size(); list++)
        {
            RefListStruct& refList = asps.refListStruct.at(list);

            size_t maxNumRefAtlasFrame = refList.numRefEntries;
            context.refAtlasFrameList[list].resize(maxNumRefAtlasFrame);
          
            for (size_t i = 0; i < refList.numRefEntries; i++)
            {
                int32_t absDiff = refList.absDeltaAfocSt.at(i);
                bool sign = refList.strpfEntrySignFlag.at(i);
                
                int32_t value = (sign == 0) ? -absDiff : absDiff;
                
                context.refAtlasFrameList.at(list).at(i) = value;
            }
        }
    }

    void setRefAFOCList(ParserContext& context, FrameData& frame)
    {
        size_t numOfAvailableRefAtlasFrameList = context.refAtlasFrameList.size();

        std::vector<std::vector<size_t>> refAFOCList;
        refAFOCList.resize(numOfAvailableRefAtlasFrameList);

        int32_t refPOC = 0;

        for (size_t i = 0; i < numOfAvailableRefAtlasFrameList; i++)
        {
            size_t maxRefNum = context.refAtlasFrameList.at(i).size();

            for (size_t j = 0; j < maxRefNum; j++)
            {
                refPOC = int32_t(frame.index) + int32_t(context.refAtlasFrameList.at(i).at(j));

                if (refPOC >= 0)
                {
                    refAFOCList[i].push_back(refPOC);
                }
            }

            if (refAFOCList[i].size() == 0)
            {
                refAFOCList[i].push_back(255);
            }
        }
        
        frame.refAFOCList = refAFOCList;
    }

    void createPatchFrameDataStructures(ParserContext& context, FrameGroup& frameGroup)
    {
        VpccParameterSet& sps = context.vpccParameterSets[0];
        std::vector<AtlasTileGroupLayerRbsp>& atglulist = context.atlasTileGroupLayer;

        size_t frameCount = atglulist.size();
        frameGroup.frames.resize(frameCount);
        
        constructRefList(context, 0, 0);
        
        size_t atlasIndex = 0;

        for (size_t i = 0; i < frameCount; i++)
        {
            FrameData& frame = frameGroup.frames[i];
            frame.afOrderCnt = i;
            frame.index = i;
            frame.width = sps.frameWidth.at(atlasIndex);
            frame.height = sps.frameHeight.at(atlasIndex);
            frame.rawPatchEnabledFlag = sps.rawPatchEnabledFlag.at(atlasIndex);
            
            if (i > 0)
            {
                setRefAFOCList( context, frame );
            }
            
            createPatchFrameDataStructure(context, frameGroup, frame, i);
            createBlockToPatchFromBoundaryBox(context, frame, frame.patches[0].occupancyResolution);
        }
    }

    void createPatchFrameDataStructure(ParserContext& context, FrameGroup& frameGroup, FrameData& frame, size_t frameIndex)
    {
        VpccParameterSet& sps = context.vpccParameterSets.at(0);

        size_t atlasIndex = 0;
        GeometryInformation& gi = sps.geometryInformation.at(atlasIndex);

        AtlasSequenceParameterSetRBSP& asps  = context.atlasSequenceParameterSet.at(0);
        AtlasFrameParameterSetRbsp& afps = context.atlasFrameParameterSet.at(0);
        AtlasTileGroupLayerRbsp& atglu = context.atlasTileGroupLayer.at(frameIndex);
        AtlasTileGroupHeader& atgh = atglu.atlasTileGroupHeader;
        AtlasTileGroupDataUnit& atgdu = atglu.atlasTileGroupDataUnit;

        std::vector<Patch>& patches = frame.patches;
        std::vector<MissedPointsPatch>& pcmPatches = frame.missedPointsPatches;
        std::vector<EomPatch>& eomPatches = frame.eomPatches;

        int64_t prevSizeU0 = 0;
        int64_t prevSizeV0 = 0;

        int64_t prevPatchSize2DXInPixel = 0;
        int64_t prevPatchSize2DYInPixel = 0;

        int64_t predIndex = 0;

        const size_t minLevel = sps.minLevel;

        size_t numRawPatches = 0;
        size_t numNonRawPatch = 0;
        size_t numEomPatch = 0;

        TileGroup::Enum tileGroupType = atgh.atghType;

        size_t patchCount = atgdu.patchInformationData.size();

        for (size_t i = 0; i < patchCount; i++)
        {
            PatchType::Enum currPatchType = getCurrPatchType(tileGroupType, atgdu.patchInformationData.at(i).patchMode);

            if (currPatchType == PatchType::RAW)
            {
                numRawPatches++;
            }
            else if (currPatchType== PatchType::EOM)
            {
                numEomPatch++;
            }
        }

        numNonRawPatch = patchCount - numRawPatches - numEomPatch;

        eomPatches.reserve(numEomPatch);
        patches.resize(numNonRawPatch);
        pcmPatches.resize(numRawPatches);
        
        size_t totalNumberOfMps = 0;
        size_t patchIndex = 0;

        uint32_t occupancyPackingBlockSize = ::pow(2, asps.log2PatchPackingBlockSize);

        int32_t quantizerSizeX = 1 << atgh.atghPatchSizeXinfoQuantizer;
        int32_t quantizerSizeY = 1 << atgh.atghPatchSizeYinfoQuantizer;

        for (patchIndex = 0; patchIndex < patchCount; patchIndex++)
        {
            PatchInformationData& pid = atgdu.patchInformationData.at(patchIndex);
            PatchType::Enum currPatchType = getCurrPatchType(tileGroupType, atgdu.patchInformationData.at(patchIndex).patchMode);

            if (currPatchType == PatchType::INTRA)
            {
                PatchDataUnit& pdu = pid.patchDataUnit;
                
                Patch& patch = patches.at(patchIndex);
                patch.occupancyResolution = occupancyPackingBlockSize;
                patch.u0 = pdu.pdu2dPosX;
                patch.v0 = pdu.pdu2dPosY;
                patch.u1 = pdu.pdu3dPosX;
                patch.v1 = pdu.pdu3dPosY;

                bool lodEnableFlag = pdu.pduLodEnableFlag;
                
                if (lodEnableFlag)
                {
                    patch.levelOfDetailX = pdu.pduLodScaleXminus1 + 1;
                    patch.levelOfDetailY = pdu.pduLodScaleY + (patch.levelOfDetailX > 1 ? 1 : 2);
                }
                else
                {
                    patch.levelOfDetailX = 1;
                    patch.levelOfDetailY = 1;
                }
                
                patch.sizeD = std::min(pdu.pdu3dPosDeltaMaxZ * minLevel, (size_t)255);
                
                if (asps.patchSizeQuantizerPresentFlag)
                {
                    int32_t quantizedDeltaSizeU = pdu.pdu2dDeltaSizeX;
                    int32_t quantizedDeltaSizeV = pdu.pdu2dDeltaSizeY;
                    
                    patch.size2DXInPixel = prevPatchSize2DXInPixel + quantizedDeltaSizeU * quantizerSizeX;
                    patch.size2DYInPixel = prevPatchSize2DYInPixel + quantizedDeltaSizeV * quantizerSizeY;
                    
                    patch.sizeU0 = ceil((double)patch.size2DXInPixel / (double)occupancyPackingBlockSize);
                    patch.sizeV0 = ceil((double)patch.size2DYInPixel / (double)occupancyPackingBlockSize);
                }
                else
                {
                    patch.sizeU0 = prevSizeU0 + pdu.pdu2dDeltaSizeX;
                    patch.sizeV0 = prevSizeV0 + pdu.pdu2dDeltaSizeY;
                }
                
                size_t pduProjectionPlane = asps.degree45ProjectionPatchPresentFlag ? (pdu.pduProjectionId >> 2) : pdu.pduProjectionId;
                size_t pdu45degreeProjectionRotationAxis = asps.degree45ProjectionPatchPresentFlag ? (pdu.pduProjectionId & 0x03) : 0;

                patch.normalAxis = pduProjectionPlane % 3;
                patch.projectionMode = pduProjectionPlane < 3 ? 0 : 1;
                patch.patchOrientation = pdu.pduOrientationIndex;
                patch.axisOfAdditionalPlane = pdu45degreeProjectionRotationAxis;
                
                const size_t max3DCoordinate = 1 << (gi.geometry3dCoordinatesBitdepthMinus1 + 1);
                
                if (patch.projectionMode == 0)
                {
                    patch.d1 = (int32_t)pdu.pdu3dPosMinZ * minLevel;
                }
                else
                {
                    if (asps.degree45ProjectionPatchPresentFlag == 0)
                    {
                        patch.d1 = max3DCoordinate - (int32_t)pdu.pdu3dPosMinZ * minLevel;
                    }
                    else
                    {
                        patch.d1 = (max3DCoordinate << 1) - (int32_t)pdu.pdu3dPosMinZ * minLevel;
                    }
                }
                
                prevSizeU0 = patch.sizeU0;
                prevSizeV0 = patch.sizeV0;
                
                prevPatchSize2DXInPixel = patch.size2DXInPixel;
                prevPatchSize2DYInPixel = patch.size2DYInPixel;
                
                if (patch.normalAxis == 0)
                {
                    patch.tangentAxis = 2;
                    patch.bitangentAxis = 1;
                }
                else if (patch.normalAxis == 1)
                {
                    patch.tangentAxis = 2;
                    patch.bitangentAxis = 0;
                }
                else
                {
                    patch.tangentAxis = 0;
                    patch.bitangentAxis = 1;
                }

                /*
                patch.allocOneLayerData();

                if (asps.pointLocalReconstructionEnabledFlag)
                {
                    setPointLocalReconstructionData(frame, patch, pdu.pointLocalReconstructionData, occupancyPackingBlockSize);
                }
                */
            }
            else if (currPatchType == PatchType::INTER)
            {
                InterPatchDataUnit& ipdu = pid.interPatchDataUnit;
                
                Patch& patch = patches.at(patchIndex);
                patch.occupancyResolution = occupancyPackingBlockSize;
                patch.bestMatchIndex = (int32_t)(ipdu.ipduRefPatchIndex + predIndex);
                
                predIndex += ipdu.ipduRefPatchIndex + 1;
                
                patch.refAtlasFrameIdx = ipdu.ipduRefIndex;
                
                size_t refPOC = frame.refAFOCList.at(0).at(patch.refAtlasFrameIdx);
                const Patch& refPatch = frameGroup.frames.at(refPOC).patches.at(patch.bestMatchIndex); // TODO

                patch.projectionMode = refPatch.projectionMode;
                patch.u0 = ipdu.ipdu2dPosX + refPatch.u0;
                patch.v0 = ipdu.ipdu2dPosY + refPatch.v0;
                patch.patchOrientation = refPatch.patchOrientation;
                patch.u1 = ipdu.ipdu3dPosX + refPatch.u1;
                patch.v1 = ipdu.ipdu3dPosY + refPatch.v1;
                
                if (asps.patchSizeQuantizerPresentFlag)
                {
                    patch.size2DXInPixel = refPatch.size2DXInPixel + ipdu.ipdu2dDeltaSizeX * quantizerSizeX;
                    patch.size2DYInPixel = refPatch.size2DYInPixel + ipdu.ipdu2dDeltaSizeY * quantizerSizeY;
                    patch.sizeU0 = ceil((double)patch.size2DXInPixel / (double)occupancyPackingBlockSize);
                    patch.sizeV0 = ceil((double)patch.size2DYInPixel / (double)occupancyPackingBlockSize);
                }
                else
                {
                    patch.sizeU0 = ipdu.ipdu2dDeltaSizeX + refPatch.sizeU0;
                    patch.sizeV0 = ipdu.ipdu2dDeltaSizeY + refPatch.sizeV0;
                }
                
                patch.normalAxis = refPatch.normalAxis;
                patch.tangentAxis = refPatch.tangentAxis;
                patch.bitangentAxis = refPatch.bitangentAxis;
                patch.axisOfAdditionalPlane = refPatch.axisOfAdditionalPlane;
                    
                const size_t max3DCoordinate = 1 << (gi.geometry3dCoordinatesBitdepthMinus1 + 1);
                
                if (patch.projectionMode == 0)
                {
                    patch.d1 = (ipdu.ipdu3dPosMinZ + (refPatch.d1 / minLevel)) * minLevel;
                }
                else
                {
                    if (asps.degree45ProjectionPatchPresentFlag == 0)
                    {
                        patch.d1 = max3DCoordinate - (ipdu.ipdu3dPosMinZ + ((max3DCoordinate - refPatch.d1) / minLevel)) * minLevel;
                    }
                    else
                    {
                        patch.d1 = (max3DCoordinate << 1 ) - (ipdu.ipdu3dPosMinZ + (((max3DCoordinate << 1) - refPatch.d1) / minLevel)) * minLevel;
                    }
                }
                
                const int64_t delta_DD = ipdu.ipdu3dPosDeltaMaxZ;
                size_t prevDD = refPatch.sizeD / minLevel;
                
                if (prevDD * minLevel != refPatch.sizeD)
                {
                    prevDD += 1;
                }
                
                patch.sizeD = std::min(size_t((delta_DD + prevDD ) * minLevel), (size_t)255);
                patch.levelOfDetailX = refPatch.levelOfDetailX;
                patch.levelOfDetailY = refPatch.levelOfDetailY;
                prevSizeU0 = patch.sizeU0;
                prevSizeV0 = patch.sizeV0;
                
                prevPatchSize2DXInPixel = patch.size2DXInPixel;
                prevPatchSize2DYInPixel = patch.size2DYInPixel;

                /*
                patch.allocOneLayerData(); //do we need this?
                
                if (asps.pointLocalReconstructionEnabledFlag)
                {
                    setPointLocalReconstructionData(frame, patch, ipdu.pointLocalReconstructionData, occupancyPackingBlockSize);
                }
                */
            }
            else if (currPatchType == PatchType::MERGE)
            {
                assert(-2);
                
                Patch& patch = patches.at(patchIndex);
                patch.occupancyResolution = occupancyPackingBlockSize;
                
                MergePatchDataUnit& mpdu = pid.mergePatchDataUnit;
                
                bool overridePlrFlag = false;
                const size_t max3DCoordinate = 1 << (gi.geometry3dCoordinatesBitdepthMinus1 + 1);

                patch.bestMatchIndex = patchIndex;
                    
                predIndex = patchIndex;
                
                patch.refAtlasFrameIdx = mpdu.mpduRefIndex;
                
                size_t refPOC = frame.refAFOCList.at(0).at(patch.refAtlasFrameIdx);
                const Patch& refPatch = frameGroup.frames.at(refPOC).patches.at(patch.bestMatchIndex);
          
                if (mpdu.mpduOverride2dParamsFlag)
                {
                    patch.u0 = mpdu.mpdu2dPosX + refPatch.u0;
                    patch.v0 = mpdu.mpdu2dPosY + refPatch.v0;
                    
                    if (asps.patchSizeQuantizerPresentFlag)
                    {
                        patch.size2DXInPixel = refPatch.size2DXInPixel + (mpdu.mpdu2dDeltaSizeX * quantizerSizeX);
                        patch.size2DYInPixel = refPatch.size2DYInPixel + (mpdu.mpdu2dDeltaSizeY * quantizerSizeY);

                        patch.sizeU0 = ceil( (double)patch.size2DXInPixel / (double)occupancyPackingBlockSize);
                        patch.sizeV0 = ceil( (double)patch.size2DYInPixel / (double)occupancyPackingBlockSize);
                    }
                    else
                    {
                        patch.sizeU0 = mpdu.mpdu2dDeltaSizeX + refPatch.sizeU0;
                        patch.sizeV0 = mpdu.mpdu2dDeltaSizeY + refPatch.sizeV0;
                    }

                    if (asps.pointLocalReconstructionEnabledFlag)
                    {
                        overridePlrFlag = true;
                    }
                }
                else
                {
                    if (mpdu.mpduOverride3dParamsFlag)
                    {
                        patch.u1 = mpdu.mpdu3dPosX + refPatch.u1;
                        patch.v1 = mpdu.mpdu3dPosY + refPatch.v1;
                        
                        if (patch.projectionMode == 0)
                        {
                            patch.d1 = (mpdu.mpdu3dPosMinZ + (refPatch.d1 / minLevel)) * minLevel;
                        }
                        else
                        {
                            if (asps.degree45ProjectionPatchPresentFlag == 0)
                            {
                                patch.d1 = max3DCoordinate - (mpdu.mpdu3dPosMinZ + ((max3DCoordinate - refPatch.d1) / minLevel)) * minLevel;
                            }
                            else
                            {
                                patch.d1 = (max3DCoordinate << 1 ) - (mpdu.mpdu3dPosMinZ + (((max3DCoordinate << 1 ) - refPatch.d1) / minLevel)) * minLevel;
                            }
                        }
              
                        const int64_t delta_DD = mpdu.mpdu3dPosDeltaMaxZ;
                        size_t prevDD = refPatch.sizeD / minLevel;
                        
                        if (prevDD * minLevel != refPatch.sizeD)
                        {
                            prevDD += 1;
                        }
                        
                        patch.sizeD = std::min(size_t((delta_DD + prevDD) * minLevel), (size_t)255);

                        if (asps.pointLocalReconstructionEnabledFlag)
                        {
                            overridePlrFlag = mpdu.mpduOverridePlrFlag;
                        }
                    }
                }

                patch.projectionMode = refPatch.projectionMode;
                patch.patchOrientation = refPatch.patchOrientation;

                patch.normalAxis = refPatch.normalAxis;
                patch.tangentAxis = refPatch.tangentAxis;
                patch.bitangentAxis = refPatch.bitangentAxis;
                patch.axisOfAdditionalPlane = refPatch.axisOfAdditionalPlane;
          
                patch.levelOfDetailX = refPatch.levelOfDetailX;
                patch.levelOfDetailY = refPatch.levelOfDetailY;
                prevSizeU0 = patch.sizeU0;
                prevSizeV0 = patch.sizeV0;
                prevPatchSize2DXInPixel = patch.size2DXInPixel;
                prevPatchSize2DYInPixel = patch.size2DYInPixel;

                /*
                patch.allocOneLayerData(); //do we need this?
                
                if (asps.pointLocalReconstructionEnabledFlag)
                {
                    setPointLocalReconstructionData(frame, patch, mpdu.pointLocalReconstructionData, context.occupancyPackingBlockSize);
                }
                */
            }
            else if (currPatchType == PatchType::SKIP)
            {
                assert(-1);
                
                Patch& patch = patches.at(patchIndex);
                patch.bestMatchIndex = (int32_t)patchIndex;
                
                predIndex += patchIndex;
                
                patch.refAtlasFrameIdx = 0;
                
                size_t refPOC = frame.refAFOCList.at(0).at(patch.refAtlasFrameIdx);
                const Patch& refPatch = frameGroup.frames.at(refPOC).patches.at(patch.bestMatchIndex);
          
                patch.projectionMode = refPatch.projectionMode;
                patch.u0 = refPatch.u0;
                patch.v0 = refPatch.v0;
                patch.patchOrientation = refPatch.patchOrientation;
                patch.u1 = refPatch.u1;
                patch.v1 = refPatch.v1;
                
                if (asps.patchSizeQuantizerPresentFlag)
                {
                    patch.size2DXInPixel = refPatch.size2DXInPixel;
                    patch.size2DYInPixel = refPatch.size2DYInPixel;

                    patch.sizeU0 = ceil((double)patch.size2DXInPixel / (double)occupancyPackingBlockSize);
                    patch.sizeV0 = ceil((double)patch.size2DYInPixel / (double)occupancyPackingBlockSize);
                }
                else
                {
                    patch.sizeU0 = refPatch.sizeU0;
                    patch.sizeV0 = refPatch.sizeV0;
                }

                patch.normalAxis = refPatch.normalAxis;
                patch.tangentAxis = refPatch.tangentAxis;
                patch.bitangentAxis = refPatch.bitangentAxis;
                patch.axisOfAdditionalPlane = refPatch.axisOfAdditionalPlane;

                const size_t max3DCoordinate = 1 << (gi.geometry3dCoordinatesBitdepthMinus1 + 1);

                if (patch.projectionMode == 0)
                {
                    patch.d1 = ((refPatch.d1 / minLevel)) * minLevel;
                }
                else
                {
                    if (asps.degree45ProjectionPatchPresentFlag == 0)
                    {
                        patch.d1 = max3DCoordinate - (((max3DCoordinate - refPatch.d1) / minLevel)) * minLevel;
                    }
                    else
                    {
                        patch.d1 = (max3DCoordinate << 1) - ((((max3DCoordinate << 1) - refPatch.d1) / minLevel)) * minLevel;
                    }
                }
                
                size_t prevDD = refPatch.sizeD / minLevel;
                
                if (prevDD * minLevel != refPatch.sizeD)
                {
                    prevDD += 1;
                }
                
                patch.sizeD = std::min(size_t((prevDD) * minLevel), (size_t)255);

                patch.levelOfDetailX = refPatch.levelOfDetailX;
                patch.levelOfDetailY = refPatch.levelOfDetailY;

                prevSizeU0 = patch.sizeU0;
                prevSizeV0 = patch.sizeV0;

                prevPatchSize2DXInPixel = patch.size2DXInPixel;
                prevPatchSize2DYInPixel = patch.size2DYInPixel;

                /*
                patch.allocOneLayerData();
                */
            }
            else if (currPatchType == PatchType::RAW)
            {
                RawPatchDataUnit& ppdu = pid.rawPatchDataUnit;

                MissedPointsPatch& missedPointsPatch = pcmPatches.at(patchIndex - numNonRawPatch);
                missedPointsPatch.u0 = ppdu.rpdu2dPosX;
                missedPointsPatch.v0 = ppdu.rpdu2dPosY;
                missedPointsPatch.sizeU0 = ppdu.rpdu2dDeltaSizeX;
                missedPointsPatch.sizeV0 = ppdu.rpdu2dDeltaSizeY;
                
                if (afps.afpsRaw3dPosBitCountExplicitModeFlag)
                {
                    missedPointsPatch.u1 = ppdu.rpdu3dPosX;
                    missedPointsPatch.v1 = ppdu.rpdu3dPosY;
                    missedPointsPatch.d1 = ppdu.rpdu3dPosZ;
                }
                else
                {
                    const size_t pcmU1V1D1Level = 1 << (gi.geometryNominal2dBitdepthMinus1 + 1);
                    missedPointsPatch.u1 = ppdu.rpdu3dPosX * pcmU1V1D1Level;
                    missedPointsPatch.v1 = ppdu.rpdu3dPosY * pcmU1V1D1Level;
                    missedPointsPatch.d1 = ppdu.rpdu3dPosZ * pcmU1V1D1Level;
                }
                
                missedPointsPatch.numberOfMps = ppdu.rpduRawPoints;
                missedPointsPatch.occupancyResolution = occupancyPackingBlockSize;

                totalNumberOfMps += missedPointsPatch.numberOfMps;
            }
            else if (currPatchType == PatchType::EOM)
            {
                EOMPatchDataUnit& epdu = pid.eomPatchDataUnit;
                std::vector<EomPatch>& eomPatches = frame.eomPatches;

                EomPatch eomPatch;
                eomPatch.u0 = epdu.epdu2dPosX;
                eomPatch.v0 = epdu.epdu2dPosY;
                eomPatch.sizeU = epdu.epdu2dDeltaSizeX;
                eomPatch.sizeV = epdu.epdu2dDeltaSizeY;
                eomPatch.memberPatches.resize(epdu.epduAssociatedPatcheCountMinus1 + 1);
                eomPatch.eddCountPerPatch.resize(epdu.epduAssociatedPatcheCountMinus1 + 1);
                eomPatch.eddCount = 0;

                for (size_t i = 0; i < eomPatch.memberPatches.size(); i++)
                {
                    eomPatch.memberPatches[i] = epdu.epduAssociatedPatches.at(i);
                    eomPatch.eddCountPerPatch[i] = epdu.epduEomPointsPerPatch.at(i);
                    eomPatch.eddCount += eomPatch.eddCountPerPatch[i];
                }

                eomPatches.push_back(eomPatch);
            }
            else if (currPatchType == PatchType::END)
            {
                break;
            }
            else
            {
                assert(false);
            }
        }
    }

    size_t patchToCanvas(Patch& patch, const size_t u, const size_t v, size_t canvasStride, size_t canvasHeight, size_t& x, size_t& y)
    {
        switch (patch.patchOrientation)
        {
            case PatchOrientation::DEFAULT:
                x = u + patch.u0 * patch.occupancyResolution;
                y = v + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::ROT90:
                x = (patch.sizeV0 * patch.occupancyResolution - 1 - v) + patch.u0 * patch.occupancyResolution;
                y = u + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::ROT180:
                x = (patch.sizeU0 * patch.occupancyResolution - 1 - u) + patch.u0 * patch.occupancyResolution;
                y = (patch.sizeV0 * patch.occupancyResolution - 1 - v) + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::ROT270:
                x = v + patch.u0 * patch.occupancyResolution;
                y = (patch.sizeU0 * patch.occupancyResolution - 1 - u) + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::MIRROR:
                x = (patch.sizeU0 * patch.occupancyResolution - 1 - u) + patch.u0 * patch.occupancyResolution;
                y = v + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::MROT90:
                x = (patch.sizeV0 * patch.occupancyResolution - 1 - v) + patch.u0 * patch.occupancyResolution;
                y = (patch.sizeU0 * patch.occupancyResolution - 1 - u) + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::MROT180:
                x = u + patch.u0 * patch.occupancyResolution;
                y = (patch.sizeV0 * patch.occupancyResolution - 1 - v) + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::MROT270:
                x = v + patch.u0 * patch.occupancyResolution;
                y = u + patch.v0 * patch.occupancyResolution;
                break;

            case PatchOrientation::SWAP:
                x = v + patch.u0 * patch.occupancyResolution;
                y = u + patch.v0 * patch.occupancyResolution;
                break;

            default:
                assert(0);
                break;
        }

        if (x < 0) return -1;
        if (y < 0) return -1;
        if (x >= canvasStride) return -1;
        if (y >= canvasHeight) return -1;

        return (x + canvasStride * y);
    }

    int32_t patchBlockToCanvasBlock(Patch& patch, const size_t blockU, const size_t blockV, size_t canvasStrideBlk, size_t canvasHeightBlk)
    {
        size_t x = 0;
        size_t y = 0;

        switch (patch.patchOrientation)
        {
            case PatchOrientation::DEFAULT:
                x = blockU + patch.u0;
                y = blockV + patch.v0;
                break;

            case PatchOrientation::ROT90:
                x = (patch.sizeV0 - 1 - blockV) + patch.u0;
                y = blockU + patch.v0;
                break;

            case PatchOrientation::ROT180:
                x = (patch.sizeU0 - 1 - blockU) + patch.u0;
                y = (patch.sizeV0 - 1 - blockV) + patch.v0;
                break;

            case PatchOrientation::ROT270:
                x = blockV + patch.u0;
                y = (patch.sizeU0 - 1 - blockU) + patch.v0;
                break;

            case PatchOrientation::MIRROR:
                x = (patch.sizeU0 - 1 - blockU) + patch.u0;
                y = blockV + patch.v0;
                break;

            case PatchOrientation::MROT90:
                x = (patch.sizeV0 - 1 - blockV) + patch.u0;
                y = (patch.sizeU0 - 1 - blockU) + patch.v0;
                break;

            case PatchOrientation::MROT180:
                x = blockU + patch.u0;
                y = (patch.sizeV0 - 1 - blockV) + patch.v0;
                break;

            case PatchOrientation::MROT270:
                x = blockV + patch.u0;
                y = blockU + patch.v0;
                break;

            case PatchOrientation::SWAP:
                x = blockV + patch.u0;
                y = blockU + patch.v0;
                break;

            default:
                return -1;
                break;
        }

        if (x < 0) return -1;
        if (y < 0) return -1;
        if (x >= canvasStrideBlk) return -1;
        if (y >= canvasHeightBlk) return -1;

        return (x + canvasStrideBlk * y);
    }

    void createBlockToPatchFromBoundaryBox(ParserContext& context, FrameData& frame, size_t occupancyResolution)
    {
        std::vector<Patch>& patches = frame.patches;
        const size_t patchCount = patches.size();

        const size_t blockToPatchWidth = frame.width / occupancyResolution;
        const size_t blockToPatchHeight = frame.height / occupancyResolution;

        const size_t blockCount = blockToPatchWidth * blockToPatchHeight;
        std::vector<size_t>& blockToPatch = frame.blockToPatch;

        blockToPatch.resize(blockCount, 0);

        for (size_t patchIndex = 0; patchIndex < patchCount; ++patchIndex)
        {
            Patch& patch = patches[patchIndex];

            for (size_t v0 = 0; v0 < patch.sizeV0; ++v0)
            {
                for (size_t u0 = 0; u0 < patch.sizeU0; ++u0)
                {
                    const int32_t blockIndex = patchBlockToCanvasBlock(patch, u0, v0, blockToPatchWidth, blockToPatchHeight);
                    
                    if (context.atlasSequenceParameterSet.at(0).patchPrecedenceOrderFlag)
                    {
                        if (blockToPatch[blockIndex] == 0)
                        {
                            blockToPatch[blockIndex] = patchIndex + 1;
                        }
                    }
                    else
                    {
                        blockToPatch[blockIndex] = patchIndex + 1;
                    }
                }
            }
        }
    }

    bool parse(Bitstream& bitstream, ParserContext& context, FrameGroup& frameGroup)
    {
        // Parse VPCC units
        VPCCUnitType::Enum unitType = VPCCUnitType::VPS;

        if (!vpccUnit(bitstream, context, frameGroup, unitType) && unitType != VPCCUnitType::VPS)
        {
            return false;
        }

        if (!vpccUnit(bitstream, context, frameGroup, unitType) && unitType != VPCCUnitType::AD)
        {
            return false;
        }

        if (!vpccUnit(bitstream, context, frameGroup, unitType) && unitType != VPCCUnitType::OVD)
        {
            return false;
        }

        if (!vpccUnit(bitstream, context, frameGroup, unitType) && unitType != VPCCUnitType::GVD)
        {
            return false;
        }

        if (!vpccUnit(bitstream, context, frameGroup, unitType) && unitType != VPCCUnitType::AVD)
        {
            return false;
        }
        
        createPatchFrameDataStructures(context, frameGroup);

        return true;
    }

    bool parseFirstFrameGroup(Bitstream& bitstream, FrameGroup& frameGroup)
    {
        uint32_t ssvhUnitSizePrecisionBytesMinus1;
        sampleStreamVpccHeader(bitstream, ssvhUnitSizePrecisionBytesMinus1);
        
        ParserContext context;
        context.ssvhUnitSizePrecisionBytesMinus1 = ssvhUnitSizePrecisionBytesMinus1;
        
        return parse(bitstream, context, frameGroup);
    }

    bool parseAllFrameGroups(Bitstream& bitstream, std::vector<FrameGroup>& frameGroups)
    {
        uint32_t ssvhUnitSizePrecisionBytesMinus1;
        sampleStreamVpccHeader(bitstream, ssvhUnitSizePrecisionBytesMinus1);
        
        while (size_t bytesLeft = VPCC::BitstreamReader::bytesAvailable(bitstream))
        {
            ParserContext context;
            context.ssvhUnitSizePrecisionBytesMinus1 = ssvhUnitSizePrecisionBytesMinus1;
            
            FrameGroup frameGroup;
            bool result = parse(bitstream, context, frameGroup);
            
            if (result)
            {
                frameGroups.push_back(frameGroup);
            }
            else
            {
                return false;
            }
        }

        return true;
    }
}

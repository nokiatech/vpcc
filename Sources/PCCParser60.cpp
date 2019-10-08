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

#include "PCCParser60.h"

#include <cmath>

#include "HEVC.h"
#include "Logger.h"
#include "FileSystem.h"

namespace PCC
{
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

    void parseVideoStream(VideoStream& stream, std::vector<VideoFrame>& videoFrames, VideoType::Enum type)
    {
        std::vector<HEVC::NALUnit> nalUnits;
        HEVC::readNALUnits(stream.data(), stream.size(), nalUnits);

        // Find POCs
        for (size_t startIndex = 0; startIndex < nalUnits.size();)
        {
            HEVC::NALUnit startNalUnit = nalUnits.at(startIndex);

            size_t endIndex = HEVC::findFrameEnd(startIndex, nalUnits);
            HEVC::NALUnit endNalUnit = nalUnits.at(endIndex);

            VideoFrame videoFrame;
            videoFrame.offset = startNalUnit.offset;
            videoFrame.length = (endNalUnit.offset - startNalUnit.offset) + endNalUnit.length;

            videoFrames.push_back(videoFrame);

            startIndex = endIndex + 1;
        }

#if 0 // DEBUG

        LOG_D("---------- VIDEO STREAM - NAL UNITS - BEGIN ----------");

        LOG_I("Video stream type: %s", VideoType::toString(type));

        for (uint32_t i = 0; i< nalUnits.size(); i++)
        {
            HEVC::NALUnit nalUnit = nalUnits[i];

            const char* string = HEVC::NALUnitType::toString(nalUnit.type);
            LOG_D("0x%zx (%zu) = %s, length = %lu, header length = %lu", nalUnit.offset, nalUnit.offset, string, nalUnit.length, nalUnit.headerLength);
        }

        LOG_D("---------- VIDEO STREAM - NAL UNITS - END ----------");

#endif
    }

    void dumpVideoStream(VideoStream& stream, VideoType::Enum type)
    {
#if 0 // DEBUG

        // Dump encoded YUV sequence
        std::string outputPath;

        if (type == VideoType::OCCUPANCY)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_OCCUPANCY.265");

            counter++;
        }
        else if (type == VideoType::GEOMETRY)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_GEOMETRY.265");

            counter++;
        }
        else if (type == VideoType::GEOMETRY_D0)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_GEOMETRY_D0.265");

            counter++;
        }
        else if (type == VideoType::GEOMETRY_D1)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_GEOMETRY_D1.265");

            counter++;
        }
        else if (type == VideoType::GEOMETRY_MP)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_GEOMETRY_MP.265");

            counter++;
        }
        else if (type == VideoType::TEXTURE)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_TEXTURE.265");

            counter++;
        }
        else if (type == VideoType::TEXTURE_MP)
        {
            static size_t counter = 0;

            outputPath.append(std::to_string(counter));
            outputPath.append("_TEXTURE_MP.265");

            counter++;
        }

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

        // TODO: Seems to be that TMC2 6.0 implementation reads 64 bits, but outputs uint32_t -> 32 bits are lost...
        BitstreamReader::skipBits(bitstream, 32);
        uint64_t totalSize = BitstreamReader::readUInt32(bitstream);

        header.magic = containerMagicNumber;
        header.version = containerVersion;
        header.totalSize = totalSize;

        return true;
    }

    void parsePcmSeparateVideoData(Bitstream& bitstream, ParserContext& context, uint8_t bitCount)
    {
        VPCCParameterSet& vpcc = context.vpcc;
        SequenceParameterSet& sps = context.sps;

        if (sps.pcmSeparateVideoPresentFlag && !vpcc.layerIndex)
        {
            vpcc.pcmVideoFlag = BitstreamReader::readBits(bitstream, 1);

            BitstreamReader::readBits(bitstream, bitCount);
        }
        else
        {
            BitstreamReader::readBits(bitstream, bitCount + 1);
        }
    }

    bool parseVPCCUnitHeader(Bitstream& bitstream, ParserContext& context, VPCCUnitType::Enum& vpccUnitType)
    {
        VPCCParameterSet& vpcc = context.vpcc;

        vpccUnitType = (VPCCUnitType::Enum)BitstreamReader::readBits(bitstream, 5);

        if (vpccUnitType == VPCCUnitType::AVD || vpccUnitType == VPCCUnitType::GVD ||
            vpccUnitType == VPCCUnitType::OVD || vpccUnitType == VPCCUnitType::PDG)
        {
            vpcc.sequenceParameterSetId = BitstreamReader::readBits(bitstream, 4);
        }

        if (vpccUnitType == VPCCUnitType::AVD)
        {
            vpcc.attributeIndex = BitstreamReader::readBits(bitstream, 7);
            vpcc.attributeDimensionIndex = BitstreamReader::readBits(bitstream, 7);

            SequenceParameterSet& sps = context.sps;

            if ( sps.multipleLayerStreamsPresentFlag)
            {
                vpcc.layerIndex = BitstreamReader::readBits(bitstream, 4);

                parsePcmSeparateVideoData(bitstream, context, 4);
            }
            else
            {
                parsePcmSeparateVideoData(bitstream, context, 8);
            }
        }
        else if (vpccUnitType == VPCCUnitType::GVD)
        {
            SequenceParameterSet& sps = context.sps;

            if (sps.multipleLayerStreamsPresentFlag)
            {
                vpcc.layerIndex = BitstreamReader::readBits(bitstream, 4);

                parsePcmSeparateVideoData(bitstream, context, 18);
            }
            else
            {
                parsePcmSeparateVideoData(bitstream, context, 22);
            }
        }
        else if (vpccUnitType == VPCCUnitType::OVD || vpccUnitType == VPCCUnitType::PDG)
        {
            // TODO: Just ignored
            BitstreamReader::readBits(bitstream, 23);
        }
        else
        {
            // TODO: Just ignored
            BitstreamReader::readBits(bitstream, 27);
        }

        return true;
    }

    void parseRefListStruct(Bitstream& bitstream, ParserContext& context, RefListStruct& rls, PatchSequenceParameterSet& psps)
    {
        rls.numRefEntries = BitstreamReader::readUVLC(bitstream);

        rls.absDeltaPfocSt.resize(rls.numRefEntries, 0);
        rls.pfocLsbLt.resize(rls.numRefEntries, 0);
        rls.stRefPatchFrameFlag.resize(rls.numRefEntries, false);
        rls.strpfEntrySignFlag.resize(rls.numRefEntries, false);

        for (size_t i = 0; i < rls.numRefEntries; i++)
        {
            if (psps.longTermRefPatchFramesFlag)
            {
                bool stRefPatchFrameFlag = BitstreamReader::readBits(bitstream, 1);
                rls.stRefPatchFrameFlag[i] = stRefPatchFrameFlag;

                if (stRefPatchFrameFlag)
                {
                    uint8_t absDeltaPfocSt = BitstreamReader::readUVLC(bitstream);
                    rls.absDeltaPfocSt[i] = absDeltaPfocSt;

                    if (rls.absDeltaPfocSt[i] > 0)
                    {
                        bool strpfEntrySignFlag = BitstreamReader::readBits(bitstream, 1);
                        rls.strpfEntrySignFlag[i] = strpfEntrySignFlag;
                    }
                    else
                    {
                        uint8_t bitCount = psps.log2MaxPatchFrameOrderCntLsb + 4;
                        rls.pfocLsbLt[i] = BitstreamReader::readBits(bitstream, bitCount);
                    }
                }
            }
        }
    }

    void parsePatchSequenceParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg)
    {
        uint32_t index = BitstreamReader::readUVLC(bitstream);

        RefListStruct rls;

        PatchSequenceParameterSet& psps = pdg.patchSequenceParameterSet[index];
        psps.refListStruct.push_back(rls);
        psps.patchSequenceParameterSetId = index;
        psps.log2PatchPackingBlockSize = BitstreamReader::readBits(bitstream, 3);
        psps.log2MaxPatchFrameOrderCntLsb = BitstreamReader::readUVLC(bitstream);
        psps.maxDecPatchFrameBufferingMinus1 = BitstreamReader::readUVLC(bitstream);
        psps.longTermRefPatchFramesFlag = BitstreamReader::readBits(bitstream, 1);
        psps.numRefPatchFrameListsInPsps = BitstreamReader::readUVLC(bitstream);

        psps.refListStruct.resize(psps.numRefPatchFrameListsInPsps);

        for (size_t i = 0; i < psps.numRefPatchFrameListsInPsps; i++)
        {
            parseRefListStruct(bitstream, context, psps.refListStruct[i], psps);
        }

        psps.useEightOrientationsFlag = BitstreamReader::readBits(bitstream, 1);
        psps.normalAxisLimitsQuantizationEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        psps.normalAxisMaxDeltaValueEnabledFlag = BitstreamReader::readBits(bitstream, 1);
    }

    void parseGeometryPatchParams(Bitstream& bitstream, ParserContext& context, GeometryPatchParams& gpp, PatchFrameGeometryParameterSet& gfps)
    {
        if (gfps.geometryPatchScaleParamsEnabledFlag)
        {
            gpp.geometryPatchScaleParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (gpp.geometryPatchScaleParamsPresentFlag)
            {
                for (size_t i = 0; i < 3; i++)
                {
                    uint32_t geometryPatchScaleOnAxis = BitstreamReader::readBits(bitstream, 32);
                    gpp.geometryPatchScaleOnAxis[i] = geometryPatchScaleOnAxis;
                }
            }
        }

        if (gfps.geometryPatchOffsetParamsEnabledFlag)
        {
            gpp.geometryPatchOffsetParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (gpp.geometryPatchOffsetParamsPresentFlag)
            {
                for (size_t i = 0; i < 3; i++)
                {
                    int32_t geometryPatchOffsetOnAxis = BitstreamReader::readBits(bitstream, 32);
                    gpp.geometryPatchOffsetOnAxis[i] = geometryPatchOffsetOnAxis;
                }
            }
        }

        if (gfps.geometryPatchRotationParamsEnabledFlag)
        {
            gpp.geometryPatchRotationParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (gpp.geometryPatchRotationParamsPresentFlag)
            {
                for (size_t i = 0; i < 4; i++)
                {
                    int32_t geometryPatchRotationXYZW = BitstreamReader::readBits(bitstream, 32);
                    gpp.geometryPatchRotationXYZW[i] = geometryPatchRotationXYZW;
                }
            }
        }

        if (gfps.geometryPatchPointSizeInfoEnabledFlag)
        {
            gpp.geometryPatchPointSizeInfoPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (gpp.geometryPatchPointSizeInfoPresentFlag)
            {
                gpp.geometryPatchPointSizeInfo = BitstreamReader::readBits(bitstream, 16);
            }
        }

        if (gfps.geometryPatchPointShapeInfoEnabledFlag)
        {
            gpp.geometryPatchPointShapeInfoPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (gpp.geometryPatchPointShapeInfoPresentFlag)
            {
                gpp.geometryPatchPointShapeInfo = BitstreamReader::readBits(bitstream, 4);
            }
        }
    }

    void parseGeometryPatchParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg)
    {
        uint32_t gppsIndex = BitstreamReader::readUVLC(bitstream);
        uint32_t pfgpsIndex = BitstreamReader::readUVLC(bitstream);

        GeometryPatchParameterSet& gpps = pdg.geometryPatchParameterSet[gppsIndex];
        gpps.geometryPatchParameterSetId = gppsIndex;
        gpps.patchFrameGeometryParameterSetId = pfgpsIndex;

        PatchFrameGeometryParameterSet& pfgps = pdg.patchFrameGeometryParameterSet[pfgpsIndex];

        if (pfgps.geometryPatchScaleParamsEnabledFlag ||
            pfgps.geometryPatchOffsetParamsEnabledFlag ||
            pfgps.geometryPatchRotationParamsEnabledFlag ||
            pfgps.geometryPatchPointSizeInfoEnabledFlag ||
            pfgps.geometryPatchPointShapeInfoEnabledFlag)
        {
            gpps.geometryPatchParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (gpps.geometryPatchParamsPresentFlag)
            {
                parseGeometryPatchParams(bitstream, context, gpps.geometryPatchParams, pfgps);
            }
        }

        byteAlignment(bitstream);
    }

    void parseAttributePatchParams(Bitstream& bitstream, ParserContext& context, AttributePatchParams& app, PatchFrameAttributeParameterSet& afps, size_t dimension)
    {
        if (afps.attributePatchScaleParamsEnabledFlag)
        {
            app.attributePatchScaleParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (app.attributePatchScaleParamsPresentFlag)
            {
                for (size_t i = 0; i < dimension; i++)
                {
                    uint32_t attributePatchScale = BitstreamReader::readBits(bitstream, 32);
                    app.attributePatchScale[i] = attributePatchScale;
                }
            }
        }

        if (afps.attributePatchOffsetParamsEnabledFlag)
        {
            app.attributePatchOffsetParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (app.attributePatchOffsetParamsPresentFlag)
            {
                for (size_t i = 0; i < dimension; i++ )
                {
                    int32_t attributePatchOffset = BitstreamReader::readBits(bitstream, 32);
                    app.attributePatchOffset[i] = attributePatchOffset;
                }
            }
        }
    }

    void parseAttributePatchParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, SequenceParameterSet& sps)
    {
        uint32_t appsIndex = BitstreamReader::readUVLC(bitstream);
        uint32_t pfapsIndex = BitstreamReader::readUVLC(bitstream);

        AttributePatchParameterSet& apps = pdg.attributePatchParameterSet[appsIndex];
        apps.attributePatchParameterSetId = appsIndex;
        apps.patchFrameAttributeParameterSetId = pfapsIndex;
        apps.attributeDimensionMinus1 = BitstreamReader::readBits(bitstream, 8);

        PatchFrameAttributeParameterSet& pfaps = pdg.patchFrameAttributeParameterSet[pfapsIndex];

        if (pfaps.attributePatchScaleParamsEnabledFlag || pfaps.attributePatchOffsetParamsEnabledFlag)
        {
            apps.attributePatchParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (apps.attributePatchParamsPresentFlag)
            {
                size_t attributeDimension = apps.attributeDimensionMinus1 + 1;

                parseAttributePatchParams(bitstream, context, apps.attributePatchParams, pfaps, attributeDimension);
            }
        }

        byteAlignment(bitstream);
    }

    void parsePatchFrameTileInformation(Bitstream& bitstream, ParserContext& context, PatchFrameTileInformation& pfti, SequenceParameterSet& sps)
    {
        pfti.singleTileInPatchFrameFlag = BitstreamReader::readBits(bitstream, 1);

        if (!pfti.singleTileInPatchFrameFlag)
        {
            pfti.uniformTileSpacingFlag = BitstreamReader::readBits(bitstream, 1);

            if (pfti.uniformTileSpacingFlag)
            {
                uint32_t tileColumnWidthMinus1 = BitstreamReader::readUVLC(bitstream);
                uint32_t tileRowHeightMinus1 = BitstreamReader::readUVLC(bitstream);

                pfti.tileColumnWidthMinus1[0] = tileColumnWidthMinus1;
                pfti.tileRowHeightMinus1[0] = tileRowHeightMinus1;
            }
            else
            {
                uint32_t numTileColumnsMinus1 = BitstreamReader::readUVLC(bitstream);
                uint32_t numTileRowsMinus1 = BitstreamReader::readUVLC(bitstream);

                pfti.numTileColumnsMinus1 = numTileColumnsMinus1;
                pfti.numTileRowsMinus1 = numTileRowsMinus1;

                for (size_t i = 0; i < pfti.numTileColumnsMinus1; i++)
                {
                    pfti.tileColumnWidthMinus1[i] = BitstreamReader::readUVLC(bitstream);
                }

                for (size_t i = 0; i < pfti.numTileRowsMinus1; i++)
                {
                    pfti.tileRowHeightMinus1[i] = BitstreamReader::readUVLC(bitstream);
                }
            }
        }

        pfti.singleTilePerTileGroupFlag = BitstreamReader::readBits(bitstream, 1);

        if (!pfti.singleTilePerTileGroupFlag)
        {
            const uint32_t numTilesInPatchFrame = (pfti.numTileColumnsMinus1 + 1) * (pfti.numTileRowsMinus1 + 1);
            pfti.numTileGroupsInPatchFrameMinus1 = BitstreamReader::readUVLC(bitstream);

            for (size_t i = 0; i <= pfti.numTileGroupsInPatchFrameMinus1; i++)
            {
                uint8_t bitCount = fixedLengthCodeBitsCount(numTilesInPatchFrame + 1);

                if (i > 0)
                {
                    pfti.topLeftTileIdx[i] = BitstreamReader::readBits(bitstream, bitCount);
                }

                bitCount = fixedLengthCodeBitsCount(numTilesInPatchFrame - pfti.topLeftTileIdx[i] + 1);

                pfti.bottomRightTileIdxDelta[i] = BitstreamReader::readBits(bitstream, bitCount);
            }
        }

        pfti.signalledTileGroupIdFlag = BitstreamReader::readBits(bitstream, 1);

        if (pfti.signalledTileGroupIdFlag)
        {
            pfti.signalledTileGroupIdLengthMinus1 = BitstreamReader::readUVLC(bitstream);

            for (size_t i = 0; i <= pfti.signalledTileGroupIdLengthMinus1; i++)
            {
                uint8_t bitCount = pfti.signalledTileGroupIdLengthMinus1 + 1;
                pfti.tileGroupId[i] = BitstreamReader::readBits(bitstream, bitCount);
            }
        }
    }

    void parsePatchFrameParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, SequenceParameterSet& sps)
    {
        AttributeInformation& ai = sps.attributeInformation;

        uint32_t pfpsIndex  = BitstreamReader::readUVLC(bitstream);
        uint32_t pspsIndex  = BitstreamReader::readUVLC(bitstream);
        uint32_t gpfpsIndex = BitstreamReader::readUVLC(bitstream);

        PatchFrameParameterSet& pfps = pdg.patchFrameParameterSet[pfpsIndex];

        pfps.patchFrameParameterSetId = pfpsIndex;
        pfps.patchSequenceParameterSetId = pspsIndex;
        pfps.geometryPatchFrameParameterSetId = gpfpsIndex;

        pfps.localOverrideAttributePatchEnabledFlag.resize(ai.attributeCount, false);
        pfps.attributePatchFrameParameterSetId.resize(ai.attributeCount, 0);

        for ( size_t i = 0; i < ai.attributeCount; i++ )
        {
            pfps.attributePatchFrameParameterSetId[i] = BitstreamReader::readUVLC(bitstream);
        }

        parsePatchFrameTileInformation(bitstream, context, pfps.patchFrameTileInformation, sps);

        pfps.localOverrideGeometryPatchEnabledFlag = BitstreamReader::readBits(bitstream, 1);

        for ( size_t i = 0; i < ai.attributeCount; i++ )
        {
            bool localOverrideAttributePatchEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfps.localOverrideAttributePatchEnabledFlag[i] = localOverrideAttributePatchEnabledFlag;
        }

        pfps.additionalLtPfocLsbLen = BitstreamReader::readUVLC(bitstream);

        if (sps.projection45DegreeEnabledFlag)
        {
            pfps.projection45DegreeEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        }
        else
        {
            pfps.projection45DegreeEnabledFlag = false;
        }

        byteAlignment(bitstream);
    }

    void parseAttributeFrameParams(Bitstream& bitstream, ParserContext& context, AttributeFrameParams& afp, size_t attributeDimension)
    {
        afp.attributeScale.resize(attributeDimension, 0);
        afp.attributeOffset.resize(attributeDimension, 0);
        afp.attributeSmoothingParamsPresentFlag.resize(attributeDimension, 0);
        afp.attributeSmoothingGridSizeMinus2.resize(attributeDimension, 0);
        afp.attributeSmoothingThreshold.resize(attributeDimension, 0);
        afp.attributeSmoothingThresholdAttributeDifference.resize(attributeDimension, 0);
        afp.attributeSmoothingThresholdAttributeVariation.resize(attributeDimension, 0);
        afp.attributeSmoothingLocalEntropyThreshold.resize(attributeDimension, 0);

        for (size_t i = 0; i < attributeDimension; i++)
        {
            bool attributeSmoothingParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
            afp.attributeSmoothingParamsPresentFlag[i] = attributeSmoothingParamsPresentFlag;
        }

        afp.attributeScaleParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        afp.attributeOffsetParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);

        for (size_t i = 0; i < attributeDimension; i++)
        {
            if (afp.attributeSmoothingParamsPresentFlag[i])
            {
                afp.attributeSmoothingGridSizeMinus2[i] = BitstreamReader::readBits(bitstream, 8);
                afp.attributeSmoothingThreshold[i] = BitstreamReader::readBits(bitstream, 8);
                afp.attributeSmoothingLocalEntropyThreshold[i] = BitstreamReader::readBits(bitstream, 3);
                afp.attributeSmoothingThresholdAttributeVariation[i] = BitstreamReader::readBits(bitstream, 8);
                afp.attributeSmoothingThresholdAttributeDifference[i] = BitstreamReader::readBits(bitstream, 8);
            }
        }

        if (afp.attributeScaleParamsPresentFlag)
        {
            for (size_t i = 0; i < attributeDimension; i++)
            {
                uint32_t attributeScale = BitstreamReader::readBits(bitstream, 32);
                afp.attributeScale[i] = attributeScale;
            }
        }

        if (afp.attributeOffsetParamsPresentFlag)
        {
            for ( size_t i = 0; i < attributeDimension; i++ )
            {
                int32_t attributeOffset = BitstreamReader::readBits(bitstream, 32);
                afp.attributeOffset[i] = attributeOffset;
            }
        }
    }

    void parsePatchFrameAttributeParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, SequenceParameterSet& sps)
    {
        uint32_t pfapsIndex = BitstreamReader::readUVLC(bitstream);
        uint32_t pspsIndex = BitstreamReader::readUVLC(bitstream);

        PatchFrameAttributeParameterSet& pfaps = pdg.patchFrameAttributeParameterSet[pfapsIndex];
        pfaps.patchFrameAttributeParameterSetId = pfapsIndex;
        pfaps.patchSequencParameterSetId = pspsIndex;

        AttributeInformation& ai = sps.attributeInformation;
        size_t attributeDimension = ai.attributeDimensionMinus1[pfaps.patchFrameAttributeParameterSetId] + 1;

        if (ai.attributeParamsEnabledFlag)
        {
            parseAttributeFrameParams(bitstream, context, pfaps.attributeFrameParams, attributeDimension);
        }
        else
        {
            pfaps.attributeFrameParams.attributeScale.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeOffset.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeSmoothingParamsPresentFlag.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeSmoothingGridSizeMinus2.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeSmoothingThreshold.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeSmoothingThresholdAttributeDifference.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeSmoothingThresholdAttributeVariation.resize(attributeDimension, 0);
            pfaps.attributeFrameParams.attributeSmoothingLocalEntropyThreshold.resize(attributeDimension, 0);
        }

        if (ai.attributePatchParamsEnabledFlag)
        {
            pfaps.attributePatchScaleParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfaps.attributePatchOffsetParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        }

        byteAlignment(bitstream);
    }

    void parseGeometryFrameParams(Bitstream& bitstream, ParserContext& context, GeometryFrameParams& gfp)
    {
        gfp.geometrySmoothingParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        gfp.geometryScaleParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        gfp.geometryOffsetParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        gfp.geometryRotationParamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        gfp.geometryPointSizeInfoPresentFlag = BitstreamReader::readBits(bitstream, 1);
        gfp.geometryPointShapeInfoPresentFlag = BitstreamReader::readBits(bitstream, 1);

        if (gfp.geometrySmoothingParamsPresentFlag)
        {
            gfp.geometrySmoothingEnabledFlag = BitstreamReader::readBits(bitstream, 1);

            if (gfp.geometrySmoothingEnabledFlag)
            {
                gfp.geometrySmoothingGridSizeMinus2 = BitstreamReader::readBits(bitstream, 7);
                gfp.geometrySmoothingThreshold = BitstreamReader::readBits(bitstream, 8);
            }
        }

        if (gfp.geometryScaleParamsPresentFlag)
        {
            for ( size_t d = 0; d < 3; d++ )
            {
                gfp.geometryScaleOnAxis[d] = BitstreamReader::readBits(bitstream, 32);
            }
        }

        if (gfp.geometryOffsetParamsPresentFlag)
        {
            for ( size_t d = 0; d < 3; d++ )
            {
                gfp.geometryOffsetOnAxis[d] = BitstreamReader::readBits(bitstream, 32);
            }
        }

        if (gfp.geometryRotationParamsPresentFlag)
        {
            for ( size_t d = 0; d < 4; d++ )
            {
                gfp.geometryRotationXYZW[d] = BitstreamReader::readBits(bitstream, 32);
            }
        }

        if (gfp.geometryPointSizeInfoPresentFlag)
        {
            gfp.geometryPointSizeInfo = BitstreamReader::readBits(bitstream, 16);
        }

        if (gfp.geometryPointShapeInfoPresentFlag)
        {
            gfp.geometryPointShapeInfo = BitstreamReader::readBits(bitstream, 4);
        }
    }

    void patchFrameGeometryParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, SequenceParameterSet& sps)
    {
        uint32_t pfgpsIndex = BitstreamReader::readUVLC(bitstream);
        uint32_t pspsIndex = BitstreamReader::readUVLC(bitstream);

        PatchFrameGeometryParameterSet& pfgps = pdg.patchFrameGeometryParameterSet[pfgpsIndex];
        pfgps.patchFrameGeometryParameterSetId = pfgpsIndex;
        pfgps.patchSequenceParameterSetId = pspsIndex;

        GeometryInformation& gi = sps.geometryInformation;

        if (gi.geometryParamsEnabledFlag)
        {
            parseGeometryFrameParams(bitstream, context, pfgps.geometryFrameParams);
        }

        if (gi.geometryPatchParamsEnabledFlag)
        {
            pfgps.geometryPatchScaleParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchOffsetParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchRotationParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchPointSizeInfoEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchPointShapeInfoEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        }

        byteAlignment(bitstream);
    }

    void parsePatchFrameGeometryParameterSet(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, SequenceParameterSet& sps)
    {
        uint32_t pfgpsIndex = BitstreamReader::readUVLC(bitstream);
        uint32_t pspsIndex = BitstreamReader::readUVLC(bitstream);

        PatchFrameGeometryParameterSet& pfgps = pdg.patchFrameGeometryParameterSet[pfgpsIndex];
        pfgps.patchFrameGeometryParameterSetId = pfgpsIndex;
        pfgps.patchSequenceParameterSetId = pspsIndex;

        GeometryInformation& gi = sps.geometryInformation;

        if (gi.geometryParamsEnabledFlag)
        {
            parseGeometryFrameParams(bitstream, context, pfgps.geometryFrameParams);
        }

        if (gi.geometryPatchParamsEnabledFlag)
        {
            pfgps.geometryPatchScaleParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchOffsetParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchRotationParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchPointSizeInfoEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            pfgps.geometryPatchPointShapeInfoEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        }

        byteAlignment(bitstream);
    }

    void parsePatchTileGroupHeader(Bitstream& bitstream, ParserContext& context, PatchTileGroupHeader& ptgh, PatchTileGroupHeader& pfhPrev)
    {
        SequenceParameterSet& sps = context.sps;
        GeometryInformation& gi = sps.geometryInformation;
        PatchDataGroup& pdg = context.pdg;

        uint32_t pfpsIndex = BitstreamReader::readUVLC(bitstream);
        ptgh.patchFrameParameterSetId = pfpsIndex;

        PatchFrameParameterSet& pfps = pdg.patchFrameParameterSet[ptgh.patchFrameParameterSetId];
        PatchSequenceParameterSet& psps = pdg.patchSequenceParameterSet[pfps.patchSequenceParameterSetId];
        PatchFrameTileInformation& pfti = pfps.patchFrameTileInformation;

        ptgh.address = BitstreamReader::readBits(bitstream, pfti.signalledTileGroupIdLengthMinus1 + 1);
        ptgh.type = BitstreamReader::readUVLC(bitstream);
        ptgh.patchFrameOrderCntLsb = BitstreamReader::readBits(bitstream, psps.log2MaxPatchFrameOrderCntLsb + 4);

        if (psps.numRefPatchFrameListsInPsps > 0)
        {
            ptgh.refPatchFrameListSpsFlag = BitstreamReader::readBits(bitstream, 1);
        }

        if (ptgh.refPatchFrameListSpsFlag)
        {
            if (psps.numRefPatchFrameListsInPsps > 1)
            {
                uint32_t bitCount = fixedLengthCodeBitsCount(psps.numRefPatchFrameListsInPsps + 1);
                ptgh.refPatchFrameListIdx = BitstreamReader::readBits(bitstream, bitCount);
            }
        }
        else
        {
            RefListStruct rls;
            parseRefListStruct(bitstream, context, rls, psps);

            psps.refListStruct.push_back(rls);
        }

        uint8_t rlsIdx = psps.numRefPatchFrameListsInPsps ? ptgh.refPatchFrameListIdx : psps.numRefPatchFrameListsInPsps;
        RefListStruct& rls = psps.refListStruct[rlsIdx];

        size_t numLtrpEntries = 0;

        for (size_t i = 0; i < rls.numRefEntries; i++)
        {
            if (!rls.stRefPatchFrameFlag[i])
            {
                numLtrpEntries++;
            }
        }

        for (size_t j = 0; j < numLtrpEntries; j++)
        {
            bool additionalPfocLsbPresentFlag = BitstreamReader::readBits(bitstream, 1);
            ptgh.additionalPfocLsbPresentFlag[j] = additionalPfocLsbPresentFlag;

            if (additionalPfocLsbPresentFlag)
            {
                uint8_t bitCount = pfps.additionalLtPfocLsbLen;

                uint32_t additionalPfocLsbVal = BitstreamReader::readBits(bitstream, bitCount);
                ptgh.additionalPfocLsbVal[j] = additionalPfocLsbVal;
            }
        }

        ptgh.normalAxisMinValueQuantizer = 0;
        ptgh.normalAxisMaxDeltaValueQuantizer = 0;

        if (psps.normalAxisLimitsQuantizationEnabledFlag)
        {
            ptgh.normalAxisMinValueQuantizer = BitstreamReader::readBits(bitstream, 5);

            if (psps.normalAxisMaxDeltaValueEnabledFlag)
            {
                ptgh.normalAxisMaxDeltaValueQuantizer = BitstreamReader::readBits(bitstream, 5);
            }
        }

        const uint8_t maxBitCountForMinDepth = (uint8_t)gi.geometry3dCoordinatesBitdepthMinus1;
        const uint8_t maxBitCountForMaxDepth = (uint8_t)gi.geometry3dCoordinatesBitdepthMinus1;

        ptgh.interPredictPatch3dShiftNormalAxisBitCountMinus1 = maxBitCountForMinDepth;

        if (pfps.projection45DegreeEnabledFlag == 0)
        {
            ptgh.interPredictPatch2dDeltaSizeDBitCountMinus1 = maxBitCountForMaxDepth;
        }
        else
        {
            ptgh.interPredictPatch2dDeltaSizeDBitCountMinus1 = maxBitCountForMaxDepth + 1;
        }

        if (ptgh.type == PatchFrameType::P && rls.numRefEntries > 1)
        {
            ptgh.numRefIdxActiveOverrideFlag = BitstreamReader::readBits(bitstream, 1);

            if (ptgh.numRefIdxActiveOverrideFlag)
            {
                ptgh.numRefIdxActiveMinus1 = BitstreamReader::readUVLC(bitstream);
            }
        }

        if (ptgh.type == PatchFrameType::I)
        {
            ptgh.interPredictPatch2dShiftUBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
            ptgh.interPredictPatch2dShiftVBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
            ptgh.interPredictPatch3dShiftTangentAxisBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
            ptgh.interPredictPatch3dShiftBitangentAxisBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
            ptgh.interPredictPatchLodBitCount = BitstreamReader::readBits(bitstream, 8);
        }
        else
        {
            ptgh.interPredictPatchBitCountFlag = BitstreamReader::readBits(bitstream, 1);

            if (ptgh.interPredictPatchBitCountFlag)
            {
                ptgh.interPredictPatch2dShiftUBitCountFlag = BitstreamReader::readBits(bitstream, 1);

                if (ptgh.interPredictPatch2dShiftUBitCountFlag)
                {
                    ptgh.interPredictPatch2dShiftUBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
                }

                ptgh.interPredictPatch2dShiftVBitCountFlag = BitstreamReader::readBits(bitstream, 1);

                if (ptgh.interPredictPatch2dShiftVBitCountFlag)
                {
                    ptgh.interPredictPatch2dShiftVBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
                }

                ptgh.interPredictPatch3dShiftTangentAxisBitCountFlag = BitstreamReader::readBits(bitstream, 1);

                if (ptgh.interPredictPatch3dShiftTangentAxisBitCountFlag)
                {
                    ptgh.interPredictPatch3dShiftTangentAxisBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
                }

                ptgh.interPredictPatch3dShiftBitangentAxisBitCountFlag = BitstreamReader::readBits(bitstream, 1);

                if (ptgh.interPredictPatch3dShiftBitangentAxisBitCountFlag)
                {
                    ptgh.interPredictPatch3dShiftBitangentAxisBitCountMinus1 = BitstreamReader::readBits(bitstream, 8);
                }

                ptgh.interPredictPatchLodBitCountFlag = BitstreamReader::readBits(bitstream, 1);

                if (ptgh.interPredictPatchLodBitCountFlag)
                {
                    ptgh.interPredictPatchLodBitCount = BitstreamReader::readBits(bitstream, 8) + 1;
                }
            }

            if (!ptgh.interPredictPatchBitCountFlag || !ptgh.interPredictPatch2dShiftUBitCountFlag)
            {
                ptgh.interPredictPatch2dShiftUBitCountMinus1 = pfhPrev.interPredictPatch2dShiftUBitCountMinus1;
            }

            if ( !ptgh.interPredictPatchBitCountFlag || !ptgh.interPredictPatch2dShiftVBitCountFlag)
            {
                ptgh.interPredictPatch2dShiftVBitCountMinus1 = pfhPrev.interPredictPatch2dShiftVBitCountMinus1;
            }

            if (!ptgh.interPredictPatchBitCountFlag || !ptgh.interPredictPatch3dShiftTangentAxisBitCountFlag)
            {
                ptgh.interPredictPatch3dShiftTangentAxisBitCountMinus1 = pfhPrev.interPredictPatch3dShiftTangentAxisBitCountMinus1;
            }

            if (!ptgh.interPredictPatchBitCountFlag || !ptgh.interPredictPatch3dShiftBitangentAxisBitCountFlag)
            {
                ptgh.interPredictPatch3dShiftBitangentAxisBitCountMinus1 = pfhPrev.interPredictPatch3dShiftBitangentAxisBitCountMinus1;
            }

            if (!ptgh.interPredictPatchBitCountFlag || !ptgh.interPredictPatchLodBitCountFlag)
            {
                ptgh.interPredictPatchLodBitCount = pfhPrev.interPredictPatchLodBitCount;
            }
        }

        if (sps.pcmPatchEnabledFlag)
        {
            ptgh.pcm3dShiftBitCountPresentFlag = BitstreamReader::readBits(bitstream, 1);

            if (ptgh.pcm3dShiftBitCountPresentFlag)
            {
                ptgh.pcm3dShiftAxisBitCountMinus1 = BitstreamReader::readBits(bitstream, gi.geometry3dCoordinatesBitdepthMinus1 + 1);
            }
        }
        else
        {
            size_t bitCountPcmU1V1D1 = gi.geometry3dCoordinatesBitdepthMinus1 - gi.geometryNominal2dBitdepthMinus1;
            ptgh.pcm3dShiftAxisBitCountMinus1 = bitCountPcmU1V1D1 - 1;
        }

        byteAlignment(bitstream);
    }

    void parsePointLocalReconstructionData(Bitstream& bitstream, ParserContext& context, PointLocalReconstructionData& plrd)
    {
        SequenceParameterSet& sps = context.sps;
        PointLocalReconstructionInformation& plri = sps.pointLocalReconstructionInformation;

        const size_t  blockCount = plrd.blockToPatchMapWidth * plrd.blockToPatchMapHeight;
        const uint8_t bitCountMode = (uint8_t)fixedLengthCodeBitsCount((uint32_t)plri.numberOfModesMinus1);

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
            for ( size_t i = 0; i < blockCount; i++ )
            {
                plrd.blockPresentFlag[i] = BitstreamReader::readBits(bitstream, 1);

                if (plrd.blockPresentFlag[i])
                {
                    plrd.blockModeMinus1[i] = BitstreamReader::readBits(bitstream, bitCountMode);
                }
            }
        }
    }

    void parsePatchDataUnit(Bitstream& bitstream, ParserContext& context, PatchDataUnit& pdu, PatchTileGroupHeader& ptgh)
    {
        SequenceParameterSet& sps = context.sps;

        uint8_t ptghPatchFrameParameterSetId = ptgh.patchFrameParameterSetId;
        PatchFrameParameterSet& pfps = context.pdg.patchFrameParameterSet[ptghPatchFrameParameterSetId];

        uint8_t pfpsPatchSequenceParameterSetId = pfps.patchSequenceParameterSetId;
        PatchSequenceParameterSet& psps = context.pdg.patchSequenceParameterSet[pfpsPatchSequenceParameterSetId];

        pdu.shiftU = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch2dShiftUBitCountMinus1 + 1);
        pdu.shiftV = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch2dShiftVBitCountMinus1 + 1);

        pdu.deltaSizeU = BitstreamReader::readSVLC(bitstream);
        pdu.deltaSizeV = BitstreamReader::readSVLC(bitstream);

        pdu.shiftTangentAxis = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch3dShiftTangentAxisBitCountMinus1 + 1);
        pdu.shiftBiTangentAxis = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch3dShiftBitangentAxisBitCountMinus1 + 1);
        pdu.shiftMinNormalAxis = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch3dShiftNormalAxisBitCountMinus1 + 1);

        if (psps.normalAxisMaxDeltaValueEnabledFlag)
        {
            pdu.shiftDeltaMaxNormalAxis = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch2dDeltaSizeDBitCountMinus1 + 1);
        }

        pdu.projectPlane = (Axis6::Enum)BitstreamReader::readBits(bitstream, 3);

        if (psps.useEightOrientationsFlag)
        {
            pdu.orientationIndex = BitstreamReader::readBits(bitstream, 3);
        }
        else
        {
            pdu.orientationIndex = BitstreamReader::readBits(bitstream, 1);
        }

        if (ptgh.interPredictPatchLodBitCount > 0)
        {
            pdu.lod = BitstreamReader::readBits(bitstream, ptgh.interPredictPatchLodBitCount);
        }

        if (pfps.projection45DegreeEnabledFlag)
        {
            pdu.projection45DegreePresentFlag = BitstreamReader::readBits(bitstream, 1);
        }

        if (pdu.projection45DegreePresentFlag)
        {
            pdu.projection45DegreeRotationAxis = BitstreamReader::readBits(bitstream, 2);
        }
        else
        {
            pdu.projection45DegreeRotationAxis = 0;
        }

        if (sps.pointLocalReconstructionEnabledFlag)
        {
            PointLocalReconstructionData& plrd = pdu.pointLocalReconstructionData;

            size_t plrBlockToPatchMapWidth = context.previousPatchSizeU + pdu.deltaSizeU;
            size_t plrBlockToPatchMapHeight = context.previousPatchSizeV + pdu.deltaSizeV;

            plrd.blockToPatchMapWidth = plrBlockToPatchMapWidth;
            plrd.blockToPatchMapHeight = plrBlockToPatchMapHeight;

            plrd.blockPresentFlag.resize(plrBlockToPatchMapWidth * plrBlockToPatchMapHeight, false);
            plrd.blockModeMinus1.resize(plrBlockToPatchMapWidth * plrBlockToPatchMapHeight, 0);

            parsePointLocalReconstructionData(bitstream, context, plrd);

            context.previousPatchSizeU += pdu.deltaSizeU;
            context.previousPatchSizeV += pdu.deltaSizeV;
        }
    }

    void parseDeltaPatchDataUnit(Bitstream& bitstream, ParserContext& context, DeltaPatchDataUnit& dpdu, PatchTileGroupHeader& ptgh)
    {
        SequenceParameterSet& sps = context.sps;
        PatchDataGroup& pdg = context.pdg;

        uint8_t ptghPatchFrameParameterSetId = ptgh.patchFrameParameterSetId;
        PatchFrameParameterSet& pfps = pdg.patchFrameParameterSet[ptghPatchFrameParameterSetId];

        uint8_t pfpsPatchSequenceParameterSetId = pfps.patchSequenceParameterSetId;
        PatchSequenceParameterSet& psps = pdg.patchSequenceParameterSet[pfpsPatchSequenceParameterSetId];

        dpdu.deltaPatchIndex = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaShiftU = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaShiftV = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaSizeU = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaSizeV = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaShiftTangentAxis = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaShiftBiTangentAxis = BitstreamReader::readSVLC(bitstream);
        dpdu.deltaShiftMinNormalAxis = BitstreamReader::readSVLC(bitstream);

        dpdu.lod = 0;

        if (psps.normalAxisMaxDeltaValueEnabledFlag)
        {
            dpdu.shiftDeltaMaxNormalAxis = BitstreamReader::readSVLC(bitstream);
        }

        if (sps.pointLocalReconstructionEnabledFlag)
        {
            PointLocalReconstructionData& plrd = dpdu.pointLocalReconstructionData;
            PatchTileGroupLayerUnit& pfluPrev = pdg.patchTileGroupLayerUnit[context.predictionFramePatchTileGroupLayerUnitIndex];
            PatchTileGroupHeader& pfhPrev = pfluPrev.patchTileGroupHeader;
            PatchTileGroupDataUnit& pfduPrev = pfluPrev.patchTileGroupDataUnit;
            PatchInformationData& pidPrev = pfduPrev.patchInformationData[dpdu.deltaPatchIndex + context.predictionPatchIndex];

            uint8_t patchMode = pfduPrev.patchMode[dpdu.deltaPatchIndex + context.predictionPatchIndex];

            size_t sizeU = dpdu.deltaSizeU;
            size_t sizeV = dpdu.deltaSizeV;

            if ((pfhPrev.type == PatchFrameType::I && patchMode == PatchModeI::INTRA) ||
                ((pfhPrev.type) == PatchFrameType::P && patchMode == PatchModeP::INTRA))
            {
                sizeU += pidPrev.patchDataUnit.pointLocalReconstructionData.blockToPatchMapWidth;
                sizeV += pidPrev.patchDataUnit.pointLocalReconstructionData.blockToPatchMapHeight;
            }
            else if (pfhPrev.type == PatchFrameType::P && patchMode == PatchModeP::INTER)
            {
                sizeU += pidPrev.deltaPatchDataUnit.pointLocalReconstructionData.blockToPatchMapWidth;
                sizeV += pidPrev.deltaPatchDataUnit.pointLocalReconstructionData.blockToPatchMapHeight;
            }

            plrd.blockToPatchMapWidth = sizeU;
            plrd.blockToPatchMapHeight = sizeV;
            plrd.blockPresentFlag.resize(plrd.blockToPatchMapWidth * plrd.blockToPatchMapHeight, false);
            plrd.blockModeMinus1.resize(plrd.blockToPatchMapWidth * plrd.blockToPatchMapHeight, 0);

            parsePointLocalReconstructionData(bitstream, context, plrd);

            context.previousPatchSizeU = sizeU;
            context.previousPatchSizeV = sizeV;

            context.predictionPatchIndex += (dpdu.deltaPatchIndex + 1);
        }
    }

    void parsePcmPatchDataUnit(Bitstream& bitstream, ParserContext& context, PCMPatchDataUnit& ppdu, PatchTileGroupHeader& ptgh)
    {
        SequenceParameterSet& sps = context.sps;

        if (sps.pcmSeparateVideoPresentFlag)
        {
            ppdu.patchInPcmVideoFlag = BitstreamReader::readBits(bitstream, 1);
        }

        ppdu.shiftU = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch2dShiftUBitCountMinus1 + 1);
        ppdu.shiftV = BitstreamReader::readBits(bitstream, ptgh.interPredictPatch2dShiftVBitCountMinus1 + 1);

        ppdu.deltaSizeU = BitstreamReader::readSVLC(bitstream);
        ppdu.deltaSizeV = BitstreamReader::readSVLC(bitstream);

        ppdu.shiftTangentAxis = BitstreamReader::readBits(bitstream, ptgh.pcm3dShiftAxisBitCountMinus1 + 1);
        ppdu.shiftBiTangentAxis = BitstreamReader::readBits(bitstream, ptgh.pcm3dShiftAxisBitCountMinus1 + 1);
        ppdu.shiftNormalAxis = BitstreamReader::readBits(bitstream, ptgh.pcm3dShiftAxisBitCountMinus1 + 1);

        ppdu.pcmPoints = BitstreamReader::readUVLC(bitstream);
    }

    void parsePatchInformationData(Bitstream& bitstream, ParserContext& context, PatchInformationData& pid, size_t patchMode, PatchTileGroupHeader& ptgh)
    {
        SequenceParameterSet& sps = context.sps;
        PatchDataGroup& pdg = context.pdg;

        AttributeInformation& ai = sps.attributeInformation;
        PatchFrameParameterSet& pfps = pdg.patchFrameParameterSet[ptgh.patchFrameParameterSetId];

        pid.overrideAttributePatchFlag.resize(ai.attributeCount, false);
        pid.attributePatchParameterSetId.resize(ai.attributeCount, 0);

        if ((ptgh.type == PatchFrameType::I && patchMode == PatchModeI::INTRA) ||
            (ptgh.type == PatchFrameType::P && patchMode == PatchModeP::INTRA))
        {
            if (pfps.localOverrideGeometryPatchEnabledFlag)
            {
                pid.overrideGeometryPatchFlag = BitstreamReader::readBits(bitstream, 1);

                if (pid.overrideGeometryPatchFlag)
                {
                    pid.geometryPatchParameterSetId = BitstreamReader::readUVLC(bitstream);
                }
            }

            pfps.localOverrideAttributePatchEnabledFlag.resize(ai.attributeCount, false);
            pfps.attributePatchFrameParameterSetId.resize(ai.attributeCount, 0);

            for (int32_t i = 0; i < ai.attributeCount; i++)
            {
                bool localOverrideAttributePatchEnabledFlag = pfps.localOverrideAttributePatchEnabledFlag[i];

                if (localOverrideAttributePatchEnabledFlag)
                {
                    bool overrideAttributePatchFlag = BitstreamReader::readBits(bitstream, 1);
                    pid.overrideAttributePatchFlag.push_back(overrideAttributePatchFlag);
                }
                else
                {
                    pid.overrideAttributePatchFlag.push_back(false);
                }

                bool overrideAttributePatchFlag = pid.overrideAttributePatchFlag[i];

                if (overrideAttributePatchFlag)
                {
                    pid.attributePatchParameterSetId.push_back(BitstreamReader::readUVLC(bitstream));
                }
                else
                {
                    pid.attributePatchParameterSetId.push_back(0);
                }
            }

            PatchDataUnit& pdu = pid.patchDataUnit;
            pdu.frameIndex = pid.frameIndex;
            pdu.patchIndex = pid.patchIndex;

            parsePatchDataUnit(bitstream, context, pdu, ptgh);
        }
        else if (ptgh.type == PatchFrameType::P && patchMode == PatchModeP::INTER)
        {
            DeltaPatchDataUnit& dpdu = pid.deltaPatchDataUnit;
            dpdu.frameIndex = pid.frameIndex;
            dpdu.patchIndex = pid.patchIndex;

            parseDeltaPatchDataUnit(bitstream, context, dpdu, ptgh);
        }
        else if ((ptgh.type == PatchFrameType::I && patchMode == PatchModeI::PCM) ||
                 (ptgh.type == PatchFrameType::P && patchMode == PatchModeP::PCM))
        {
            PCMPatchDataUnit& ppdu = pid.pcmPatchDataUnit;
            ppdu.frameIndex = pid.frameIndex;
            ppdu.patchIndex = pid.patchIndex;

            parsePcmPatchDataUnit(bitstream, context, ppdu, ptgh);
        }
    }

    void parsePatchTileGroupDataUnit(Bitstream& bitstream, ParserContext& context, PatchTileGroupDataUnit& ptgdu, PatchTileGroupHeader& ptgh)
    {
        context.previousPatchSizeU = 0;
        context.previousPatchSizeV = 0;

        context.predictionPatchIndex = 0;

        size_t patchIndex = 0;

        PatchFrameType::Enum tileGroupType = (PatchFrameType::Enum)ptgh.type;
        uint8_t patchMode = BitstreamReader::readUVLC(bitstream);

        ptgdu.patchMode.clear();
        ptgdu.patchInformationData.clear();

        while (!((tileGroupType == PatchFrameType::I && patchMode == PatchModeI::END) ||
                 (tileGroupType == PatchFrameType::P && patchMode == PatchModeP::END)))
        {
            ptgdu.patchMode.push_back(patchMode);

            PatchInformationData pid;
            pid.frameIndex = ptgdu.frameIndex;
            pid.patchIndex = patchIndex;

            patchIndex++;

            parsePatchInformationData(bitstream, context, pid, patchMode, ptgh);

            ptgdu.patchInformationData.push_back(pid);

            patchMode = BitstreamReader::readUVLC(bitstream);
        }

        byteAlignment(bitstream);
    }

    void parsePatchTileGroupLayerUnit(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, uint32_t frameIndex)
    {
        PatchTileGroupLayerUnit tmp;
        pdg.patchTileGroupLayerUnit.push_back(tmp);

        PatchTileGroupLayerUnit& ptglu = pdg.patchTileGroupLayerUnit.back();
        ptglu.frameIndex = frameIndex;

        PatchTileGroupHeader& ptgh = ptglu.patchTileGroupHeader;
        ptgh.frameIndex = frameIndex;

        PatchTileGroupDataUnit& ptgdu = ptglu.patchTileGroupDataUnit;
        ptgdu.frameIndex = frameIndex;

        size_t index = std::max(0, (int32_t)pdg.patchTileGroupLayerUnit.size() - 1);
        assert(pdg.patchTileGroupLayerUnit.size() > index);

        PatchTileGroupLayerUnit& ptgluPrev = pdg.patchTileGroupLayerUnit[index];
        parsePatchTileGroupHeader(bitstream, context, ptgh, ptgluPrev.patchTileGroupHeader);

        parsePatchTileGroupDataUnit(bitstream, context, ptgdu, ptgh);
    }

    void parsePrefixSei(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg)
    {
        // TODO: No spec yet!
    }

    void parseSuffixSei(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg)
    {
        // TODO: No spec yet!
    }

    void parsePatchDataGroupUnitPayload(Bitstream& bitstream, ParserContext& context, PatchDataGroup& pdg, SequenceParameterSet& sps, PDGUnitType::Enum unitType, size_t frameIndex)
    {
        switch (unitType)
        {
            case PDGUnitType::PSPS:
            {
                parsePatchSequenceParameterSet(bitstream, context, pdg);
                break;
            }

            case PDGUnitType::GPPS:
            {
                parseGeometryPatchParameterSet(bitstream, context, pdg);
                break;
            }

            case PDGUnitType::APPS:
            {
                parseAttributePatchParameterSet(bitstream, context, pdg, sps);
                break;
            }

            case PDGUnitType::PFPS:
            {
                parsePatchFrameParameterSet(bitstream, context, pdg, sps);
                break;
            }

            case PDGUnitType::PFAPS:
            {
                parsePatchFrameAttributeParameterSet(bitstream, context, pdg, sps);
                break;
            }

            case PDGUnitType::PFGPS:
            {
                parsePatchFrameGeometryParameterSet(bitstream, context, pdg, sps);
                break;
            }

            case PDGUnitType::PTGLU:
            {
                parsePatchTileGroupLayerUnit(bitstream, context, pdg, frameIndex);
                break;
            }

            case PDGUnitType::PREFIX_SEI:
            {
                parsePrefixSei(bitstream, context, pdg);
                break;
            }

            case PDGUnitType::SUFFIX_SEI:
            {
                parseSuffixSei(bitstream, context, pdg);
                break;
            }

            default:
            {
                assert(0);
                break;
            }
        }
    }

    void parsePatchDataGroup(Bitstream& bitstream, ParserContext& context)
    {
        SequenceParameterSet& sps = context.sps;
        PatchDataGroup& pdg = context.pdg;

        context.predictionFramePatchTileGroupLayerUnitIndex = -1;

        size_t frameCount = 0;
        size_t prevFrameindex = 0;

        size_t i = 0;

        do
        {
            PDGUnitType::Enum unitType = (PDGUnitType::Enum)BitstreamReader::readUVLC(bitstream);
            parsePatchDataGroupUnitPayload(bitstream, context, pdg, sps, unitType, frameCount);

            if (unitType == PDGUnitType::PTGLU)
            {
                frameCount++;
                prevFrameindex = i;

                context.predictionFramePatchTileGroupLayerUnitIndex++;
            }

            i++;
        }
        while (!BitstreamReader::readBits(bitstream, 1));

        byteAlignment( bitstream );
    }

    void parseVideoBitstream(Bitstream& bitstream, ParserContext& context, VideoType::Enum videoType)
    {
        uint32_t size = BitstreamReader::readBits(bitstream, 32);

        FrameGroup& frameGroup = *context.currentFrameGroup;
        std::vector<uint8_t>* videoBitstream = NULL;

        if (videoType == VideoType::OCCUPANCY)
        {
            videoBitstream = &frameGroup.occupancy;
        }
        else if (videoType == VideoType::GEOMETRY)
        {
            videoBitstream = &frameGroup.geometry;
        }
        else if (videoType == VideoType::GEOMETRY_D0)
        {
            videoBitstream = &frameGroup.geometryD0;
        }
        else if (videoType == VideoType::GEOMETRY_D1)
        {
            videoBitstream = &frameGroup.geometryD1;
        }
        else if (videoType == VideoType::GEOMETRY_MP)
        {
            videoBitstream = &frameGroup.geometryMP;
        }
        else if (videoType == VideoType::TEXTURE)
        {
            videoBitstream = &frameGroup.texture;
        }
        else if (videoType == VideoType::TEXTURE_MP)
        {
            videoBitstream = &frameGroup.textureMP;
        }

        videoBitstream->resize(size);

        size_t bytesRead = BitstreamReader::readBytes(bitstream, videoBitstream->data(), size);
        assert(size == bytesRead);
    }

    void parseVPCCVideoDataUnit(Bitstream& bitstream, ParserContext& context, VPCCUnitType::Enum vpccUnitType)
    {
        SequenceParameterSet& sps = context.sps;

        if (vpccUnitType == VPCCUnitType::OVD)
        {
            parseVideoBitstream(bitstream, context, VideoType::OCCUPANCY);
        }
        else if (vpccUnitType == VPCCUnitType::GVD)
        {
            size_t index = sps.layerCountMinus1 > 0 ? 1 : 0;

            if (!sps.layerAbsoluteCodingEnabledFlag[index])
            {
                parseVideoBitstream(bitstream, context, VideoType::GEOMETRY_D0);
                parseVideoBitstream(bitstream, context, VideoType::GEOMETRY_D1);
            }
            else
            {
                parseVideoBitstream(bitstream, context, VideoType::GEOMETRY);
            }

            if (sps.pcmPatchEnabledFlag && sps.pcmSeparateVideoPresentFlag)
            {
                parseVideoBitstream(bitstream, context, VideoType::GEOMETRY_MP);
            }
        }
        else if (vpccUnitType == VPCCUnitType::AVD)
        {
            if (sps.attributeInformation.attributeCount > 0)
            {
                parseVideoBitstream(bitstream, context, VideoType::TEXTURE);

                if (sps.pcmPatchEnabledFlag && sps.pcmSeparateVideoPresentFlag)
                {
                    parseVideoBitstream(bitstream, context, VideoType::TEXTURE_MP);
                }
            }
        }
    }

    void parseProfileTierLevel(Bitstream& bitstream, ParserContext& context, ProfileTierLevel& ptl)
    {
        ptl.tierFlag = BitstreamReader::readBits(bitstream, 1);
        ptl.profileCodecGroupIdc = BitstreamReader::readBits(bitstream, 7);
        ptl.profilePccToolsetIdc = BitstreamReader::readBits(bitstream, 8);
        ptl.profileReconctructionIdc = BitstreamReader::readBits(bitstream, 8);

        BitstreamReader::readBits(bitstream, 32);

        ptl.levelIdc = BitstreamReader::readBits(bitstream, 8);
    }

    void parseOccupancyInformation(Bitstream& bitstream, ParserContext& context, OccupancyInformation& oi)
    {
        oi.occupancyCodecId = BitstreamReader::readBits(bitstream, 8);
        oi.lossyOccupancyMapCompressionThreshold = BitstreamReader::readBits(bitstream, 8);
    }

    void parseGeometryInformation(Bitstream& bitstream, ParserContext& context, GeometryInformation& gi, SequenceParameterSet& sps)
    {
        gi.geometryCodecId = BitstreamReader::readBits(bitstream, 8);
        gi.geometryNominal2dBitdepthMinus1 = BitstreamReader::readBits(bitstream, 5);
        gi.geometry3dCoordinatesBitdepthMinus1 = BitstreamReader::readBits(bitstream, 5);

        if (sps.pcmSeparateVideoPresentFlag)
        {
            gi.pcmGeometryCodecId = BitstreamReader::readBits(bitstream, 8);
        }

        gi.geometryParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        gi.geometryPatchParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
    }

    void parseAttributeInformation(Bitstream& bitstream, ParserContext& context, AttributeInformation& ai, SequenceParameterSet& sps)
    {
        ai.attributeCount = BitstreamReader::readBits(bitstream, 7);

        ai.attributeTypeId.resize(ai.attributeCount, 0);
        ai.attributeCodecId.resize(ai.attributeCount, 0);
        ai.pcmAttributeCodecId.resize(ai.attributeCount, 0);
        ai.attributeDimensionMinus1.resize(ai.attributeCount, 0);
        ai.attributeDimensionPartitionsMinus1.resize(ai.attributeCount, 0);
        ai.attributeNominal2dBitdepthMinus1.resize(ai.attributeCount, 0);
        ai.attributePartitionChannelsMinus1.resize(ai.attributeCount);

        for (size_t i = 0; i < ai.attributeCount; i++)
        {
            uint8_t attributeTypeId = BitstreamReader::readBits(bitstream, 4);
            uint8_t attributeCodecId = BitstreamReader::readBits(bitstream, 8);

            ai.attributeTypeId[i] = attributeTypeId;
            ai.attributeCodecId[i] = attributeCodecId;

            if (sps.pcmSeparateVideoPresentFlag)
            {
                uint8_t pcmAttributeCodecId = BitstreamReader::readBits(bitstream, 8);
                ai.pcmAttributeCodecId[i] = pcmAttributeCodecId;
            }

            uint8_t attributeDimensionMinus1 = BitstreamReader::readBits(bitstream, 8);
            ai.attributeDimensionMinus1[i] = attributeDimensionMinus1;

            if (attributeDimensionMinus1 > 0)
            {
                uint8_t attributeDimensionPartitionsMinus1 = BitstreamReader::readBits(bitstream, 7);
                ai.attributeDimensionPartitionsMinus1[i] = attributeDimensionPartitionsMinus1;

                int32_t remainingDimensions = ai.attributeDimensionMinus1[i];
                int32_t k = ai.attributeDimensionPartitionsMinus1[i];

                for (int32_t j = 0; j < k; j++)
                {
                    if ((k - j) == remainingDimensions)
                    {
                        ai.attributePartitionChannelsMinus1[i][j] = 0;
                    }
                    else
                    {
                        uint32_t uvlc = BitstreamReader::readUVLC(bitstream);
                        ai.attributePartitionChannelsMinus1[i][j] = uvlc;
                    }

                    remainingDimensions -= ai.attributePartitionChannelsMinus1[i][j] + 1;
                }

                if (k >= ai.attributePartitionChannelsMinus1[i].size())
                {
                    ai.attributePartitionChannelsMinus1[i].resize(k + 1, 0);
                }

                ai.attributePartitionChannelsMinus1[i][k] = remainingDimensions;
            }

            uint8_t attributeNominal2dBitdepthMinus1 = BitstreamReader::readBits(bitstream, 5);
            ai.attributeNominal2dBitdepthMinus1[i] = attributeNominal2dBitdepthMinus1;
        }

        if (ai.attributeCount > 0)
        {
            ai.attributeParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            ai.attributePatchParamsEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            ai.attributeMSBAlignFlag = BitstreamReader::readBits(bitstream, 1);
        }
    }

    void parsePointLocalReconstructionInformation(Bitstream& bitstream, ParserContext& context, PointLocalReconstructionInformation& plri)
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

    void parseSequenceParameterSet(Bitstream& bitstream, ParserContext& context)
    {
        SequenceParameterSet& sps = context.sps;

        parseProfileTierLevel(bitstream, context, sps.profileTierLevel);

        sps.sequenceParameterSetId = BitstreamReader::readBits(bitstream, 4);
        sps.frameWidth = BitstreamReader::readBits(bitstream, 16);
        sps.frameHeight = BitstreamReader::readBits(bitstream, 16);
        sps.avgFrameRatePresentFlag = BitstreamReader::readBits(bitstream, 1);

        if ( sps.avgFrameRatePresentFlag)
        {
            sps.avgFrameRate = BitstreamReader::readBits(bitstream, 16);
        }

        sps.enhancedOccupancyMapForDepthFlag = BitstreamReader::readBits(bitstream, 1);
        sps.layerCountMinus1 = BitstreamReader::readBits(bitstream, 4);

        sps.layerAbsoluteCodingEnabledFlag.resize(sps.layerCountMinus1 + 1);
        sps.layerPredictorIndexDiff.resize(sps.layerCountMinus1 + 1);

        if (sps.layerCountMinus1 > 0)
        {
            sps.multipleLayerStreamsPresentFlag = BitstreamReader::readBits(bitstream, 1);
        }
        else
        {
            sps.layerAbsoluteCodingEnabledFlag[0] = BitstreamReader::readBits(bitstream, 1);
        }

        for (size_t i = 0; i < sps.layerCountMinus1; i++)
        {
            size_t index = (i + 1);

            if (sps.layerAbsoluteCodingEnabledFlag.size() < index + 1)
            {
                sps.layerAbsoluteCodingEnabledFlag.resize(index + 1, 0);
            }

            bool layerAbsoluteCodingEnabledFlag = BitstreamReader::readBits(bitstream, 1);
            sps.layerAbsoluteCodingEnabledFlag[index] = layerAbsoluteCodingEnabledFlag;

            if (layerAbsoluteCodingEnabledFlag == 0)
            {
                if (i > 0)
                {
                    size_t index = (i + 1);

                    if (sps.layerPredictorIndexDiff.size() < index + 1)
                    {
                        sps.layerPredictorIndexDiff.resize(index + 1);
                    }

                    uint32_t uvlc = BitstreamReader::readUVLC(bitstream);
                    sps.layerPredictorIndexDiff[index] = uvlc;
                }
                else
                {
                    size_t index = 0;

                    if (sps.layerPredictorIndexDiff.size() < index + 1)
                    {
                        sps.layerPredictorIndexDiff.resize(index + 1);
                    }

                    sps.layerPredictorIndexDiff[(index + 1)] = 0;
                }
            }
        }

        sps.pcmPatchEnabledFlag = BitstreamReader::readBits(bitstream, 1);

        if (sps.pcmPatchEnabledFlag)
        {
            sps.pcmSeparateVideoPresentFlag = BitstreamReader::readBits(bitstream, 1);
        }

        parseOccupancyInformation(bitstream, context, sps.occupancyInformation);
        parseGeometryInformation(bitstream, context, sps.geometryInformation, sps);
        parseAttributeInformation(bitstream, context, sps.attributeInformation, sps);

        sps.patchInterPredictionEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        sps.pixelDeinterleavingFlag = BitstreamReader::readBits(bitstream, 1);
        sps.pointLocalReconstructionEnabledFlag = BitstreamReader::readBits(bitstream, 1);

        if (sps.pointLocalReconstructionEnabledFlag)
        {
            parsePointLocalReconstructionInformation(bitstream, context, sps.pointLocalReconstructionInformation);
        }

        sps.removeDuplicatePointEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        sps.projection45DegreeEnabledFlag = BitstreamReader::readBits(bitstream, 1);
        sps.patchPrecedenceOrderFlag = BitstreamReader::readBits(bitstream, 1);

        // NOTE: Not part of the V-PCC documets, needs to be removed in future
        {
            sps.losslessGeo444 = BitstreamReader::readBits(bitstream, 1);
            sps.losslessGeo = BitstreamReader::readBits(bitstream, 1);
            sps.losslessTexture = BitstreamReader::readBits(bitstream, 1);
            sps.minLevel = BitstreamReader::readBits(bitstream, 8);
            sps.surfaceThickness = BitstreamReader::readBits(bitstream, 8);
        }

        byteAlignment(bitstream);
    }

    bool parseVPCCUnitPayload(Bitstream& bitstream, ParserContext& context, VPCCUnitType::Enum& vpccUnitType)
    {
        if (vpccUnitType == VPCCUnitType::SPS)
        {
            parseSequenceParameterSet(bitstream, context);
        }
        else if (vpccUnitType == VPCCUnitType::PDG)
        {
            parsePatchDataGroup(bitstream, context);
        }
        else if (vpccUnitType == VPCCUnitType::OVD || vpccUnitType == VPCCUnitType::GVD || vpccUnitType == VPCCUnitType::AVD)
        {
            parseVPCCVideoDataUnit(bitstream, context, vpccUnitType);
        }

        return true;
    }

    bool parseVPCCUnit(Bitstream& bitstream, ParserContext& context, VPCCUnitType::Enum& vpccUnitType)
    {
        if (parseVPCCUnitHeader(bitstream, context, vpccUnitType))
        {
            return parseVPCCUnitPayload(bitstream, context, vpccUnitType);
        }

        return false;
    }

    void createPatches(ParserContext& context, Frame& frame, Frame& previousFrame, size_t frameIndex)
    {
        SequenceParameterSet& sps = context.sps;
        PatchDataGroup& pdg = context.pdg;
        PatchTileGroupLayerUnit& ptglu = pdg.patchTileGroupLayerUnit[frameIndex];
        PatchTileGroupHeader& ptgh = ptglu.patchTileGroupHeader;
        PatchTileGroupDataUnit& ptgdu = ptglu.patchTileGroupDataUnit;
        PatchFrameParameterSet& pfps = pdg.patchFrameParameterSet[0];

        uint32_t previousSizeU0 = 0;
        uint32_t previousSizeV0 = 0;

        int64_t predictionIndex = 0;

        const size_t minLevel = sps.minLevel;

        PatchFrameType::Enum patchFrameType = (PatchFrameType::Enum)ptgh.type;

        size_t patchCount = ptgdu.patchMode.size();

        size_t numPCMPatches = 0;
        size_t numNonPCMPatch = 0;

        // Calculate number of PCM patches
        for (size_t i = 0; i < patchCount; i++)
        {
            if ((patchFrameType == PatchFrameType::I && ptgdu.patchMode[i] == PatchModeI::PCM) ||
                (patchFrameType == PatchFrameType::P && ptgdu.patchMode[i] == PatchModeP::PCM))
            {
                numPCMPatches++;
            }
        }

        numNonPCMPatch = patchCount - numPCMPatches;

        frame.patches.resize(numNonPCMPatch);

        size_t totalNumberOfMps = 0;
        size_t patchIndex = 0;

        PatchSequenceParameterSet& psps = context.pdg.patchSequenceParameterSet[0];
        uint32_t occupancyPackingBlockSize = ::pow(2, psps.log2PatchPackingBlockSize);

        for (patchIndex = 0; patchIndex < patchCount; patchIndex++)
        {
            PatchInformationData& pid = ptgdu.patchInformationData[patchIndex];

            if ((patchFrameType == PatchFrameType::I && ptgdu.patchMode[patchIndex] == PatchModeI::INTRA) ||
                (patchFrameType == PatchFrameType::P && ptgdu.patchMode[patchIndex] == PatchModeP::INTRA))
            {
                    PatchDataUnit& pdu = pid.patchDataUnit;

                    Patch& patch = frame.patches[patchIndex];
                    patch.occupancyResolution = occupancyPackingBlockSize;
                    patch.u0 = pdu.shiftU;
                    patch.v0 = pdu.shiftV;
                    patch.u1 = pdu.shiftTangentAxis;
                    patch.v1 = pdu.shiftBiTangentAxis;
                    patch.sizeD = (uint32_t)std::min(pdu.shiftDeltaMaxNormalAxis * minLevel, (size_t)255);
                    patch.sizeU0 = previousSizeU0 + pdu.deltaSizeU;
                    patch.sizeV0 = previousSizeV0 + pdu.deltaSizeV;
                    patch.normalAxis = uint32_t(pdu.projectPlane) % 3;
                    patch.projectionMode = uint32_t(pdu.projectPlane) < 3 ? 0 : 1;
                    patch.patchOrientation = pdu.orientationIndex;
                    patch.axisOfAdditionalPlane = pdu.projection45DegreePresentFlag ? pdu.projection45DegreeRotationAxis : 0;

                    const int32_t max3DCoordinate = 1 << (context.sps.geometryInformation.geometry3dCoordinatesBitdepthMinus1 + 1);

                    size_t index = sps.layerCountMinus1 > 0 ? 1 : 0;

                    if (patch.projectionMode == 0 || !context.sps.layerAbsoluteCodingEnabledFlag[index])
                    {
                        patch.d1 = (int32_t)pdu.shiftMinNormalAxis * minLevel;
                    }
                    else
                    {
                        if (pfps.projection45DegreeEnabledFlag == 0)
                        {
                            patch.d1 = max3DCoordinate - (int32_t)pdu.shiftMinNormalAxis * minLevel;
                        }
                        else
                        {
                            patch.d1 = max3DCoordinate - (int32_t)pdu.shiftMinNormalAxis * minLevel;
                        }
                    }

                    previousSizeU0 = patch.sizeU0;
                    previousSizeV0 = patch.sizeV0;

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
                }
                else if (patchFrameType == PatchFrameType::P && ptgdu.patchMode[patchIndex] == PatchModeP::INTER)
                {
                    DeltaPatchDataUnit& dpdu = pid.deltaPatchDataUnit;

                    int64_t bestMatchIndex = dpdu.deltaPatchIndex + predictionIndex;
                    const Patch& previousPatch = previousFrame.patches[bestMatchIndex];

                    predictionIndex += dpdu.deltaPatchIndex + 1;

                    Patch& patch = frame.patches[patchIndex];
                    patch.u0 = dpdu.deltaShiftU + previousPatch.u0;
                    patch.v0 = dpdu.deltaShiftV + previousPatch.v0;
                    patch.u1 = dpdu.deltaShiftTangentAxis + previousPatch.u1;
                    patch.v1 = dpdu.deltaShiftBiTangentAxis + previousPatch.v1;
                    patch.sizeU0 = dpdu.deltaSizeU + previousPatch.sizeU0;
                    patch.sizeV0 = dpdu.deltaSizeV + previousPatch.sizeV0;
                    patch.occupancyResolution = occupancyPackingBlockSize;
                    patch.normalAxis = previousPatch.normalAxis;
                    patch.tangentAxis = previousPatch.tangentAxis;
                    patch.bitangentAxis = previousPatch.bitangentAxis;
                    patch.projectionMode = previousPatch.projectionMode;
                    patch.patchOrientation = previousPatch.patchOrientation;
                    patch.axisOfAdditionalPlane = previousPatch.axisOfAdditionalPlane;
                    patch.bestMatchIndex = (int32_t)bestMatchIndex;

                    const int32_t max3DCoordinate = 1 << (context.sps.geometryInformation.geometry3dCoordinatesBitdepthMinus1 + 1);

                    size_t index = sps.layerCountMinus1 > 0 ? 1 : 0;

                    if (patch.projectionMode == 0 || !context.sps.layerAbsoluteCodingEnabledFlag[index])
                    {
                        patch.d1 = (dpdu.deltaShiftMinNormalAxis + (previousPatch.d1 / minLevel)) * minLevel;
                    }
                    else
                    {
                        if (pfps.projection45DegreeEnabledFlag == 0)
                        {
                            patch.d1 = max3DCoordinate - (dpdu.deltaShiftMinNormalAxis + ((max3DCoordinate - previousPatch.d1) / minLevel)) * minLevel;
                        }
                        else
                        {
                            patch.d1 = (max3DCoordinate << 1) - (dpdu.deltaShiftMinNormalAxis + (((max3DCoordinate << 1) - previousPatch.d1) / minLevel)) * minLevel;
                        }
                    }

                    const int64_t delta_DD = dpdu.shiftDeltaMaxNormalAxis;
                    int64_t prevDD = previousPatch.sizeD / minLevel;

                    if (prevDD * minLevel != previousPatch.sizeD)
                    {
                        prevDD += 1;
                    }

                    patch.sizeD = (uint32_t)std::min(int64_t((delta_DD + prevDD) * minLevel), (int64_t)255);

                    previousSizeU0 = patch.sizeU0;
                    previousSizeV0 = patch.sizeV0;
                }
                else if ((ptgh.type == PatchFrameType::I && ptgdu.patchMode[patchIndex] == (uint8_t)PatchModeI::PCM) ||
                         (ptgh.type == PatchFrameType::P && ptgdu.patchMode[patchIndex] == (uint8_t)PatchModeI::PCM))
                {
                    assert(false);
                }
                else if ((ptgh.type == PatchFrameType::I && ptgdu.patchMode[patchIndex] == (uint8_t)PatchModeP::END) ||
                         (ptgh.type == PatchFrameType::P && ptgdu.patchMode[patchIndex] == (uint8_t)PatchModeP::END))
                {
                    assert(false);
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

    void createBlockToPatchFromBoundaryBox(Frame& frame)
    {
        std::vector<Patch>& patches = frame.patches;
        const size_t patchCount = patches.size();

        const size_t blockToPatchWidth = frame.width / patches[0].occupancyResolution;
        const size_t blockToPatchHeight = frame.height / patches[0].occupancyResolution;

        const size_t blockCount = blockToPatchWidth * blockToPatchHeight;
        std::vector<size_t>& blockToPatch = frame.blockToPatch;

        blockToPatch.resize(blockCount, 0);

        for (size_t patchIndex = 0; patchIndex < patchCount; ++patchIndex)
        {
            Patch& patch = patches[patchIndex];

            for (size_t v0 = 0; v0 < patch.sizeV0; ++v0)
            {
                for ( size_t u0 = 0; u0 < patch.sizeU0; ++u0 )
                {
                    const int32_t blockIndex  = patchBlockToCanvasBlock(patch, u0, v0, blockToPatchWidth, blockToPatchHeight);
                    blockToPatch[blockIndex] = patchIndex + 1;
                }
            }
        }
    }

    bool parse(Bitstream& bitstream, FrameGroup& frameGroup)
    {
        ParserContext context;
        context.currentFrameGroup = &frameGroup;

        // Parse VPCC units
        VPCCUnitType::Enum unitType;

        if (!parseVPCCUnit(bitstream, context, unitType) && unitType != VPCCUnitType::SPS)
        {
            return false;
        }

        if (!parseVPCCUnit(bitstream, context, unitType) && unitType != VPCCUnitType::PDG)
        {
            return false;
        }

        if (!parseVPCCUnit(bitstream, context, unitType) && unitType != VPCCUnitType::OVD)
        {
            return false;
        }

        if (!parseVPCCUnit(bitstream, context, unitType) && unitType != VPCCUnitType::GVD)
        {
            return false;
        }

        if (!parseVPCCUnit(bitstream, context, unitType) && unitType != VPCCUnitType::AVD)
        {
            return false;
        }

        // Generate all frame data ready for video playback
        {
            frameGroup.sps = context.sps;
            frameGroup.pdg = context.pdg;

            // Parse HEVC video streams and create video frames
            std::vector<VideoFrame> occupanyFrames;
            parseVideoStream(frameGroup.occupancy, occupanyFrames, VideoType::OCCUPANCY);
            dumpVideoStream(frameGroup.occupancy, VideoType::OCCUPANCY);

            std::vector<VideoFrame> geometryFrames;
            parseVideoStream(frameGroup.geometry, geometryFrames, VideoType::GEOMETRY);
            dumpVideoStream(frameGroup.geometry, VideoType::GEOMETRY);

#if 0

            std::vector<VideoFrame> geometryD0Frames;
            parseVideoStream(frameGroup.geometryD0, geometryD0Frames, VideoType::GEOMETRY_D0);
            dumpVideoStream(frameGroup.geometryD0, VideoType::GEOMETRY_D0);

            std::vector<VideoFrame> geometryD1Frames;
            parseVideoStream(frameGroup.geometryD1, geometryD1Frames, VideoType::GEOMETRY_D1);
            dumpVideoStream(frameGroup.geometryD1, VideoType::GEOMETRY_D1);

            std::vector<VideoFrame> geometryMPFrames;
            parseVideoStream(frameGroup.geometryMP, geometryMPFrames, VideoType::GEOMETRY_MP);
            dumpVideoStream(frameGroup.geometryMP, VideoType::GEOMETRY_MP);

#endif

            std::vector<VideoFrame> textureFrames;
            parseVideoStream(frameGroup.texture, textureFrames, VideoType::TEXTURE);
            dumpVideoStream(frameGroup.texture, VideoType::TEXTURE);

#if 0

            std::vector<VideoFrame> textureMPFrames;
            parseVideoStream(frameGroup.textureMP, textureMPFrames, VideoType::TEXTURE_MP);
            dumpVideoStream(frameGroup.textureMP, VideoType::TEXTURE_MP);

#endif

            bool dualLayerSkipping = (occupanyFrames.size() * 2 == geometryFrames.size());

            // Create PCC frames
            size_t frameCount = context.pdg.patchTileGroupLayerUnit.size();
            frameGroup.frames.resize(frameCount);

            size_t previousFrameIndex = 0;

            for (size_t i = 0; i < frameCount; i++)
            {
                Frame& previousFrame = frameGroup.frames[previousFrameIndex];

                Frame& frame = frameGroup.frames[i];
                frame.index = i;
                frame.width = frameGroup.sps.frameWidth;
                frame.height = frameGroup.sps.frameHeight;

                frame.occupancy = occupanyFrames[i];

                if (dualLayerSkipping)
                {
                    frame.geometry = geometryFrames[(i * 2)];
                }
                else
                {
                    frame.geometry = geometryFrames[i];
                }

#if 0

                frame.geometryD0 = geometryD0Frames[i];
                frame.geometryD1 = geometryD1Frames[i];
                frame.geometryMP = geometryMPFrames[i];

#endif

                if (dualLayerSkipping)
                {
                    frame.texture = textureFrames[(i * 2)];
                }
                else
                {
                    frame.texture = textureFrames[i];
                }

#if 0

                frame.textureMP = textureMPFrames[i];

#endif

                frame.presentationTimeUs = i;

                createPatches(context, frame, previousFrame, i);
                createBlockToPatchFromBoundaryBox(frame);

                previousFrameIndex = i;
            }
        }

        return true;
    }
}

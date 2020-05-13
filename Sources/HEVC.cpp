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

#include "HEVC.h"

#include "Helpers.h"
#include "Logger.h"

namespace HEVC
{
    NALUnitType::Enum readNALUnitHeader(Bitstream& bs)
    {
        size_t size = bs.length();
        uint8_t* buffer = bs.data();

        for (uint32_t i = 0; (i + 3) < size; i++)
        {
            size_t startOffset = 3;

            // Search for NAL unit
            bool nalUnitFound = buffer[i + 0] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 1;

            if (!nalUnitFound)
            {
                if (size - i >= 4 && buffer[i + 0] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 0 && buffer[i + 3] == 1)
                {
                    nalUnitFound = true;
                    startOffset = 4;
                }
            }

            if (nalUnitFound)
            {
                bs.seek(startOffset);

                // forbidden_zero_bit
                uint32_t forbidden_zero_bit = bs.readBits(1);
                assert(forbidden_zero_bit == 0);

                // nal_unit_type
                NALUnitType::Enum type = (NALUnitType::Enum)bs.readBits(6);

                // nuh_layer_id
                bs.readBits(6);

                // nuh_temporal_id_plus1
                bs.readBits(3);

                return type;
            }
        }

        return NALUnitType::NAL_UNIT_INVALID;
    }

    bool readNALUnits(uint8_t* data, size_t bytes, std::vector<NALUnit>& output)
    {
        Bitstream bitstream = Bitstream(data, bytes);

        return readNALUnits(bitstream, output);
    }

    bool readNALUnits(Bitstream& bitstream, std::vector<NALUnit>& output)
    {
        size_t startPosition = bitstream.position();

        uint8_t* buffer = bitstream.data() + startPosition;
        size_t size = bitstream.length();

        for (uint32_t i = 0; (i + 3) < size; i++)
        {
            size_t startOffset = 3;

            // Search for NAL unit
            bool nalUnitFound = buffer[i + 0] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 1;

            if (!nalUnitFound)
            {
                if (size - i >= 4 && buffer[i + 0] == 0 && buffer[i + 1] == 0 && buffer[i + 2] == 0 && buffer[i + 3] == 1)
                {
                    nalUnitFound = true;
                    startOffset = 4;
                }
            }

            if (nalUnitFound)
            {
                // Parse NAL unit header
                Bitstream bs = Bitstream(buffer + i + startOffset, size - i - startOffset);

                // forbidden_zero_bit
                uint32_t forbidden_zero_bit = bs.readBits(1);
                assert(forbidden_zero_bit == 0);

                // nal_unit_type
                NALUnitType::Enum type = (NALUnitType::Enum)bs.readBits(6);

                // nuh_layer_id
                bs.readBits(6);

                // nuh_temporal_id_plus1
                bs.readBits(3);

                // Cache NAL unit info
                NALUnit nalUnit;
                nalUnit.type = type;
                nalUnit.offset = i;
                nalUnit.length = 0;
                nalUnit.headerLength = startOffset;

                output.push_back(nalUnit);

                i += 3;
            }
        }

        // Calculate NAL unit lengths
        for (uint32_t i = 0; (i + 1) < output.size(); i++)
        {
            NALUnit current = output[i];
            NALUnit next = output[i + 1];

            current.length = (next.offset - current.offset);
            output[i] = current;
        }

        if (output.size() > 0)
        {
            size_t index = output.size() - 1;

            NALUnit nalUnit = output[index];
            nalUnit.length = (size - nalUnit.offset);

            output[index] = nalUnit;
        }

        // Map local offsets to global offsets
        for (uint32_t i = 0; i < output.size(); i++)
        {
            NALUnit nalUnit = output[i];
            nalUnit.offset = nalUnit.offset + startPosition;

            output[i] = nalUnit;
        }

        return true;
    }

    bool parseSlices(uint8_t* data, size_t bytes, std::vector<HEVC::NALUnit> nalUnits, std::vector<HEVC::Slice>& slices)
    {
        ParserContext parserContext;

        for (size_t i = 0; i < nalUnits.size(); ++i)
        {
            HEVC::NALUnit nalUnit = nalUnits.at(i);

            Bitstream bitstream = Bitstream(data + nalUnit.offset, nalUnit.length);

            switch (nalUnit.type)
            {
                case NALUnitType::NAL_UNIT_VPS:
                {
                    VPS vps;
                    parseVPS(bitstream, vps);

                    parserContext.videoParameterSets.push_back(vps);

                    break;
                }

                case NALUnitType::NAL_UNIT_SPS:
                {
                    SPS sps;
                    parseSPS(bitstream, sps);

                    parserContext.sequenceParameterSets.push_back(sps);

                    break;
                }

                case NALUnitType::NAL_UNIT_PPS:
                {
                    PPS pps;
                    parsePPS(bitstream, pps);

                    parserContext.pictureParameterSets.push_back(pps);

                    break;
                }

                case NALUnitType::NAL_UNIT_CODED_SLICE_TRAIL_R:
                case NALUnitType::NAL_UNIT_CODED_SLICE_TRAIL_N:
                case NALUnitType::NAL_UNIT_CODED_SLICE_TSA_N:
                case NALUnitType::NAL_UNIT_CODED_SLICE_TSA_R:
                case NALUnitType::NAL_UNIT_CODED_SLICE_STSA_N:
                case NALUnitType::NAL_UNIT_CODED_SLICE_STSA_R:
                case NALUnitType::NAL_UNIT_CODED_SLICE_BLA_W_LP:
                case NALUnitType::NAL_UNIT_CODED_SLICE_BLA_W_RADL:
                case NALUnitType::NAL_UNIT_CODED_SLICE_BLA_N_LP:
                case NALUnitType::NAL_UNIT_CODED_SLICE_IDR_W_RADL:
                case NALUnitType::NAL_UNIT_CODED_SLICE_IDR_N_LP:
                case NALUnitType::NAL_UNIT_CODED_SLICE_CRA:
                case NALUnitType::NAL_UNIT_CODED_SLICE_RADL_N:
                case NALUnitType::NAL_UNIT_CODED_SLICE_RADL_R:
                case NALUnitType::NAL_UNIT_CODED_SLICE_RASL_N:
                case NALUnitType::NAL_UNIT_CODED_SLICE_RASL_R:
                {
                    Slice slice;
                    bool result = parseSlice(bitstream, slice, nalUnit.type, parserContext.pictureParameterSets, parserContext.sequenceParameterSets);

                    if (result)
                    {
                        slices.push_back(slice);

#if 0

                        static size_t previousLength = 0;
                        static size_t previousOffset = 0;

                        size_t size = 0;
                        size_t offset = 0;

                        if (previousOffset == 0)
                        {
                            offset = 0;
                            size = nalUnit.offset + nalUnit.length;
                        }
                        else
                        {
                            offset = nalUnit.offset;
                            size = ((previousOffset + previousLength) - nalUnit.offset) + nalUnit.length;
                        }

                        previousLength = nalUnit.length;
                        previousOffset = nalUnit.offset;

                        LOG_D("POC FOUND: %d, position: %lu, size: %lu", slice.slicePicOrderCntLsb, offset, size);

#endif
                    }

                    break;
                }

                case NALUnitType::NAL_UNIT_ACCESS_UNIT_DELIMITER:
                {
                    break;
                }

                case NALUnitType::NAL_UNIT_SUFFIX_SEI:
                case NALUnitType::NAL_UNIT_PREFIX_SEI:
                {
                    break;
                }

                default:
                    assert(false);
            };
        }

        return true;
    }


    bool parseVPS(Bitstream& bitstream, VPS& vps)
    {
        // NAL unit header
        NALUnitType::Enum type = readNALUnitHeader(bitstream);
        assert(type == NALUnitType::NAL_UNIT_VPS);

        // NAL unit
        vps.vpsVideoParameterSetId = bitstream.readBits(4);

        // Reserved zero bits
        bitstream.skipBits(2);

        vps.vpsMaxLayersMinus1 = bitstream.readBits(6);
        vps.vpsMaxSubLayersMinus1 = bitstream.readBits(3);
        vps.vpsTemporalIdNestingFlag = bitstream.readBits(1);

        // Reserved zero bits
        bitstream.skipBits(16);

        parseProfileTierLevel(bitstream, vps.profileTierLevel, vps.vpsMaxSubLayersMinus1);

        vps.vpsSubLayerOrderingInfoPresentFlag = bitstream.readBits(1);

        vps.vpsMaxDecPicBufferingMinus1.resize(vps.vpsMaxSubLayersMinus1 + 1, 0);
        vps.vpsMaxNumReorderPics.resize(vps.vpsMaxSubLayersMinus1 + 1, 0);
        vps.vpsMaxLatencyIncreasePlus1.resize(vps.vpsMaxSubLayersMinus1 + 1, 0);

        for (size_t i = (vps.vpsSubLayerOrderingInfoPresentFlag ? 0 : vps.vpsMaxSubLayersMinus1); i <= vps.vpsMaxSubLayersMinus1; i++)
        {
            vps.vpsMaxDecPicBufferingMinus1[i] = bitstream.readUGolomb();
            vps.vpsMaxNumReorderPics[i] = bitstream.readUGolomb();
            vps.vpsMaxLatencyIncreasePlus1[i] = bitstream.readUGolomb();
        }

        vps.vpsMaxLayerId = bitstream.readBits(6);
        vps.vpsNumLayerSetsMinus1 = bitstream.readUGolomb();

        vps.layerIdIncludedFlag.resize(vps.vpsNumLayerSetsMinus1 + 1);

        for (size_t i = 1; i <= vps.vpsNumLayerSetsMinus1; i++)
        {
            vps.layerIdIncludedFlag[i].resize(vps.vpsMaxLayerId + 1);

            for (size_t j = 0; j <= vps.vpsMaxLayerId; j++)
            {
                vps.layerIdIncludedFlag[i][j] = bitstream.readBits(1);
            }
        }

        vps.vpsTimingInfoPresentFlag = bitstream.readBits(1);

        if (vps.vpsTimingInfoPresentFlag)
        {
            vps.vpsNumUnitsInTick = bitstream.readBits(32);
            vps.vpsTimeScale = bitstream.readBits(32);
            vps.vpsPocProportionalToTimingFlag = bitstream.readBits(1);

            if (vps.vpsPocProportionalToTimingFlag)
            {
                vps.vpsNumTicksPocDiffOneMinus1 = bitstream.readBits(1);
            }

            vps.vpsNumHrdParameters = bitstream.readUGolomb();

            if (vps.vpsNumHrdParameters > 0)
            {
                vps.hrdLayerSetIdx.resize(vps.vpsNumHrdParameters);
                vps.cprmsPresentFlag.resize(vps.vpsNumHrdParameters);

                vps.cprmsPresentFlag[0] = 1;

                for (size_t i = 0; i < vps.vpsNumHrdParameters; i++)
                {
                    vps.hrdLayerSetIdx[i] = bitstream.readUGolomb();

                    if (i > 0)
                    {
                        vps.cprmsPresentFlag[i] = bitstream.readBits(1);
                    }

                    HRDParameters hrdParameters;
                    parseHRDParameters(bitstream, hrdParameters, vps.cprmsPresentFlag[i], vps.vpsMaxSubLayersMinus1);

                    vps.hrdParameters.push_back(hrdParameters);
                }
            }
        }

        vps.vpsExtensionFlag = bitstream.readBits(1);

        return true;
    }

    bool parseSPS(Bitstream& bitstream, SPS& sps)
    {
        // NAL unit header
        NALUnitType::Enum type = readNALUnitHeader(bitstream);
        assert(type == NALUnitType::NAL_UNIT_SPS);

        // NAL unit
        sps.spsVideoParameterSetId = bitstream.readBits(4);
        sps.spsMaxSubLayersMinus1 = bitstream.readBits(3);
        sps.spsTemporalIdNestingFlag = bitstream.readBits(1);

        parseProfileTierLevel(bitstream, sps.profileTierLevel, sps.spsMaxSubLayersMinus1);

        sps.spsSeqParameterSetId = bitstream.readUGolomb();
        sps.chromaFormatIdc = bitstream.readUGolomb();

        if (sps.chromaFormatIdc == 3)
        {
            sps.separateColourPlaneFlag = bitstream.readBits(1);
        }
        else
        {
            sps.separateColourPlaneFlag = 0;
        }

        sps.picWidthInLumaSamples = bitstream.readUGolomb();
        sps.picHeightInLumaSamples = bitstream.readUGolomb();

        sps.conformanceWindowFlag = bitstream.readBits(1);

        if (sps.conformanceWindowFlag)
        {
            sps.confWinLeftOffset = bitstream.readUGolomb();
            sps.confWinRightOffset = bitstream.readUGolomb();
            sps.confWinTopOffset = bitstream.readUGolomb();
            sps.confWinBottomOffset = bitstream.readUGolomb();
        }

        sps.bitDepthLumaMinus8 = bitstream.readUGolomb();
        sps.bitDepthChromaMinus8 = bitstream.readUGolomb();
        sps.log2MaxPicOrderCntLsbMinus4 = bitstream.readUGolomb();
        sps.spsSubLayerOrderingInfoPresentFlag = bitstream.readBits(1);

        sps.spsMaxDecPicBufferingMinus1.resize(sps.spsMaxSubLayersMinus1 + 1, 0);
        sps.spsMaxNumReorderPics.resize(sps.spsMaxSubLayersMinus1 + 1, 0);
        sps.spsMaxLatencyIncreasePlus1.resize(sps.spsMaxSubLayersMinus1 + 1, 0);

        for (size_t i = (sps.spsSubLayerOrderingInfoPresentFlag ? 0 : sps.spsMaxSubLayersMinus1); i <= sps.spsMaxSubLayersMinus1; i++)
        {
            sps.spsMaxDecPicBufferingMinus1[i] = bitstream.readUGolomb();
            sps.spsMaxNumReorderPics[i] = bitstream.readUGolomb();
            sps.spsMaxLatencyIncreasePlus1[i] = bitstream.readUGolomb();
        }

        sps.log2MinLumaCodingBlockSizeMinus3 = bitstream.readUGolomb();
        sps.log2DiffMaxMinLumaCodingBlockSize = bitstream.readUGolomb();
        sps.log2MinTransformBlockSizeMinus2 = bitstream.readUGolomb();
        sps.log2DiffMaxMinTransformBlockSize = bitstream.readUGolomb();

        sps.maxTransformHierarchyDepthInter = bitstream.readUGolomb();
        sps.maxTransformHierarchyDepthIntra = bitstream.readUGolomb();

        sps.scalingListEnabledFlag = bitstream.readBits(1);

        if (sps.scalingListEnabledFlag)
        {
            sps.spsScalingListDataPresentFlag = bitstream.readBits(1);

            if (sps.spsScalingListDataPresentFlag)
            {
                parseScalingListData(bitstream, sps.scalingListData);
            }
        }

        sps.ampEnabledFlag = bitstream.readBits(1);
        sps.sampleAdaptiveOffsetEnabledFlag = bitstream.readBits(1);
        sps.pcmEnabledFlag = bitstream.readBits(1);

        if (sps.pcmEnabledFlag)
        {
            sps.pcmSampleBitDepthLumaMinus1 = bitstream.readBits(4);
            sps.pcmSampleBitDepthChromaMinus1 = bitstream.readBits(4);
            sps.log2MinPcmLumaCodingBlockSizeMinus3 = bitstream.readUGolomb();
            sps.log2DiffMaxMinPcmLumaCodingBlockSize = bitstream.readUGolomb();
            sps.pcmLoopFilterDisabledFlag = bitstream.readBits(1);
        }

        sps.numShortTermRefPicSets = bitstream.readUGolomb();

        sps.shortTermRefPicSet.resize(sps.numShortTermRefPicSets);

        for (size_t i = 0; i < sps.numShortTermRefPicSets; i++)
        {
            parseShortTermRefPicSet(bitstream, sps, sps.shortTermRefPicSet[i], i, sps.numShortTermRefPicSets, sps.shortTermRefPicSet);
        }

        sps.longTermRefPicsPresentFlag = bitstream.readBits(1);

        if (sps.longTermRefPicsPresentFlag)
        {
            sps.numLongTermRefPicsSps = bitstream.readUGolomb();

            sps.ltRefPicPocLsbSps.resize(sps.numLongTermRefPicsSps);
            sps.usedByCurrPicLtSpsFlag.resize(sps.numLongTermRefPicsSps);

            for (size_t i = 0; i < sps.numLongTermRefPicsSps; i++)
            {
                sps.ltRefPicPocLsbSps[i] = bitstream.readBits(sps.log2MaxPicOrderCntLsbMinus4 + 4);
                sps.usedByCurrPicLtSpsFlag[i] = bitstream.readBits(1);
            }
        }

        sps.spsTemporalMvpEnabledFlag = bitstream.readBits(1);
        sps.strongIntraSmoothingEnabledFlag = bitstream.readBits(1);
        sps.vuiParametersPresentFlag = bitstream.readBits(1);

        if (sps.vuiParametersPresentFlag)
        {
            parseVUIParameters(bitstream, sps.vuiParameters, sps.spsMaxSubLayersMinus1);
        }

        sps.spsExtensionFlag = bitstream.readBits(1);

        return true;
    }

    bool parsePPS(Bitstream& bitstream, PPS& pps)
    {
        // NAL unit header
        NALUnitType::Enum type = readNALUnitHeader(bitstream);
        assert(type == NALUnitType::NAL_UNIT_PPS);

        // NAL unit
        // TODO

        return true;
    }

    bool parseShortTermRefPicSet(Bitstream& bitstream, SPS& sps, ShortTermRefPicSet& strpset, size_t stRpsIdx, size_t numShortTermRefPicSets, const std::vector<ShortTermRefPicSet>& refPicSets)
    {
        if (stRpsIdx)
        {
            strpset.interRefPicSetPredictionFlag = bitstream.readBits(1);
        }

        if (strpset.interRefPicSetPredictionFlag)
        {
            if (stRpsIdx == numShortTermRefPicSets)
            {
                strpset.deltaIdxMinus1 = bitstream.readUGolomb();
            }

            strpset.deltaRpsSign = bitstream.readBits(1);
            strpset.absDeltaRpsMinus1 = bitstream.readUGolomb();

            size_t refRpsIdx = stRpsIdx - (strpset.deltaIdxMinus1 + 1);
            size_t numDeltaPocs = 0;

            if (refPicSets[refRpsIdx].interRefPicSetPredictionFlag)
            {
                for (size_t i = 0; i < refPicSets[refRpsIdx].usedByCurrPicFlag.size(); i++)
                {
                    if (refPicSets[refRpsIdx].usedByCurrPicFlag[i] || refPicSets[refRpsIdx].useDeltaFlag[i])
                    {
                        numDeltaPocs++;
                    }
                }
            }
            else
            {
                numDeltaPocs = refPicSets[refRpsIdx].numNegativePics + refPicSets[refRpsIdx].numPositivePics;
            }

            strpset.usedByCurrPicFlag.resize(numDeltaPocs + 1);
            strpset.useDeltaFlag.resize(numDeltaPocs + 1, 1);

            for (size_t i = 0; i <= numDeltaPocs; i++)
            {
                strpset.usedByCurrPicFlag[i] = bitstream.readBits(1);

                if (!strpset.usedByCurrPicFlag[i])
                {
                    strpset.useDeltaFlag[i] = bitstream.readBits(1);
                }
            }
        }
        else
        {
            strpset.numNegativePics = bitstream.readUGolomb();
            strpset.numPositivePics = bitstream.readUGolomb();

            if (strpset.numNegativePics > sps.spsMaxDecPicBufferingMinus1[sps.spsMaxSubLayersMinus1])
            {
                assert("num_negative_pics > sps_max_dec_pic_buffering_minus1");

                return false;
            }

            if (strpset.numPositivePics > sps.spsMaxDecPicBufferingMinus1[sps.spsMaxSubLayersMinus1])
            {
                assert("num_positive_pics > sps_max_dec_pic_buffering_minus1");

                return false;
            }

            strpset.deltaPocS0Minus1.resize(strpset.numNegativePics);
            strpset.usedByCurrPicS0Flag.resize(strpset.numNegativePics);

            for (size_t i = 0; i < strpset.numNegativePics; i++)
            {
                strpset.deltaPocS0Minus1[i] = bitstream.readUGolomb();
                strpset.usedByCurrPicS0Flag[i] = bitstream.readBits(1);
            }

            strpset.deltaPocS1Minus1.resize(strpset.numPositivePics);
            strpset.usedByCurrPicS1Flag.resize(strpset.numPositivePics);

            for (size_t i = 0; i < strpset.numPositivePics; i++)
            {
                strpset.deltaPocS1Minus1[i] = bitstream.readUGolomb();
                strpset.usedByCurrPicS1Flag[i] = bitstream.readBits(1);
            }
        }

        return true;
    }

    bool parseVUIParameters(Bitstream& bitstream, VUIParameters& vui, size_t maxNumSubLayersMinus1)
    {
        vui.aspectRatioInfoPresentFlag = bitstream.readBits(1);

        if (vui.aspectRatioInfoPresentFlag)
        {
            vui.aspectRatioIdc = bitstream.readBits(8);

            if (vui.aspectRatioIdc == 255) // Parse Extended SAR
            {
                vui.sarWidth = bitstream.readBits(16);
                vui.sarHeight = bitstream.readBits(16);
            }
        }

        vui.overscanInfoPresentFlag = bitstream.readBits(1);

        if (vui.overscanInfoPresentFlag)
        {
            vui.overscanAppropriateFlag = bitstream.readBits(1);
        }

        vui.videoFormat = 5;
        vui.videoFullRangeFlag = 0;
        vui.colourPrimaries = 2;
        vui.transferCharacteristics = 2;
        vui.matrixCoeffs = 2;

        vui.videoSignalTypePresentFlag = bitstream.readBits(1);

        if (vui.videoSignalTypePresentFlag)
        {
            vui.videoFormat = bitstream.readBits(3);
            vui.videoFullRangeFlag = bitstream.readBits(1);
            vui.colourDescriptionPresentFlag = bitstream.readBits(1);

            if (vui.colourDescriptionPresentFlag)
            {
                vui.colourPrimaries = bitstream.readBits(8);
                vui.transferCharacteristics = bitstream.readBits(8);
                vui.matrixCoeffs = bitstream.readBits(8);
            }
        }

        vui.chromaLocInfoPresentFlag = bitstream.readBits(1);

        if (vui.chromaLocInfoPresentFlag)
        {
            vui.chromaSampleLocTypeTopField = bitstream.readUGolomb();
            vui.chromaSampleLocTypeBottomField = bitstream.readUGolomb();
        }

        vui.neutralChromaIndicationFlag = bitstream.readBits(1);
        vui.fieldSeqFlag = bitstream.readBits(1);
        vui.frameFieldInfoPresentFlag = bitstream.readBits(1);
        vui.defaultDisplayWindowFlag = bitstream.readBits(1);

        if (vui.defaultDisplayWindowFlag)
        {
            vui.defDispWinLeftOffset = bitstream.readUGolomb();
            vui.defDispWinRightOffset = bitstream.readUGolomb();
            vui.defDispWinTopOffset = bitstream.readUGolomb();
            vui.defDispWinBottomOffset = bitstream.readUGolomb();
        }

        vui.vuiTimingInfoPresentFlag = bitstream.readBits(1);

        if (vui.vuiTimingInfoPresentFlag)
        {
            vui.vuiNumUnitsInTick = bitstream.readBits(32);
            vui.vuiTimeScale = bitstream.readBits(32);
            vui.vuiPocProportionalToTimingFlag = bitstream.readBits(1);

            if (vui.vuiPocProportionalToTimingFlag)
            {
                vui.vuiNumTicksPocDiffOneMinus1 = bitstream.readUGolomb();
            }

            vui.vuiHrdParametersPresentFlag = bitstream.readBits(1);

            if (vui.vuiHrdParametersPresentFlag)
            {
                parseHRDParameters(bitstream, vui.hrdParameters, 1, maxNumSubLayersMinus1);
            }
        }

        vui.bitstreamRestrictionFlag = bitstream.readBits(1);

        if (vui.bitstreamRestrictionFlag)
        {
            vui.tilesFixedStructureFlag = bitstream.readBits(1);
            vui.motionVectorsOverPicBoundariesFlag = bitstream.readBits(1);
            vui.restrictedRefPicListsFlag = bitstream.readBits(1);

            vui.minSpatialSegmentationIdc = bitstream.readUGolomb();
            vui.maxBytesPerPicDenom = bitstream.readUGolomb();
            vui.maxBitsPerMinCuDenom = bitstream.readUGolomb();
            vui.log2MaxMvLengthHorizontal = bitstream.readUGolomb();
            vui.log2MaxMvLengthVertical = bitstream.readUGolomb();
        }

        return true;
    }

    bool parseHRDParameters(Bitstream& bitstream, HRDParameters& hrd, uint8_t commonInfPresentFlag, size_t maxNumSubLayersMinus1)
    {
        if (commonInfPresentFlag)
        {
            hrd.nalHrdParametersPresentFlag = bitstream.readBits(1);
            hrd.vclHrdParametersPresentFlag = bitstream.readBits(1);

            if (hrd.nalHrdParametersPresentFlag || hrd.vclHrdParametersPresentFlag)
            {
                hrd.subPicHrdParamsPresentFlag = bitstream.readBits(1);

                if (hrd.subPicHrdParamsPresentFlag)
                {
                    hrd.tickDivisorMinus2 = bitstream.readBits(8);
                    hrd.duCpbRemovalDelayIncrementLengthMinus1 = bitstream.readBits(5);
                    hrd.subPicCpbParamsInPicTimingSeiFlag = bitstream.readBits(1);
                    hrd.dpbOutputDelayDuLengthMinus1 = bitstream.readBits(5);
                }

                hrd.bitRateScale = bitstream.readBits(4);
                hrd.cpbSizeScale = bitstream.readBits(4);

                if (hrd.subPicHrdParamsPresentFlag)
                {
                    hrd.cpbSizeDuScale = bitstream.readBits(4);
                }

                hrd.initialCpbRemovalDelayLengthMinus1 = bitstream.readBits(5);
                hrd.auCpbRemovalDelayLengthMinus1 = bitstream.readBits(5);
                hrd.dpbOutputDelayLengthMinus1 = bitstream.readBits(5);
            }
        }

        hrd.fixedPicRateGeneralFlag.resize(maxNumSubLayersMinus1 + 1);
        hrd.fixedPicRateWithinCvsFlag.resize(maxNumSubLayersMinus1 + 1);
        hrd.elementalDurationInTcMinus1.resize(maxNumSubLayersMinus1 + 1);
        hrd.lowDelayHrdFlag.resize(maxNumSubLayersMinus1 + 1, 0);
        hrd.cpbCntMinus1.resize(maxNumSubLayersMinus1 + 1, 0);

        if (hrd.nalHrdParametersPresentFlag)
        {
            hrd.nalSubLayerHrdParameters.resize(maxNumSubLayersMinus1 + 1);
        }

        if (hrd.vclHrdParametersPresentFlag)
        {
            hrd.vclSubLayerHrdParameters.resize(maxNumSubLayersMinus1 + 1);
        }

        for (size_t i = 0; i <= maxNumSubLayersMinus1; i++)
        {
            hrd.fixedPicRateGeneralFlag[i] = bitstream.readBits(1);

            if (hrd.fixedPicRateGeneralFlag[i])
            {
                hrd.fixedPicRateWithinCvsFlag[i] = 1;
            }

            if (!hrd.fixedPicRateGeneralFlag[i])
            {
                hrd.fixedPicRateWithinCvsFlag[i] = bitstream.readBits(1);
            }

            if (hrd.fixedPicRateWithinCvsFlag[i])
            {
                hrd.elementalDurationInTcMinus1[i] = bitstream.readUGolomb();
            }
            else
            {
                hrd.lowDelayHrdFlag[i] = bitstream.readBits(1);
            }

            if (!hrd.lowDelayHrdFlag[i])
            {
                hrd.cpbCntMinus1[i] = bitstream.readUGolomb();
            }

            if (hrd.nalHrdParametersPresentFlag)
            {
                parseSubLayerHRDParameters(bitstream, hrd.nalSubLayerHrdParameters[i], hrd.subPicHrdParamsPresentFlag, hrd.cpbCntMinus1[i]);
            }

            if (hrd.vclHrdParametersPresentFlag)
            {
                parseSubLayerHRDParameters(bitstream, hrd.vclSubLayerHrdParameters[i], hrd.subPicHrdParamsPresentFlag, hrd.cpbCntMinus1[i]);
            }
        }

        return true;
    }

    bool parseSubLayerHRDParameters(Bitstream& bitstream, SubLayerHRDParameters& slhrd, uint8_t subPicHRDParametersPresentFlag, size_t cpbCnt)
    {
        slhrd.bitRateValueMinus1.resize(cpbCnt + 1);
        slhrd.cpbSizeValueMinus1.resize(cpbCnt + 1);
        slhrd.cpbSizeDuValueMinus1.resize(cpbCnt + 1);
        slhrd.bitRateDuValueMinus1.resize(cpbCnt + 1);
        slhrd.cbrFlag.resize(cpbCnt + 1);

        for (size_t i = 0; i <= cpbCnt; i++)
        {
            slhrd.bitRateValueMinus1[i] = bitstream.readUGolomb();
            slhrd.cpbSizeValueMinus1[i] = bitstream.readUGolomb();

            if (subPicHRDParametersPresentFlag)
            {
                slhrd.cpbSizeDuValueMinus1[i] = bitstream.readUGolomb();
                slhrd.bitRateDuValueMinus1[i] = bitstream.readUGolomb();
            }

            slhrd.cbrFlag[i] = bitstream.readBits(1);
        }

        return true;
    }

    bool parseScalingListData(Bitstream& bitstream, ScalingListData& sld)
    {
        sld.scalingListPredModeFlag.resize(4);
        sld.scalingListPredMatrixIdDelta.resize(4);
        sld.scalingListDcCoefMinus8.resize(2);
        sld.scalingListDeltaCoef.resize(4);

        for (size_t sizeId = 0; sizeId < 4; sizeId++)
        {
            if (sizeId == 3)
            {
                sld.scalingListPredModeFlag[sizeId].resize(2);
                sld.scalingListPredMatrixIdDelta[sizeId].resize(2);
                sld.scalingListDcCoefMinus8[sizeId - 2].resize(2);
                sld.scalingListDeltaCoef[sizeId].resize(2);
            }
            else
            {
                sld.scalingListPredModeFlag[sizeId].resize(6);
                sld.scalingListPredMatrixIdDelta[sizeId].resize(6);
                sld.scalingListDeltaCoef[sizeId].resize(6);

                if (sizeId >= 2)
                {
                    sld.scalingListDcCoefMinus8[sizeId - 2].resize(6);
                }
            }

            for (size_t matrixId = 0; matrixId < ((sizeId == 3) ? 2 : 6); matrixId++)
            {
                sld.scalingListPredModeFlag[sizeId][matrixId] = bitstream.readBits(1);

                if (!sld.scalingListPredModeFlag[sizeId][matrixId])
                {
                    sld.scalingListPredMatrixIdDelta[sizeId][matrixId] = bitstream.readUGolomb();
                }
                else
                {
                    size_t nextCoef = 8;
                    size_t coefNum = Math::min(64, (1 << (4 + (sizeId << 1))));

                    if (sizeId > 1)
                    {
                        sld.scalingListDcCoefMinus8[sizeId-2][matrixId] = bitstream.readSGolomb();
                    }

                    sld.scalingListDeltaCoef[sizeId][matrixId].resize(coefNum);

                    for (size_t i = 0; i < coefNum; i++)
                    {
                        sld.scalingListDeltaCoef[sizeId][matrixId][i] = bitstream.readSGolomb();
                    }
                }
            }
        }

        return true;
    }

    bool parseProfileTierLevel(Bitstream& bitstream, ProfileTierLevel& ptl, size_t maxNumSubLayersMinus1)
    {
        ptl.generalProfileSpace = bitstream.readBits(2);
        ptl.generalTierFlag = bitstream.readBits(1);
        ptl.generalProfileIdc = bitstream.readBits(5);

        for (size_t i = 0; i < 32; i++)
        {
            ptl.generalProfileCompatibilityFlag[i] = bitstream.readBits(1);
        }

        ptl.generalProgressiveSourceFlag = bitstream.readBits(1);
        ptl.generalInterlacedSourceFlag = bitstream.readBits(1);
        ptl.generalNonPackedConstraintFlag = bitstream.readBits(1);
        ptl.generalFrameOnlyConstraintFlag = bitstream.readBits(1);

        // Reserved zero bits
        bitstream.skipBits(44);

        ptl.generalLevelIdc = bitstream.readBits(8);

        ptl.subLayerProfilePresentFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerLevelPresentFlag.resize(maxNumSubLayersMinus1);

        for (size_t i = 0; i < maxNumSubLayersMinus1; i++)
        {
            ptl.subLayerProfilePresentFlag[i] = bitstream.readBits(1);
            ptl.subLayerLevelPresentFlag[i] = bitstream.readBits(1);
        }

        if (maxNumSubLayersMinus1 > 0)
        {
            for (size_t i = maxNumSubLayersMinus1; i < 8; i++)
            {
                bitstream.skipBits(2);
            }
        }

        ptl.subLayerProfileSpace.resize(maxNumSubLayersMinus1);
        ptl.subLayerTierFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerProfileIdc.resize(maxNumSubLayersMinus1);
        ptl.subLayerProfileCompatibilityFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerProgressiveSourceFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerInterlacedSourceFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerNonPackedConstraintFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerFrameOnlyConstraintFlag.resize(maxNumSubLayersMinus1);
        ptl.subLayerLevelIdc.resize(maxNumSubLayersMinus1);

        for (size_t i = 0; i < maxNumSubLayersMinus1; i++)
        {
            if (ptl.subLayerProfilePresentFlag[i])
            {
                ptl.subLayerProfileSpace[i] = bitstream.readBits(2);
                ptl.subLayerTierFlag[i] = bitstream.readBits(1);
                ptl.subLayerProfileIdc[i] = bitstream.readBits(5);

                ptl.subLayerProfileCompatibilityFlag[i].resize(32);

                for (size_t j = 0; j < 32; j++)
                {
                    ptl.subLayerProfileCompatibilityFlag[i][j] = bitstream.readBits(1);
                }

                ptl.subLayerProgressiveSourceFlag[i] = bitstream.readBits(1);
                ptl.subLayerInterlacedSourceFlag[i] = bitstream.readBits(1);
                ptl.subLayerNonPackedConstraintFlag[i] = bitstream.readBits(1);
                ptl.subLayerFrameOnlyConstraintFlag[i] = bitstream.readBits(1);

                // Reserved zero bits
                bitstream.skipBits(44);
            }

            if (ptl.subLayerLevelPresentFlag[i])
            {
                ptl.subLayerLevelIdc[i] = bitstream.readBits(8);
            }
            else
            {
                ptl.subLayerLevelIdc[i] = 1;
            }
        }

        return true;
    }

    bool parseSlice(Bitstream& bitstream, Slice& slice, NALUnitType::Enum nalUnitType, std::vector<PPS>& picture_parameter_sets, std::vector<SPS>& sequence_parameter_sets)
    {
        NALUnitType::Enum type = readNALUnitHeader(bitstream);
        assert(type == nalUnitType);

        // Read slice header
        slice.firstSliceSegmentInPicFlag = bitstream.readBits(1);

        if (nalUnitType >= NALUnitType::NAL_UNIT_CODED_SLICE_BLA_W_LP && nalUnitType <= NALUnitType::NAL_UNIT_RESERVED_IRAP_VCL23)
        {
            slice.noOutputOfPriorPicsFlag = bitstream.readBits(1);
        }

        slice.slicePicParameterSetId = bitstream.readUGolomb();

        PPS& pps = picture_parameter_sets.at(slice.slicePicParameterSetId);
        SPS& sps = sequence_parameter_sets.at(pps.ppsSeqParameterSetId);

        if (!slice.firstSliceSegmentInPicFlag)
        {
            if (pps.dependentSliceSegmentsEnabledFlag)
            {
                slice.dependentSliceSegmentFlag = bitstream.readBits(1);
            }

            const uint32_t minCbLog2SizeY = sps.log2MinLumaCodingBlockSizeMinus3 + 3;
            const uint32_t ctbLog2SizeY = minCbLog2SizeY + sps.log2DiffMaxMinLumaCodingBlockSize;
            const uint32_t ctbSizeY = 1 << ctbLog2SizeY;
            const uint32_t picWidthInCtbsY = (sps.picWidthInLumaSamples + ctbSizeY - 1) / ctbSizeY;
            const uint32_t picHeightInCtbsY = (sps.picHeightInLumaSamples + ctbSizeY - 1) / ctbSizeY;
            const uint32_t picSizeInCtbsY = picWidthInCtbsY * picHeightInCtbsY;

            size_t sliceAddrLength = bitstream.bitsNeeded(picSizeInCtbsY);

            if (sliceAddrLength)
            {
                slice.sliceSegmentAddress = bitstream.readBits(sliceAddrLength);
            }
        }

        if (!slice.dependentSliceSegmentFlag)
        {
            if (pps.numExtraSliceHeaderBits)
            {
                bitstream.readBits(pps.numExtraSliceHeaderBits);
            }

            slice.sliceType = bitstream.readUGolomb();

            if (slice.sliceType != SliceType::B_SLICE && slice.sliceType != SliceType::P_SLICE && slice.sliceType != SliceType::I_SLICE)
            {
                return false;
            }

            if (pps.outputFlagPresentFlag)
            {
                slice.picOutputFlag = bitstream.readBits(1);
            }

            if (sps.separateColourPlaneFlag)
            {
                slice.colourPlaneId = bitstream.readBits(2);
            }

            if (nalUnitType != NALUnitType::NAL_UNIT_CODED_SLICE_IDR_W_RADL && nalUnitType != NALUnitType::NAL_UNIT_CODED_SLICE_IDR_N_LP)
            {
                slice.slicePicOrderCntLsb = bitstream.readBits(sps.log2MaxPicOrderCntLsbMinus4 + 4);
                slice.shortTermRefPicSetSpsFlag = bitstream.readBits();
            }
        }

        // TODO: Parse rest of the header

        // TODO: Parse slice payload

        return true;
    }

    const char* profileName(uint32_t profile)
    {
        switch(profile)
        {
            case 1:     return "Main";
            case 2:     return "Main 10";
            case 3:     return "Main Still Picture";

            default:    return "Unknown";
        }
    }

    const char* tierName(uint32_t tier)
    {
        switch(tier)
        {
            case 1:     return "Main";
            case 2:     return "High";

            default:    return "Unknown";
        }
    }

    void convertToLengthPrefixed(uint8_t* data, size_t bytes, std::vector<uint8_t>& output)
    {
        std::vector<NALUnit> nalUnits;
        HEVC::readNALUnits(data, bytes, nalUnits);

        for (size_t index = 0; index < nalUnits.size(); index++)
        {
            NALUnit nalUnit = nalUnits[index];

            // Write 4 byte NAL unit header containing length
            uint32_t dataLength = (uint32_t)(nalUnit.length - nalUnit.headerLength);
            uint32_t dataLengthBE = htonl(dataLength);

            uint8_t* lengthArray = (uint8_t*)&dataLengthBE;

            output.push_back(lengthArray[0]);
            output.push_back(lengthArray[1]);
            output.push_back(lengthArray[2]);
            output.push_back(lengthArray[3]);

            // Write data
            const uint8_t* buffer = data + nalUnit.offset;

            for (size_t i = nalUnit.headerLength; i < nalUnit.length; ++i)
            {
                output.push_back(buffer[i]);
            }
        }
    }

    void convertToStartCodePrefixed(uint8_t* data, size_t bytes, std::vector<uint8_t>& output)
    {
        // TODO:
    }

    void expandStartCodePrefixes(uint8_t* data, size_t bytes, std::vector<uint8_t>& output)
    {
        std::vector<NALUnit> nalUnits;
        HEVC::readNALUnits(data, bytes, nalUnits);

        // Read decoder parameters
        for (size_t index = 0; index < nalUnits.size(); index++)
        {
            NALUnit nalUnit = nalUnits.at(index);

            // Write start code
            uint8_t startCode[] = { 0x00, 0x00, 0x00, 0x01 };

            output.push_back(startCode[0]);
            output.push_back(startCode[1]);
            output.push_back(startCode[2]);
            output.push_back(startCode[3]);

            // Write data
            const uint8_t* buffer = data + nalUnit.offset;

            for (size_t i = nalUnit.headerLength; i < nalUnit.length; ++i)
            {
                output.push_back(buffer[i]);
            }
        }
    }

    bool isSlice(HEVC::NALUnitType::Enum type)
    {
        switch (type)
        {
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_TRAIL_R:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_TRAIL_N:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_TSA_N:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_TSA_R:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_STSA_N:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_STSA_R:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_BLA_W_LP:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_BLA_W_RADL:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_BLA_N_LP:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_IDR_W_RADL:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_IDR_N_LP:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_CRA:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_RADL_N:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_RADL_R:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_RASL_N:
            case HEVC::NALUnitType::NAL_UNIT_CODED_SLICE_RASL_R:
                return true;

            default:
                return false;
        }
    }

    size_t findFrameEnd(size_t startIndex, std::vector<NALUnit> nalUnits)
    {
        for (size_t i = startIndex; i < nalUnits.size(); i++)
        {
            HEVC::NALUnit currentNalUnit = nalUnits.at(i);

            if (isSlice(currentNalUnit.type))
            {
                if ((i + 1) < nalUnits.size())
                {
                    HEVC::NALUnit nextNalUnit = nalUnits.at(i + 1);

                    if (nextNalUnit.type == HEVC::NALUnitType::NAL_UNIT_SUFFIX_SEI)
                    {
                        return (i + 1);
                    }
                }

                return i;
            }
        }

        return startIndex;
    }

    void parseDecoderParameters(Bitstream& bitstream, DecoderParameters& decoderParameters)
    {
        uint8_t* data = bitstream.data();
        size_t bytes = bitstream.length();

        parseDecoderParameters(data, bytes, decoderParameters);
    }

    void parseDecoderParameters(uint8_t* data, size_t bytes, DecoderParameters& decoderParameters)
    {
        std::vector<HEVC::NALUnit> nalUnits;
        HEVC::readNALUnits(data, bytes, nalUnits);

        if (nalUnits.size() >= 3)
        {
            size_t startIndex = 0;

            const HEVC::NALUnit& startNalUnit = nalUnits.at(startIndex);

            size_t endIndex = HEVC::findFrameEnd(startIndex, nalUnits) - 1;
            const HEVC::NALUnit& endNalUnit = nalUnits.at(endIndex);

            size_t offset = startNalUnit.offset;
            size_t length = (endNalUnit.offset - startNalUnit.offset) + endNalUnit.length;

            std::vector<uint8_t> tmp;
            tmp.resize(length);

            memcpy(tmp.data(), data + offset, length);

            decoderParameters.data = tmp;
        }

        // Read decoder parameters
        for (size_t index = 0; index < nalUnits.size(); index++)
        {
            HEVC::NALUnit nalUnit = nalUnits.at(index);

            // Parse decoder parameters
            if (nalUnit.type == HEVC::NALUnitType::NAL_UNIT_VPS ||
                nalUnit.type == HEVC::NALUnitType::NAL_UNIT_SPS ||
                nalUnit.type == HEVC::NALUnitType::NAL_UNIT_PPS)
            {
                std::vector<uint8_t> tmp;
                const uint8_t* buffer = data + nalUnit.offset;

                // Write start code
                uint8_t startCode[] = { 0x00, 0x00, 0x00, 0x01 };

                tmp.push_back(startCode[0]);
                tmp.push_back(startCode[1]);
                tmp.push_back(startCode[2]);
                tmp.push_back(startCode[3]);

                for (size_t i = nalUnit.headerLength; i < nalUnit.length; ++i)
                {
                    tmp.push_back(buffer[i]);
                }

                if (nalUnit.type == HEVC::NALUnitType::NAL_UNIT_VPS)
                {
                    decoderParameters.vps = tmp;
                }
                else if (nalUnit.type == HEVC::NALUnitType::NAL_UNIT_SPS)
                {
                    decoderParameters.sps = tmp;
                }
                else if (nalUnit.type == HEVC::NALUnitType::NAL_UNIT_PPS)
                {
                    decoderParameters.pps = tmp;
                }
            }
        }
    }
}

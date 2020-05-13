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

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <assert.h>

#if PLATFORM_IOS || PLATFORM_MACOS || PLATFORM_ANDROID

#include <arpa/inet.h>

#elif PLATFORM_WINDOWS

#include <winsock.h>

#endif

#include <vector>

#include "HEVCBitstream.h"

namespace HEVC
{
    struct DecoderParameters
    {
        std::vector<uint8_t> vps;
        std::vector<uint8_t> sps;
        std::vector<uint8_t> pps;

        std::vector<uint8_t> data;
    };

    struct NALUnitType
    {
        enum Enum
        {
            NAL_UNIT_CODED_SLICE_TRAIL_N = 0,
            NAL_UNIT_CODED_SLICE_TRAIL_R = 1,

            NAL_UNIT_CODED_SLICE_TSA_N = 2,
            NAL_UNIT_CODED_SLICE_TSA_R = 3,

            NAL_UNIT_CODED_SLICE_STSA_N = 4,
            NAL_UNIT_CODED_SLICE_STSA_R = 5,

            NAL_UNIT_CODED_SLICE_RADL_N = 6,
            NAL_UNIT_CODED_SLICE_RADL_R = 7,

            NAL_UNIT_CODED_SLICE_RASL_N = 8,
            NAL_UNIT_CODED_SLICE_RASL_R = 9,

            NAL_UNIT_RESERVED_VCL_N10 = 10,
            NAL_UNIT_RESERVED_VCL_R11 = 11,
            NAL_UNIT_RESERVED_VCL_N12 = 12,
            NAL_UNIT_RESERVED_VCL_R13 = 13,
            NAL_UNIT_RESERVED_VCL_N14 = 14,
            NAL_UNIT_RESERVED_VCL_R15 = 15,

            NAL_UNIT_CODED_SLICE_BLA_W_LP = 16,
            NAL_UNIT_CODED_SLICE_BLA_W_RADL = 17,
            NAL_UNIT_CODED_SLICE_BLA_N_LP = 18,
            NAL_UNIT_CODED_SLICE_IDR_W_RADL = 19,
            NAL_UNIT_CODED_SLICE_IDR_N_LP = 20,
            NAL_UNIT_CODED_SLICE_CRA = 21,
            
            NAL_UNIT_RESERVED_IRAP_VCL22 = 22,
            NAL_UNIT_RESERVED_IRAP_VCL23 = 23,

            NAL_UNIT_RESERVED_VCL24 = 24,
            NAL_UNIT_RESERVED_VCL25 = 25,
            NAL_UNIT_RESERVED_VCL26 = 26,
            NAL_UNIT_RESERVED_VCL27 = 27,
            NAL_UNIT_RESERVED_VCL28 = 28,
            NAL_UNIT_RESERVED_VCL29 = 29,
            NAL_UNIT_RESERVED_VCL30 = 30,
            NAL_UNIT_RESERVED_VCL31 = 31,

            NAL_UNIT_VPS = 32,
            NAL_UNIT_SPS = 33,
            NAL_UNIT_PPS = 34,
            NAL_UNIT_ACCESS_UNIT_DELIMITER = 35,
            NAL_UNIT_EOS = 36,
            NAL_UNIT_EOB = 37,
            NAL_UNIT_FILLER_DATA = 38,
            NAL_UNIT_PREFIX_SEI = 39,
            NAL_UNIT_SUFFIX_SEI = 40,

            NAL_UNIT_RESERVED_NVCL41 = 41,
            NAL_UNIT_RESERVED_NVCL42 = 42,
            NAL_UNIT_RESERVED_NVCL43 = 43,
            NAL_UNIT_RESERVED_NVCL44 = 44,
            NAL_UNIT_RESERVED_NVCL45 = 45,
            NAL_UNIT_RESERVED_NVCL46 = 46,
            NAL_UNIT_RESERVED_NVCL47 = 47,

            NAL_UNIT_UNSPECIFIED_48 = 48,
            NAL_UNIT_UNSPECIFIED_49 = 49,
            NAL_UNIT_UNSPECIFIED_50 = 50,
            NAL_UNIT_UNSPECIFIED_51 = 51,
            NAL_UNIT_UNSPECIFIED_52 = 52,
            NAL_UNIT_UNSPECIFIED_53 = 53,
            NAL_UNIT_UNSPECIFIED_54 = 54,
            NAL_UNIT_UNSPECIFIED_55 = 55,
            NAL_UNIT_UNSPECIFIED_56 = 56,
            NAL_UNIT_UNSPECIFIED_57 = 57,
            NAL_UNIT_UNSPECIFIED_58 = 58,
            NAL_UNIT_UNSPECIFIED_59 = 59,
            NAL_UNIT_UNSPECIFIED_60 = 60,
            NAL_UNIT_UNSPECIFIED_61 = 61,
            NAL_UNIT_UNSPECIFIED_62 = 62,
            NAL_UNIT_UNSPECIFIED_63 = 63,

            NAL_UNIT_INVALID = 64,
        };

        static const char* toString(Enum value)
        {
            switch (value)
            {
                case NAL_UNIT_CODED_SLICE_TRAIL_N:      return "NAL_UNIT_CODED_SLICE_TRAIL_N";
                case NAL_UNIT_CODED_SLICE_TRAIL_R:      return "NAL_UNIT_CODED_SLICE_TRAIL_R";

                case NAL_UNIT_CODED_SLICE_TSA_N:        return "NAL_UNIT_CODED_SLICE_TSA_N";
                case NAL_UNIT_CODED_SLICE_TSA_R:        return "NAL_UNIT_CODED_SLICE_TSA_R";

                case NAL_UNIT_CODED_SLICE_STSA_N:       return "NAL_UNIT_CODED_SLICE_STSA_N";
                case NAL_UNIT_CODED_SLICE_STSA_R:       return "NAL_UNIT_CODED_SLICE_STSA_R";

                case NAL_UNIT_CODED_SLICE_RADL_N:       return "NAL_UNIT_CODED_SLICE_RADL_N";
                case NAL_UNIT_CODED_SLICE_RADL_R:       return "NAL_UNIT_CODED_SLICE_RADL_R";

                case NAL_UNIT_CODED_SLICE_RASL_N:       return "NAL_UNIT_CODED_SLICE_RASL_N";
                case NAL_UNIT_CODED_SLICE_RASL_R:       return "NAL_UNIT_CODED_SLICE_RASL_R";

                case NAL_UNIT_RESERVED_VCL_N10:         return "NAL_UNIT_RESERVED_VCL_N10";
                case NAL_UNIT_RESERVED_VCL_R11:         return "NAL_UNIT_RESERVED_VCL_R11";
                case NAL_UNIT_RESERVED_VCL_N12:         return "NAL_UNIT_RESERVED_VCL_N12";
                case NAL_UNIT_RESERVED_VCL_R13:         return "NAL_UNIT_RESERVED_VCL_R13";
                case NAL_UNIT_RESERVED_VCL_N14:         return "NAL_UNIT_RESERVED_VCL_N14";
                case NAL_UNIT_RESERVED_VCL_R15:         return "NAL_UNIT_RESERVED_VCL_R15";

                case NAL_UNIT_CODED_SLICE_BLA_W_LP:     return "NAL_UNIT_CODED_SLICE_BLA_W_LP";
                case NAL_UNIT_CODED_SLICE_BLA_W_RADL:   return "NAL_UNIT_CODED_SLICE_BLA_W_RADL";
                case NAL_UNIT_CODED_SLICE_BLA_N_LP:     return "NAL_UNIT_CODED_SLICE_BLA_N_LP";
                case NAL_UNIT_CODED_SLICE_IDR_W_RADL:   return "NAL_UNIT_CODED_SLICE_IDR_W_RADL";
                case NAL_UNIT_CODED_SLICE_IDR_N_LP:     return "NAL_UNIT_CODED_SLICE_IDR_N_LP";
                case NAL_UNIT_CODED_SLICE_CRA:          return "NAL_UNIT_CODED_SLICE_CRA";
                case NAL_UNIT_RESERVED_IRAP_VCL22:      return "NAL_UNIT_RESERVED_IRAP_VCL22";
                case NAL_UNIT_RESERVED_IRAP_VCL23:      return "NAL_UNIT_RESERVED_IRAP_VCL23";

                case NAL_UNIT_RESERVED_VCL24:           return "NAL_UNIT_RESERVED_VCL24";
                case NAL_UNIT_RESERVED_VCL25:           return "NAL_UNIT_RESERVED_VCL25";
                case NAL_UNIT_RESERVED_VCL26:           return "NAL_UNIT_RESERVED_VCL26";
                case NAL_UNIT_RESERVED_VCL27:           return "NAL_UNIT_RESERVED_VCL27";
                case NAL_UNIT_RESERVED_VCL28:           return "NAL_UNIT_RESERVED_VCL28";
                case NAL_UNIT_RESERVED_VCL29:           return "NAL_UNIT_RESERVED_VCL29";
                case NAL_UNIT_RESERVED_VCL30:           return "NAL_UNIT_RESERVED_VCL30";
                case NAL_UNIT_RESERVED_VCL31:           return "NAL_UNIT_RESERVED_VCL31";

                case NAL_UNIT_VPS:                      return "NAL_UNIT_VPS";
                case NAL_UNIT_SPS:                      return "NAL_UNIT_SPS";
                case NAL_UNIT_PPS:                      return "NAL_UNIT_PPS";
                case NAL_UNIT_ACCESS_UNIT_DELIMITER:    return "NAL_UNIT_ACCESS_UNIT_DELIMITER";
                case NAL_UNIT_EOS:                      return "NAL_UNIT_EOS";
                case NAL_UNIT_EOB:                      return "NAL_UNIT_EOB";
                case NAL_UNIT_FILLER_DATA:              return "NAL_UNIT_FILLER_DATA";
                case NAL_UNIT_PREFIX_SEI:               return "NAL_UNIT_PREFIX_SEI";
                case NAL_UNIT_SUFFIX_SEI:               return "NAL_UNIT_SUFFIX_SEI";

                case NAL_UNIT_RESERVED_NVCL41:          return "NAL_UNIT_RESERVED_NVCL41";
                case NAL_UNIT_RESERVED_NVCL42:          return "NAL_UNIT_RESERVED_NVCL42";
                case NAL_UNIT_RESERVED_NVCL43:          return "NAL_UNIT_RESERVED_NVCL43";
                case NAL_UNIT_RESERVED_NVCL44:          return "NAL_UNIT_RESERVED_NVCL44";
                case NAL_UNIT_RESERVED_NVCL45:          return "NAL_UNIT_RESERVED_NVCL45";
                case NAL_UNIT_RESERVED_NVCL46:          return "NAL_UNIT_RESERVED_NVCL46";
                case NAL_UNIT_RESERVED_NVCL47:          return "NAL_UNIT_RESERVED_NVCL47";

                case NAL_UNIT_UNSPECIFIED_48:           return "NAL_UNIT_UNSPECIFIED_48";
                case NAL_UNIT_UNSPECIFIED_49:           return "NAL_UNIT_UNSPECIFIED_49";
                case NAL_UNIT_UNSPECIFIED_50:           return "NAL_UNIT_UNSPECIFIED_50";
                case NAL_UNIT_UNSPECIFIED_51:           return "NAL_UNIT_UNSPECIFIED_51";
                case NAL_UNIT_UNSPECIFIED_52:           return "NAL_UNIT_UNSPECIFIED_52";
                case NAL_UNIT_UNSPECIFIED_53:           return "NAL_UNIT_UNSPECIFIED_53";
                case NAL_UNIT_UNSPECIFIED_54:           return "NAL_UNIT_UNSPECIFIED_54";
                case NAL_UNIT_UNSPECIFIED_55:           return "NAL_UNIT_UNSPECIFIED_55";
                case NAL_UNIT_UNSPECIFIED_56:           return "NAL_UNIT_UNSPECIFIED_56";
                case NAL_UNIT_UNSPECIFIED_57:           return "NAL_UNIT_UNSPECIFIED_57";
                case NAL_UNIT_UNSPECIFIED_58:           return "NAL_UNIT_UNSPECIFIED_58";
                case NAL_UNIT_UNSPECIFIED_59:           return "NAL_UNIT_UNSPECIFIED_59";
                case NAL_UNIT_UNSPECIFIED_60:           return "NAL_UNIT_UNSPECIFIED_60";
                case NAL_UNIT_UNSPECIFIED_61:           return "NAL_UNIT_UNSPECIFIED_61";
                case NAL_UNIT_UNSPECIFIED_62:           return "NAL_UNIT_UNSPECIFIED_62";
                case NAL_UNIT_UNSPECIFIED_63:           return "NAL_UNIT_UNSPECIFIED_63";

                case NAL_UNIT_INVALID:                  return "INVALID";

                default:
                    return NULL;
            }
        }
    };

    struct NALUnit
    {
        NALUnitType::Enum type = NALUnitType::NAL_UNIT_INVALID;
        size_t offset = 0;
        size_t length = 0;
        size_t headerLength = 0;
    };

    struct ProfileTierLevel
    {
        uint8_t generalProfileSpace = 0;
        uint8_t generalTierFlag = 0;
        uint8_t generalProfileIdc = 0;
        uint8_t generalProfileCompatibilityFlag[32] = { 0 };
        uint8_t generalProgressiveSourceFlag = 0;
        uint8_t generalInterlacedSourceFlag = 0;
        uint8_t generalNonPackedConstraintFlag = 0;
        uint8_t generalFrameOnlyConstraintFlag = 0;
        uint8_t generalLevelIdc = 0;

        std::vector<uint8_t> subLayerProfilePresentFlag;
        std::vector<uint8_t> subLayerLevelPresentFlag;
        std::vector<uint8_t> subLayerProfileSpace;
        std::vector<uint8_t> subLayerTierFlag;
        std::vector<uint8_t> subLayerProfileIdc;
        std::vector<std::vector<uint8_t>> subLayerProfileCompatibilityFlag;
        std::vector<uint8_t> subLayerProgressiveSourceFlag;
        std::vector<uint8_t> subLayerInterlacedSourceFlag;
        std::vector<uint8_t> subLayerNonPackedConstraintFlag;
        std::vector<uint8_t> subLayerFrameOnlyConstraintFlag;
        std::vector<uint8_t> subLayerLevelIdc;
    };

    struct SubLayerHRDParameters
    {
        std::vector<uint32_t> bitRateValueMinus1;
        std::vector<uint32_t> cpbSizeValueMinus1;
        std::vector<uint32_t> cpbSizeDuValueMinus1;
        std::vector<uint32_t> bitRateDuValueMinus1;
        std::vector<uint8_t> cbrFlag;
    };

    struct HRDParameters
    {
        uint8_t nalHrdParametersPresentFlag = 0;
        uint8_t vclHrdParametersPresentFlag = 0;
        uint8_t subPicHrdParamsPresentFlag = 0;
        uint8_t tickDivisorMinus2 = 0;
        uint8_t duCpbRemovalDelayIncrementLengthMinus1 = 0;
        uint8_t subPicCpbParamsInPicTimingSeiFlag = 0;
        uint8_t dpbOutputDelayDuLengthMinus1 = 0;
        uint8_t bitRateScale = 0;
        uint8_t cpbSizeScale = 0;
        uint8_t cpbSizeDuScale = 0;
        uint8_t initialCpbRemovalDelayLengthMinus1 = 0;
        uint8_t auCpbRemovalDelayLengthMinus1 = 0;
        uint8_t dpbOutputDelayLengthMinus1 = 0;

        std::vector<uint8_t> fixedPicRateGeneralFlag;
        std::vector<uint8_t> fixedPicRateWithinCvsFlag;
        std::vector<uint32_t> elementalDurationInTcMinus1;
        std::vector<uint8_t> lowDelayHrdFlag;
        std::vector<uint32_t> cpbCntMinus1;

        std::vector<SubLayerHRDParameters> nalSubLayerHrdParameters;
        std::vector<SubLayerHRDParameters> vclSubLayerHrdParameters;
    };

    struct VUIParameters
    {
        uint8_t aspectRatioInfoPresentFlag = 0;
        uint8_t aspectRatioIdc = 0;
        uint16_t sarWidth = 0;
        uint16_t sarHeight = 0;
        uint8_t overscanInfoPresentFlag = 0;
        uint8_t overscanAppropriateFlag = 0;
        uint8_t videoSignalTypePresentFlag = 0;
        uint8_t videoFormat = 0;
        uint8_t videoFullRangeFlag = 0;
        uint8_t colourDescriptionPresentFlag = 0;
        uint8_t colourPrimaries = 0;
        uint8_t transferCharacteristics = 0;
        uint8_t matrixCoeffs = 0;
        uint8_t chromaLocInfoPresentFlag = 0;
        uint32_t chromaSampleLocTypeTopField = 0;
        uint32_t chromaSampleLocTypeBottomField = 0;
        uint8_t neutralChromaIndicationFlag = 0;
        uint8_t fieldSeqFlag = 0;
        uint8_t frameFieldInfoPresentFlag = 0;
        uint8_t defaultDisplayWindowFlag = 0;
        uint32_t defDispWinLeftOffset = 0;
        uint32_t defDispWinRightOffset = 0;
        uint32_t defDispWinTopOffset = 0;
        uint32_t defDispWinBottomOffset = 0;
        uint8_t vuiTimingInfoPresentFlag = 0;
        uint32_t vuiNumUnitsInTick = 0;
        uint32_t vuiTimeScale = 0;
        uint8_t vuiPocProportionalToTimingFlag = 0;
        uint32_t vuiNumTicksPocDiffOneMinus1 = 0;
        uint8_t vuiHrdParametersPresentFlag = 0;
        HRDParameters hrdParameters;
        uint8_t bitstreamRestrictionFlag = 0;
        uint8_t tilesFixedStructureFlag = 0;
        uint8_t motionVectorsOverPicBoundariesFlag = 0;
        uint8_t restrictedRefPicListsFlag = 0;
        uint32_t minSpatialSegmentationIdc = 0;
        uint32_t maxBytesPerPicDenom = 0;
        uint32_t maxBitsPerMinCuDenom = 0;
        uint32_t log2MaxMvLengthHorizontal = 0;
        uint32_t log2MaxMvLengthVertical = 0;
    };

    struct ScalingListData
    {
        std::vector<std::vector<uint8_t>> scalingListPredModeFlag;
        std::vector<std::vector<uint32_t>> scalingListPredMatrixIdDelta;
        std::vector<std::vector<uint32_t>> scalingListDcCoefMinus8;
        std::vector<std::vector<std::vector<uint32_t>>> scalingListDeltaCoef;
    };

    struct ShortTermRefPicSet
    {
        uint8_t interRefPicSetPredictionFlag = 0;
        uint32_t deltaIdxMinus1 = 0;
        uint8_t deltaRpsSign = 0;
        uint32_t absDeltaRpsMinus1 = 0;

        std::vector<uint8_t> usedByCurrPicFlag;
        std::vector<uint8_t> useDeltaFlag;

        uint32_t numNegativePics = 0;
        uint32_t numPositivePics = 0;

        std::vector<uint32_t> deltaPocS0Minus1;
        std::vector<uint8_t> usedByCurrPicS0Flag;
        std::vector<uint32_t> deltaPocS1Minus1;
        std::vector<uint8_t> usedByCurrPicS1Flag;
    };

    struct SliceType
    {
        enum Enum
        {
            INVALID = -1,

            B_SLICE,
            P_SLICE,
            I_SLICE,

            COUNT
        };

        static const char* toString(Enum value)
        {
            switch (value)
            {
                case B_SLICE:   return "B_SLICE";
                case P_SLICE:   return "P_SLICE";
                case I_SLICE:   return "I_SLICE";

                case INVALID:   return "INVALID";

                default:
                    return NULL;
            }
        }
    };

    struct Slice
    {
        uint8_t firstSliceSegmentInPicFlag = 0;
        uint8_t noOutputOfPriorPicsFlag = 0;
        uint32_t slicePicParameterSetId = 0;
        uint8_t dependentSliceSegmentFlag = 0;
        uint32_t sliceSegmentAddress = 0;
        std::vector<uint32_t> sliceReservedUndeterminedFlag;
        uint32_t sliceType = 0;
        uint8_t picOutputFlag = 0;
        uint8_t colourPlaneId = 0;
        uint32_t slicePicOrderCntLsb = 0;
        uint8_t shortTermRefPicSetSpsFlag = 0;

        // TODO:
    };

    struct VPS
    {
        uint8_t vpsVideoParameterSetId = 0;
        uint8_t vpsMaxLayersMinus1 = 0;
        uint8_t vpsMaxSubLayersMinus1 = 0;
        uint8_t vpsTemporalIdNestingFlag = 0;
        ProfileTierLevel profileTierLevel;
        uint8_t vpsSubLayerOrderingInfoPresentFlag;

        std::vector<uint32_t> vpsMaxDecPicBufferingMinus1;
        std::vector<uint32_t> vpsMaxNumReorderPics;
        std::vector<uint32_t> vpsMaxLatencyIncreasePlus1;

        uint8_t vpsMaxLayerId = 0;
        uint32_t vpsNumLayerSetsMinus1 = 0;

        std::vector<std::vector<uint8_t>> layerIdIncludedFlag;

        uint8_t vpsTimingInfoPresentFlag = 0;
        uint32_t vpsNumUnitsInTick = 0;
        uint32_t vpsTimeScale = 0;
        uint8_t vpsPocProportionalToTimingFlag = 0;
        uint32_t vpsNumTicksPocDiffOneMinus1 = 0;
        uint32_t vpsNumHrdParameters = 0;

        std::vector<uint32_t> hrdLayerSetIdx;
        std::vector<uint8_t> cprmsPresentFlag;
        std::vector<HRDParameters> hrdParameters;

        uint8_t vpsExtensionFlag = 0;
    };

    struct SPS
    {
        uint8_t spsVideoParameterSetId = 0;
        uint8_t spsMaxSubLayersMinus1 = 0;
        uint8_t spsTemporalIdNestingFlag = 0;
        ProfileTierLevel profileTierLevel;
        uint32_t spsSeqParameterSetId = 0;
        uint32_t chromaFormatIdc = 0;
        uint8_t separateColourPlaneFlag = 0;
        uint32_t picWidthInLumaSamples = 0;
        uint32_t picHeightInLumaSamples = 0;
        uint8_t conformanceWindowFlag = 0;
        uint32_t confWinLeftOffset = 0;
        uint32_t confWinRightOffset = 0;
        uint32_t confWinTopOffset = 0;
        uint32_t confWinBottomOffset = 0;
        uint32_t bitDepthLumaMinus8 = 0;
        uint32_t bitDepthChromaMinus8 = 0;
        uint32_t log2MaxPicOrderCntLsbMinus4 = 0;
        uint8_t spsSubLayerOrderingInfoPresentFlag = 0;

        std::vector<uint32_t> spsMaxDecPicBufferingMinus1;
        std::vector<uint32_t> spsMaxNumReorderPics;
        std::vector<uint32_t> spsMaxLatencyIncreasePlus1;

        uint32_t log2MinLumaCodingBlockSizeMinus3 = 0;
        uint32_t log2DiffMaxMinLumaCodingBlockSize = 0;
        uint32_t log2MinTransformBlockSizeMinus2 = 0;
        uint32_t log2DiffMaxMinTransformBlockSize = 0;
        uint32_t maxTransformHierarchyDepthInter = 0;
        uint32_t maxTransformHierarchyDepthIntra = 0;
        uint8_t scalingListEnabledFlag = 0;
        uint8_t spsScalingListDataPresentFlag = 0;
        ScalingListData scalingListData;
        uint8_t ampEnabledFlag = 0;
        uint8_t sampleAdaptiveOffsetEnabledFlag = 0;
        uint8_t pcmEnabledFlag = 0;
        uint8_t pcmSampleBitDepthLumaMinus1 = 0;
        uint8_t pcmSampleBitDepthChromaMinus1 = 0;
        uint32_t log2MinPcmLumaCodingBlockSizeMinus3 = 0;
        uint32_t log2DiffMaxMinPcmLumaCodingBlockSize = 0;
        uint8_t pcmLoopFilterDisabledFlag = 0;
        uint32_t numShortTermRefPicSets = 0;

        std::vector<ShortTermRefPicSet> shortTermRefPicSet;

        uint8_t longTermRefPicsPresentFlag = 0;
        uint32_t numLongTermRefPicsSps = 0;

        std::vector<uint32_t> ltRefPicPocLsbSps;
        std::vector<uint8_t> usedByCurrPicLtSpsFlag;

        uint8_t spsTemporalMvpEnabledFlag = 0;
        uint8_t strongIntraSmoothingEnabledFlag = 0;
        uint8_t vuiParametersPresentFlag = 0;
        VUIParameters vuiParameters;
        uint8_t spsExtensionFlag = 0;
    };

    struct PPS
    {
        uint32_t ppsPicParameterSetId = 0;
        uint32_t ppsSeqParameterSetId = 0;
        uint8_t dependentSliceSegmentsEnabledFlag = 0;
        uint8_t outputFlagPresentFlag = 0;
        uint8_t numExtraSliceHeaderBits = 0;
        uint8_t signDataHidingFlag = 0;
        uint8_t cabacInitPresentFlag = 0;
        uint32_t numRefIdxL0DefaultActiveMinus1 = 0;
        uint32_t numRefIdxL1DefaultActiveMinus1 = 0;
        int32_t initQpMinus26 = 0;
        uint8_t constrainedIntraPredFlag = 0;
        uint8_t transformSkipEnabledFlag = 0;
        uint8_t cuQpDeltaEnabledFlag = 0;
        uint32_t diffCuQpDeltaDepth = 0;
        int32_t ppsCbQpOffset = 0;
        int32_t ppsCrQpOffset = 0;
        uint8_t ppsSliceChromaQpOffsetsPresentFlag = 0;
        uint8_t weightedPredFlag = 0;
        uint8_t weightedBipredFlag = 0;
        uint8_t transquantBypassEnabledFlag = 0;
        uint8_t tilesEnabledFlag = 0;
        uint8_t entropyCodingSyncEnabledFlag = 0;
        uint32_t numTileColumnsMinus1 = 0;
        uint32_t numTileRowsMinus1 = 0;
        uint8_t uniformSpacingFlag = 0;

        std::vector<uint32_t> columnWidthMinus1;
        std::vector<uint32_t> rowHeightMinus1;

        uint8_t loopFilterAcrossTilesEnabledFlag = 0;
        uint8_t ppsLoopFilterAcrossSlicesEnabledFlag = 0;
        uint8_t deblockingFilterControlPresentFlag = 0;
        uint8_t deblockingFilterOverrideEnabledFlag = 0;
        uint8_t ppsDeblockingFilterDisabledFlag = 0;
        uint32_t ppsBetaOffsetDiv2 = 0;
        uint32_t ppsTcOffsetDiv2 = 0;
        uint8_t ppsScalingListDataPresentFlag = 0;
        ScalingListData scalingListData;
        uint8_t listsModificationPresentFlag = 0;
        int32_t log2ParallelMergeLevelMinus2 = 0;
        uint8_t sliceSegmentHeaderExtensionPresentFlag = 0;
        uint8_t ppsExtensionFlag = 0;
    };

    struct ParserContext
    {
        std::vector<VPS> videoParameterSets;
        std::vector<SPS> sequenceParameterSets;
        std::vector<PPS> pictureParameterSets;
    };

    NALUnitType::Enum readNALUnitHeader(Bitstream& bitstream);

    bool readNALUnits(Bitstream& bitstream, std::vector<NALUnit>& output);
    bool readNALUnits(uint8_t* data, size_t bytes, std::vector<NALUnit>& output);

    bool parseSlices(uint8_t* data, size_t bytes, std::vector<HEVC::NALUnit> nalUnits, std::vector<HEVC::Slice>& slices);

    bool parseVPS(Bitstream& bitstream, VPS& vps);
    bool parseSPS(Bitstream& bitstream, SPS& sps);
    bool parsePPS(Bitstream& bitstream, PPS& pps);

    bool parseProfileTierLevel(Bitstream& bitstream, ProfileTierLevel& ptl, size_t maxNumSubLayersMinus1);
    bool parseVUIParameters(Bitstream& bitstream, VUIParameters& vui, size_t maxNumSubLayersMinus1);
    bool parseHRDParameters(Bitstream& bitstream, HRDParameters& hrd, uint8_t commonInfPresentFlag, size_t maxNumSubLayersMinus1);
    bool parseScalingListData(Bitstream& bitstream, ScalingListData& sld);
    bool parseShortTermRefPicSet(Bitstream& bitstream, SPS& sps, ShortTermRefPicSet& strpset, size_t stRpsIdx, size_t numShortTermRefPicSets, const std::vector<ShortTermRefPicSet>& refPicSets);
    bool parseSubLayerHRDParameters(Bitstream& bitstream, SubLayerHRDParameters& slhrd, uint8_t subPicHRDParametersPresentFlag, size_t cpbCnt);
    bool parseSlice(Bitstream& bitstream, Slice& slice, NALUnitType::Enum nalUnitType, std::vector<PPS>& picture_parameter_sets, std::vector<SPS>& sequence_parameter_sets);

    const char* profileName(uint32_t profile);
    const char* tierName(uint32_t tier);

    // Convert start code prefixed (Annex B format) stream to NAL unit lenght prefixed (NAL Unit stream format) stream.
    void convertToLengthPrefixed(uint8_t* data, size_t bytes, std::vector<uint8_t>& output);

    // Convert NAL unit lenght prefixed (NAL Unit stream format) stream to start code prefixed (Annex B format) stream.
    void convertToStartCodePrefixed(uint8_t* data, size_t bytes, std::vector<uint8_t>& output);

    // Expand all start codes to 4 byte length.
    void expandStartCodePrefixes(uint8_t* data, size_t bytes, std::vector<uint8_t>& output);

    size_t findFrameEnd(size_t startIndex, std::vector<HEVC::NALUnit> nalUnits);

    void parseDecoderParameters(uint8_t* data, size_t bytes, DecoderParameters& decoderParameters);
    void parseDecoderParameters(Bitstream& bitstream, DecoderParameters& decoderParameters);
}

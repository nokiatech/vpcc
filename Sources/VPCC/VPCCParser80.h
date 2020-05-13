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

#include <string>
#include <vector>

#include "VPCC/VPCCBitstream80.h"

#include "Helpers.h"
#include "HEVC.h"

////////////////////////////////////////////////////////////////////////////////
/// Data structures
////////////////////////////////////////////////////////////////////////////////
namespace VPCC
{
    // TMC2 data types

    const uint32_t TMC2ContainerMagicNumber = 23021981;
    const uint32_t TMC2ContainerVersion     = 1;

    struct TMC2Header
    {
        uint32_t magic     = 0;
        uint32_t version   = 0;
        uint64_t totalSize = 0;
    };

    // VPCC data types

    struct ColorTransform
    {
        enum Enum
        {
            NONE         = 0,
            RGB_TO_YCBCR = 1,

            COUNT
        };
    };

    struct Axis6
    {
        enum Enum
        {
            UNDEFINED = -1,

            X_NEAR = 0,
            Y_NEAR = 1,
            Z_NEAR = 2,
            X_FAR  = 3,
            Y_FAR  = 4,
            Z_FAR  = 5,

            COUNT
        };
    };

    struct Axis3
    {
        enum Enum
        {
            UNDEFINED = -1,

            X = 0,
            Y = 1,
            Z = 2,

            COUNT
        };
    };

    struct PointType
    {
        enum Enum
        {
            UNSET = 0,

            D0,
            D1,
            DF,
            SMOOTH,
            EDD,

            COUNT
        };
    };

    struct VideoType
    {
        enum Enum
        {
            INVALID = -1,

            OCCUPANCY,
            GEOMETRY,
            GEOMETRY_D0,
            GEOMETRY_D1,
            GEOMETRY_RAW,
            TEXTURE,
            TEXTURE_T0,
            TEXTURE_T1,
            TEXTURE_RAW,

            COUNT
        };

        static const char* toString(Enum value)
        {
            switch (value)
            {
            case OCCUPANCY:
                return "OCCUPANCY";
            case GEOMETRY:
                return "GEOMETRY";
            case GEOMETRY_D0:
                return "GEOMETRY_D0";
            case GEOMETRY_D1:
                return "GEOMETRY_D1";
            case GEOMETRY_RAW:
                return "GEOMETRY_RAW";
            case TEXTURE:
                return "TEXTURE";
            case TEXTURE_T0:
                return "TEXTURE_T0";
            case TEXTURE_T1:
                return "TEXTURE_T1";
            case TEXTURE_RAW:
                return "TEXTURE_RAW";

            default:
                return "INVALID";
            }
        }
    };

    struct MetadataType
    {
        enum Enum
        {
            GOF = 0,
            FRAME,
            PATCH,

            COUNT
        };
    };

    struct PatchOrientation
    {
        enum Enum
        {
            DEFAULT = 0,
            SWAP    = 1,
            ROT180  = 2,
            ROT270  = 3,
            MIRROR  = 4,
            MROT90  = 5,
            MROT180 = 6,
            ROT90   = 7,
            MROT270 = 8,

            COUNT
        };
    };

    struct VPCCUnitType
    {
        enum Enum
        {
            VPS = 0,  // 0: Sequence parameter set
            AD,       // 1: Patch Data Group
            OVD,      // 2: Occupancy Video Data
            GVD,      // 3: Geometry Video Data
            AVD,      // 4: Attribute Video Data
            RSVD_05,  // 05: Reserved
            RSVD_06,  // 06: Reserved
            RSVD_07,  // 07: Reserved
            RSVD_08,  // 08: Reserved
            RSVD_09,  // 09: Reserved
            RSVD_10,  // 10: Reserved
            RSVD_11,  // 11: Reserved
            RSVD_12,  // 12: Reserved
            RSVD_13,  // 13: Reserved
            RSVD_14,  // 14: Reserved
            RSVD_15,  // 15: Reserved
            RSVD_16,  // 16: Reserved
            RSVD_17,  // 17: Reserved
            RSVD_18,  // 18: Reserved
            RSVD_19,  // 19: Reserved
            RSVD_20,  // 20: Reserved
            RSVD_21,  // 21: Reserved
            RSVD_22,  // 22: Reserved
            RSVD_23,  // 23: Reserved
            RSVD_24,  // 24: Reserved
            RSVD_25,  // 25: Reserved
            RSVD_26,  // 26: Reserved
            RSVD_27,  // 27: Reserved
            RSVD_28,  // 28: Reserved
            RSVD_29,  // 29: Reserved
            RSVD_30,  // 30: Reserved
            RSVD_31,  // 32: Reserved

            COUNT
        };
    };

    struct PDGUnitType
    {
        enum Enum
        {
            PSPS = 0,    // 00: Patch sequence parameter set
            PFPS,        // 01: Patch frame parameter set
            PFGPS,       // 02: Patch frame geometry parameter set
            PFAPS,       // 03: Patch frame attribute parameter set
            GPPS,        // 04: Geometry patch parameter set
            APPS,        // 05: Attribute patch parameter set
            PTGLU,       // 06: Patch tile group layer unit
            PREFIX_SEI,  // 07: Prefix SEI message
            SUFFIX_SEI,  // 08: Suffix SEI message
            RSVD_09,     // 09: Reserved
            RSVD_10,     // 10: Reserved
            RSVD_11,     // 11: Reserved
            RSVD_12,     // 12: Reserved
            RSVD_13,     // 13: Reserved
            RSVD_14,     // 14: Reserved
            RSVD_15,     // 15: Reserved
            RSVD_16,     // 16: Reserved
            RSVD_17,     // 17: Reserved
            RSVD_18,     // 18: Reserved
            RSVD_19,     // 19: Reserved
            RSVD_20,     // 20: Reserved
            RSVD_21,     // 21: Reserved
            RSVD_22,     // 22: Reserved
            RSVD_23,     // 23: Reserved
            RSVD_24,     // 24: Reserved
            RSVD_25,     // 25: Reserved
            RSVD_26,     // 26: Reserved
            RSVD_27,     // 27: Reserved
            RSVD_28,     // 28: Reserved
            RSVD_29,     // 29: Reserved
            RSVD_30,     // 30: Reserved
            RSVD_31,     // 32: Reserved

            COUNT
        };
    };

    struct SeiPayloadType
    {
        enum Enum
        {
            BUFFERING_PERIOD = 0,             //  0: bufferingPeriod
            ATLAS_FRAME_TIMING,               //  1: atlasFrameTiming
            FILLER_PAYLOAD,                   //  2: fillerPayload
            USER_DATAREGISTERED_ITUTT35,      //  3: userDataRegisteredItuTT35
            USER_DATA_UNREGISTERED,           //  4: userDataUnregistered
            RECOVERY_POINT,                   //  5: recoveryPoint
            NO_DISPLAY,                       //  6: noDisplay
            TIME_CODE,                        //  7: timeCode
            REGIONAL_NESTING,                 //  8: regionalNesting
            SEI_MANIFEST,                     //  9: seiManifest
            SEI_PREFIX_INDICATION,            // 10: seiPrefixIndication
            GEOMETRY_TRANSFORMATION_PARAMS,   // 11: geometryTransformationParams
            ATTRIBUTE_TRANSFORMATION_PARAMS,  // 12: attributeTransformationParams
            ACTIVE_SUBSTREAMS,                // 13: activeSubstreams
            COMPONENT_CODEC_MAPPING,          // 14: componentCodecMapping
            VOLUMETRIC_TILING_INFO,           // 15: volumetricTilingInfo
            PRESENTATION_INFORMATION,         // 16: presentationInformation
            SMOOTHING_PARAMETERS,             // 17: smoothingParameters
            RESERVED_SEI_MESSAGE,             // 18: reservedSeiMessage
        };
    };

    struct NalUnitType
    {
        enum Enum
        {
            TRAIL =
                0,  // 0: Coded tile group of a non-TSA, non STSA trailing atlas frame atlas_tile_group_layer_rbsp() ACL
            TSA,    // 1: Coded tile group of a TSA atlas frame, atlas_tile_group_layer_rbsp() ACL
            STSA,   // 2: Coded tile group of a STSA atlas frame, atlas_tile_group_layer_rbsp() ACL
            RADL,   // 3: Coded tile group of a RADL atlas frame, atlas_tile_group_layer_rbsp() ACL
            RASL,   // 4: Coded tile group of a RASL atlas frame, atlas_tile_group_layer_rbsp() ACL
            SKIP,   // 5: Coded tile group of a skipped atlas frame, atlas_tile_group_layer_rbsp() ACL
            RSV_ACL_6,    // 6: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_7,    // 7: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_8,    // 8: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_9,    // 9: Reserved non-IRAP ACL NAL unit types ACL
            BLA_W_LP,     // 10: Coded tile group of a BLA atlas frame, atlas_tile_group_layer_rbsp() ACL
            BLA_W_RADL,   // 11: Coded tile group of a BLA atlas frame, atlas_tile_group_layer_rbsp() ACL
            BLA_N_LP,     // 12: Coded tile group of a BLA atlas frame, atlas_tile_group_layer_rbsp() ACL
            GBLA_W_LP,    // 13: Coded tile group of a GBLA atlas frame, atlas_tile_group_layer_rbsp() ACL
            GBLA_W_RADL,  // 14: Coded tile group of a GBLA atlas frame, atlas_tile_group_layer_rbsp() ACL
            GBLA_N_LP,    // 15: Coded tile group of a GBLA atlas frame, atlas_tile_group_layer_rbsp() ACL
            IDR_W_RADL,   // 16: Coded tile group of an IDR atlas frame, atlas_tile_group_layer_rbsp() ACL
            IDR_N_LP,     // 17: Coded tile group of an IDR atlas frame, atlas_tile_group_layer_rbsp() ACL
            GIDR_W_RADL,  // 18: Coded tile group of a GIDR atlas frame, atlas_tile_group_layer_rbsp() ACL
            GIDR_N_LP,    // 19: Coded tile group of a GIDR atlas frame, atlas_tile_group_layer_rbsp() ACL
            CRA,          // 20: Coded tile group of a CRA atlas frame, atlas_tile_group_layer_rbsp() ACL
            GCRA,         // 21: Coded tile group of a GCRA atlas frame, atlas_tile_group_layer_rbsp() ACL
            IRAP_ACL_22,  // 22: Reserved IRAP ACL NAL unit types ACL
            IRAP_ACL_23,  // 23: Reserved IRAP ACL NAL unit types ACL
            RSV_ACL_24,   // 24: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_25,   // 25: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_26,   // 26: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_27,   // 27: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_28,   // 28: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_29,   // 29: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_30,   // 30: Reserved non-IRAP ACL NAL unit types ACL
            RSV_ACL_31,   // 31: Reserved non-IRAP ACL NAL unit types ACL
            ASPS,         // 32: Atlas sequence parameter set atlas_sequence_parameter_set_rbsp() non-ACL
            AFPS,         // 33: Atlas frame parameter set atlas_frame_parameter_set_rbsp() non-ACL
            AUD,          // 34: Access unit delimiter access_unit_delimiter_rbsp() non-ACL
            VPCC_AUD,     // 35: V-PCC access unit delimiter access_unit_delimiter_rbsp() non-ACL
            EOS,          // 36: End of sequence end_of_seq_rbsp() non-ACL
            EOB,          // 37! End of bitstream end_of_atlas_substream_rbsp() non-ACL
            FD,           // 38: Filler filler_data_rbsp() non-ACL
            PREFIX_SEI,   // 39: Supplemental enhancement information sei_rbsp() non-ACL
            SUFFIX_SEI,   // 40: Supplemental enhancement information sei_rbsp() non-ACL
            RSV_NACL_41,  // 42: Reserved non-ACL NAL unit types non-ACL
            RSV_NACL_42,  // 42: Reserved non-ACL NAL unit types non-ACL
            RSV_NACL_43,  // 43: Reserved non-ACL NAL unit types non-ACL
            RSV_NACL_44,  // 44: Reserved non-ACL NAL unit types non-ACL
            RSV_NACL_45,  // 45: Reserved non-ACL NAL unit types non-ACL
            RSV_NACL_46,  // 46: Reserved non-ACL NAL unit types non-ACL
            RSV_NACL_47,  // 47: Reserved non-ACL NAL unit types non-ACL
            UNSPEC_48,    // 48: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_49,    // 49: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_50,    // 50: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_51,    // 51: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_52,    // 52: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_53,    // 53: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_54,    // 54: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_55,    // 55: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_56,    // 56: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_57,    // 57: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_58,    // 58: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_59,    // 59: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_60,    // 60: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_61,    // 61: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_62,    // 62: Unspecified non-ACL NAL unit types non-ACL
            UNSPEC_63     // 63: Unspecified non-ACL NAL unit types non-ACL
        };
    };

    struct CodecID
    {
        enum Enum
        {
            HEVC = 0,

            COUNT
        };
    };

    struct PatchFrameType
    {
        enum Enum
        {
            I = 0,
            P,

            COUNT
        };
    };

    struct PatchModeI
    {
        enum Enum
        {
            INTRA = 0,
            RAW,
            EOM,
            END = 14
        };
    };

    struct PatchModeP
    {
        enum Enum
        {
            SKIP = 0,
            INTRA,
            INTER,
            MERGE,
            RAW,
            EOM,
            END = 14,
        };
    };

    struct PatchType
    {
        enum Enum
        {
            INTRA = 0,
            INTER,
            MERGE,
            SKIP,
            RAW,
            EOM,
            END,
            ERROR,
        };
    };

    struct TileGroup
    {
        enum Enum
        {
            P = 0,
            SKIP,
            I,
        };
    };

    struct Vector3
    {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct Point3
    {
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
    };

    struct Box3
    {
        Vector3 min;
        Vector3 max;
    };

    struct Color3
    {
        uint8_t x = 0;
        uint8_t y = 0;
        uint8_t z = 0;
    };

    struct Matrix3x3
    {
        float data[3][3] = { 0 };
    };

    struct VpccUnitHeader
    {
        uint8_t unitType                = 0;
        uint8_t sequenceParamterSetId   = 0;
        uint8_t atlasId                 = 0;
        uint8_t attributeIndex          = 0;
        uint8_t attributeDimensionIndex = 0;
        uint8_t mapIndex                = 0;
        bool rawVideoFlag               = false;

        size_t unitSize					= 0;
        size_t unitPos					= 0;
    };

    struct ProfileTierLevel
    {
        bool tierFlag                    = false;
        uint8_t profileCodecGroupIdc     = 0;
        uint8_t profilePccToolsetIdc     = 0;
        uint8_t profileReconctructionIdc = 0;
        uint8_t levelIdc                 = 0;
    };

    struct GeometryInformation
    {
        uint8_t geometryCodecId                     = 0;
        uint8_t geometryNominal2dBitdepthMinus1     = 10;
        bool geometryMSBAlignFlag                   = false;
        uint8_t geometry3dCoordinatesBitdepthMinus1 = 9;
        uint8_t rawGeometryCodecId                  = 0;
    };

    struct OccupancyInformation
    {
        uint8_t occupancyCodecId                      = 0;
        uint8_t lossyOccupancyMapCompressionThreshold = 0;
        uint8_t occupancyNominal2DBitdepthMinus1      = 0;
        bool occupancyMSBAlignFlag                    = 0;
    };

    struct AttributeInformation
    {
        uint8_t attributeCount = 0;
        std::vector<uint8_t> attributeTypeId;
        std::vector<uint8_t> attributeCodecId;
        std::vector<uint8_t> rawAttributeCodecId;
        std::vector<std::vector<bool>> attributeMapAbsoluteCodingEnabledFlagList;
        std::vector<uint8_t> attributeDimensionMinus1;
        std::vector<uint8_t> attributeDimensionPartitionsMinus1;
        std::vector<std::vector<uint8_t>> attributePartitionChannelsMinus1;
        std::vector<uint8_t> attributeNominal2dBitdepthMinus1;
        bool attributeMSBAlignFlag = false;

        void allocate()
        {
            attributeTypeId.resize(attributeCount, 0);
            attributeCodecId.resize(attributeCount, 0);
            rawAttributeCodecId.resize(attributeCount, 0);
            attributeDimensionMinus1.resize(attributeCount, 0);
            attributeDimensionPartitionsMinus1.resize(attributeCount, 0);
            attributeNominal2dBitdepthMinus1.resize(attributeCount, 0);
            attributePartitionChannelsMinus1.resize(attributeCount);
            attributeMapAbsoluteCodingEnabledFlagList.resize(attributeCount);
        }

        void addAttributeMapAbsoluteCodingEnabledFlag(size_t attIdx, bool value)
        {
            attributeMapAbsoluteCodingEnabledFlagList[attIdx].push_back(value);
        }

        void setAttributePartitionChannelsMinus1(uint32_t index, uint32_t j, uint8_t value)
        {
            if (j >= attributePartitionChannelsMinus1[index].size())
            {
                attributePartitionChannelsMinus1[index].resize(j + 1, 0);
            }
            
            attributePartitionChannelsMinus1[index][j] = value;
        }
    };

    // 7.3.4.1  General V-PCC Sequence parameter set syntax
    struct VpccParameterSet
    {
        ProfileTierLevel profileTierLevel;
        uint32_t vpccParameterSetId = 0;
        uint32_t atlasCountMinus1   = 0;
        std::vector<uint16_t> frameWidth;
        std::vector<uint16_t> frameHeight;
        std::vector<uint8_t> mapCountMinus1;
        std::vector<bool> multipleMapStreamsPresentFlag;
        std::vector<std::vector<bool>> mapAbsoluteCodingEnableFlag;
        std::vector<std::vector<size_t>> mapPredictorIndexDiff;
        std::vector<bool> rawPatchEnabledFlag;
        std::vector<bool> rawSeparateVideoPresentFlag;
        std::vector<GeometryInformation> geometryInformation;
        std::vector<OccupancyInformation> occupancyInformation;
        std::vector<AttributeInformation> attributeInformation;
        bool extensionPresentFlag = false;
        size_t extensionLength = 0;
        std::vector<uint8_t> extensionDataByte;

        void allocateAtlas()
        {
            frameWidth.resize(atlasCountMinus1 + 1, 1);
            frameHeight.resize(atlasCountMinus1 + 1);
            mapCountMinus1.resize(atlasCountMinus1 + 1);
            multipleMapStreamsPresentFlag.resize(atlasCountMinus1 + 1);
            mapAbsoluteCodingEnableFlag.resize(atlasCountMinus1 + 1);
            mapPredictorIndexDiff.resize(atlasCountMinus1 + 1);
            rawPatchEnabledFlag.resize(atlasCountMinus1 + 1);
            rawSeparateVideoPresentFlag.resize(atlasCountMinus1 + 1);
            geometryInformation.resize(atlasCountMinus1 + 1);
            occupancyInformation.resize(atlasCountMinus1 + 1);
            attributeInformation.resize(atlasCountMinus1 + 1);
        }

        void allocateMap(size_t atlasIndex)
        {
            mapAbsoluteCodingEnableFlag[atlasIndex].resize(mapCountMinus1[atlasIndex] + 1, 1);
            mapPredictorIndexDiff[atlasIndex].resize(mapCountMinus1[atlasIndex] + 1);
        }

        // THE NEXT PARAMETERS ARE NOT IN THE VPCC CD SYNTAX DOCUMENTS AND WILL BE REMOVE
        bool losslessGeo444 = false;
        bool losslessGeo = false;
        bool losslessTexture = false;
        size_t surfaceThickness = 0;
        uint8_t minLevel = 0;
        // THE NEXT PARAMETERS ARE NOT IN THE VPCC CD SYNTAX DOCUMENTS AND WILL BE REMOVE
    };

    // 7.3.5 NAL unit syntax
    struct NalUnit
    {
        NalUnitType::Enum nalUnitType = NalUnitType::TRAIL;
        uint8_t layerId               = 0;
        uint8_t temporalyIdPlus1      = 0;
        size_t nalUnitSize            = 0;
        std::vector<uint8_t> nalUnitData;

        void allocate()
        {
            nalUnitData.resize(nalUnitSize, 0);
        }
    };

    // 7.3.6.2 Point local reconstruction information syntax
    struct PointLocalReconstructionInformation
    {
        void allocate()
        {
            minimumDepth.resize(numberOfModesMinus1 + 1, 0);
            neighbourMinus1.resize(numberOfModesMinus1 + 1, 0);
            interpolateFlag.resize(numberOfModesMinus1 + 1, false);
            fillingFlag.resize(numberOfModesMinus1 + 1, false);
        }

        bool mapEnabledFlag         = 0;
        uint8_t numberOfModesMinus1 = 0;
        std::vector<bool> interpolateFlag;
        std::vector<bool> fillingFlag;
        std::vector<uint8_t> minimumDepth;
        std::vector<uint8_t> neighbourMinus1;
        uint8_t blockThresholdPerPatchMinus1 = 0;
    };

    // C.2 Sample stream NAL unit syntax and semantics
    struct SampleStreamNalUnit
    {
        uint8_t unitSizePrecisionBytesMinus1 = 0;
        std::vector<NalUnit> nalUnit;

        NalUnit& addNalUnit()
        {
            nalUnit.resize(nalUnit.size() + 1);
            return nalUnit.back();
        }
    };

    // F.2  VUI syntax
    // F.2.3  Sub-layer HRD parameters syntax
    struct HrdSubLayerParameters
    {
        //        HrdSubLayerParameters& operator=(const HrdSubLayerParameters&) = default;
        void allocate(size_t size)
        {
            hrdBitRateValueMinus1.resize(size, 0);
            hrdCabSizeValueMinus1.resize(size, 0);
            hrdCbrFlag.resize(size, false);
        }

        std::vector<uint32_t> hrdBitRateValueMinus1;
        std::vector<uint32_t> hrdCabSizeValueMinus1;
        std::vector<bool> hrdCbrFlag;
    };

    // F.2.2  HRD parameters syntax
    struct HrdParameters
    {
        static const uint8_t maxNumSubLayersMinus1    = 0;
        bool hrdNalParametersPresentFlag              = 0;
        bool hrdAclParametersPresentFlag              = 0;
        uint8_t hrdBitRateScale                       = 0;
        uint8_t hrdCabSizeScale                       = 0;
        uint8_t hrdInitialCabRemovalDelayLengthMinus1 = 0;
        uint8_t hrdAuCabRemovalDelayLengthMinus1      = 0;
        uint8_t hrdDabOutputDelayLengthMinus1         = 0;
        bool hrdFixedAtlasRateGeneralFlag[maxNumSubLayersMinus1 + 1];
        bool hrdFixedAtlasRateWithinCasFlag[maxNumSubLayersMinus1 + 1];
        bool hrdLowDelayFlag[maxNumSubLayersMinus1 + 1];
        uint32_t hrdElementalDurationInTcMinus1 = 0;
        uint32_t hrdCabCntMinus1                = 0;
        HrdSubLayerParameters hrdSubLayerParameters[2][maxNumSubLayersMinus1 + 1];
    };

    // F.2.1  VUI parameters syntax
    struct VUIParameters
    {
        bool vuiTimingInfoPresentFlag        = false;
        bool vuiPocProportionalToTimingFlag  = false;
        bool vuiHrdParametersPresentFlag     = false;
        uint32_t vuiNumUnitsInTick           = 1001;
        uint32_t vuiTimeScale                = 60000;
        uint32_t vuiNumTicksPocDiffOneMinus1 = 0;
        HrdParameters hrdParameters;
    };

    // 7.3.6.12  Reference list structure syntax
    struct RefListStruct
    {
        uint8_t numRefEntries = 0;

        std::vector<uint8_t> absDeltaAfocSt;
        std::vector<uint8_t> afocLsbLt;
        std::vector<bool> stRefAtlasFrameFlag;
        std::vector<bool> strpfEntrySignFlag;

        void allocate()
        {
            absDeltaAfocSt.resize(numRefEntries, 0);
            afocLsbLt.resize(numRefEntries, 0);
            stRefAtlasFrameFlag.resize(numRefEntries, false);
            strpfEntrySignFlag.resize(numRefEntries, false);
        }
    };

    // 7.3.6.1 Atlas sequence parameter set RBSP
    struct AtlasSequenceParameterSetRBSP
    {
        void allocateRefListStruct()
        {
            refListStruct.resize(numRefAtlasFrameListsInAsps);
        }
		
        void allocatePointLocalReconstructionInformation()
        {
            pointLocalReconstructionInformation.resize(mapCountMinus1 + 1);
        }
		
        void addRefListStruct(RefListStruct value)
        {
            refListStruct.push_back(value);
        }
		
        RefListStruct& addRefListStruct()
        {
            RefListStruct ref;
            refListStruct.push_back(ref);
            return refListStruct.back();
        }
		
        void addPointLocalReconstructionInformation(PointLocalReconstructionInformation value)
        {
            pointLocalReconstructionInformation.push_back(value);
        }
		
        PointLocalReconstructionInformation& addPointLocalReconstructionInformation()
        {
            PointLocalReconstructionInformation plri;
            pointLocalReconstructionInformation.push_back(plri);
            return pointLocalReconstructionInformation.back();
        }

        uint8_t altasSequenceParameterSetId        = 0;
        uint16_t frameWidth                        = 0;
        uint16_t frameHeight                       = 0;
        uint8_t log2PatchPackingBlockSize          = 0;
        uint8_t log2MaxAtlasFrameOrderCntLsbMinus4 = 4;
        uint8_t maxDecAtlasFrameBufferingMinus1    = 0;
        bool longTermRefAtlasFramesFlag            = false;
        uint8_t numRefAtlasFrameListsInAsps        = 0;
        std::vector<RefListStruct> refListStruct;
        bool useEightOrientationsFlag                 = false;
        bool degree45ProjectionPatchPresentFlag       = false;
        bool normalAxisLimitsQuantizationEnabledFlag  = true;
        bool normalAxisMaxDeltaValueEnabledFlag       = false;
        bool removeDuplicatePointEnabledFlag          = false;
        bool pixelDeinterleavingFlag                  = false;
        bool patchPrecedenceOrderFlag                 = false;
        bool patchSizeQuantizerPresentFlag            = false;
        bool enhancedOccupancyMapForDepthFlag         = false;
        bool pointLocalReconstructionEnabledFlag      = false;
        uint8_t mapCountMinus1                        = 0;
        uint8_t enhancedOccupancyMapFixBitCountMinus1 = 1;
        std::vector<PointLocalReconstructionInformation> pointLocalReconstructionInformation;
        uint8_t surfaceThicknessMinus1 = 3;
        bool vuiParametersPresentFlag  = false;
        bool extensionPresentFlag      = false;
        bool extensionDataFlag         = false;

        VUIParameters vuiParameters;
    };


    // 7.3.6.4  Atlas frame tile information syntax
    struct AtlasFrameTileInformation
    {
        bool singleTileInAtlasFrameFlag           = false;
        bool uniformTileSpacingFlag               = false;
        uint32_t numTileColumnsMinus1             = 0;
        uint32_t numTileRowsMinus1                = 0;
        uint32_t singleTilePerTileGroupFlag       = 0;
        uint32_t numTileGroupsInAtlasFrameMinus1  = 0;
        bool signalledTileGroupIdFlag             = false;
        uint32_t signalledTileGroupIdLengthMinus1 = 0;
        std::vector<uint32_t> tileColumnWidthMinus1{0};
        std::vector<uint32_t> tileRowHeightMinus1{0};
        std::vector<uint32_t> topLeftTileIdx{0};
        std::vector<uint32_t> bottomRightTileIdxDelta{0};
        std::vector<uint32_t> tileGroupId{0};
    };

    // 7.3.6.3  Atlas frame parameter set RBSP syntax
    struct AtlasFrameParameterSetRbsp
    {
        uint8_t afpsAtlasFrameParameterSetId    = 0;
        uint8_t afpsAtlasSequenceParameterSetId = 0;
        AtlasFrameTileInformation atlasFrameTileInformation;
        uint8_t afpsNumRefIdxDefaultActiveMinus1   = 0;
        uint8_t afpsAdditionalLtAfocLsbLen         = 0;
        size_t afps2dPosXBitCountMinus1            = 0;
        size_t afps2dPosYBitCountMinus1            = 0;
        size_t afps3dPosXBitCountMinus1            = 0;
        size_t afps3dPosYBitCountMinus1            = 0;
        bool afpsLodModeEnableFlag                 = false;
        bool afpsOverrideEomForDepthFlag           = false;
        uint8_t afpsEomNumberOfPatchBitCountMinus1 = 0;
        uint8_t afpsEomMaxBitCountMinus1           = 0;
        bool afpsRaw3dPosBitCountExplicitModeFlag  = false;
        uint8_t afpsExtensionPresentFlag           = 0;
        bool afpsExtensionDataFlag                 = false;
    };


    // 7.3.6.11  Atlas tile group header syntax
    struct AtlasTileGroupHeader
    {
        uint8_t atghFrameIndex                 = 0;
        uint8_t atghAtlasFrameParameterSetId   = 0;
        uint32_t atghAddress                   = 0;
        TileGroup::Enum atghType               = TileGroup::Enum::P;
        uint8_t atghAtlasFrmOrderCntLsb        = 0;
        bool atghRefAtlasFrameListSpsFlag      = false;
        uint8_t atghRefAtlasFrameListIdx       = 0;
        uint8_t atghPosMinZQuantizer           = 0;
        uint8_t atghPosDeltaMaxZQuantizer      = 0;
        uint8_t atghPatchSizeXinfoQuantizer    = 0;
        uint8_t atghPatchSizeYinfoQuantizer    = 0;
        uint8_t atghRaw3dPosAxisBitCountMinus1 = 0;
        bool atghNumRefIdxActiveOverrideFlag   = false;
        uint8_t atghNumRefIdxActiveMinus1      = 0;
        std::vector<bool> atghAdditionalAfocLsbPresentFlag{false};
        std::vector<uint8_t> atghAdditionalAfocLsbVal{0};
        RefListStruct refListStruct;
    };

    // 7.3.7.9 Point local reconstruction data syntax
    struct PointLocalReconstructionData
    {
        size_t blockToPatchMapHeight = 0;
        size_t blockToPatchMapWidth  = 0;
        bool levelFlag               = false;
        bool presentFlag             = false;
        uint8_t modeMinus1           = 0;
        std::vector<bool> blockPresentFlag;
        std::vector<uint8_t> blockModeMinus1;

        void allocate(size_t blockToPatchMapWidth, size_t blockToPatchMapHeight)
        {
            blockToPatchMapWidth  = blockToPatchMapWidth;
            blockToPatchMapHeight = blockToPatchMapHeight;
            blockPresentFlag.resize(blockToPatchMapWidth * blockToPatchMapHeight, false);
            blockModeMinus1.resize(blockToPatchMapWidth * blockToPatchMapHeight, 0);
        }
    };

    // 7.3.7.3  Patch data unit syntax
    struct PatchDataUnit
    {
        size_t pdu2dPosX           = 0;
        size_t pdu2dPosY           = 0;
        int64_t pdu2dDeltaSizeX    = 0;
        int64_t pdu2dDeltaSizeY    = 0;
        size_t pdu3dPosX           = 0;
        size_t pdu3dPosY           = 0;
        size_t pdu3dPosMinZ        = 0;
        size_t pdu3dPosDeltaMaxZ   = 0;
        size_t pduProjectionId     = 0;
        size_t pduOrientationIndex = 0;
        bool pduLodEnableFlag      = false;
        uint8_t pduLodScaleXminus1 = 0;
        uint8_t pduLodScaleY       = 0;
        PointLocalReconstructionData pointLocalReconstructionData;
        size_t pduPatchIndex = 0;
        size_t pduFrameIndex = 0;
    };

    // 7.3.7.6  Inter patch data unit syntax
    struct InterPatchDataUnit
    {
        int64_t ipduRefIndex       = 0;
        int64_t ipduRefPatchIndex  = 0;
        int64_t ipdu2dPosX         = 0;
        int64_t ipdu2dPosY         = 0;
        int64_t ipdu2dDeltaSizeX   = 0;
        int64_t ipdu2dDeltaSizeY   = 0;
        int64_t ipdu3dPosX         = 0;
        int64_t ipdu3dPosY         = 0;
        int64_t ipdu3dPosMinZ      = 0;
        int64_t ipdu3dPosDeltaMaxZ = 0;
        size_t ipduPatchIndex      = 0;
        size_t ipduFrameIndex      = 0;
        PointLocalReconstructionData pointLocalReconstructionData;
    };

    // 7.3.7.5  Merge patch data unit syntax
    struct MergePatchDataUnit
    {
        bool mpduOverride2dParamsFlag = false;
        bool mpduOverride3dParamsFlag = false;
        int64_t mpduRefIndex          = 0;
        int64_t mpdu2dPosX            = 0;
        int64_t mpdu2dPosY            = 0;
        int64_t mpdu2dDeltaSizeX      = 0;
        int64_t mpdu2dDeltaSizeY      = 0;
        int64_t mpdu3dPosX            = 0;
        int64_t mpdu3dPosY            = 0;
        int64_t mpdu3dPosMinZ         = 0;
        int64_t mpdu3dPosDeltaMaxZ    = 0;
        int64_t mpduOverridePlrFlag   = 0;
        size_t mpduPatchIndex         = 0;
        size_t mpduFrameIndex         = 0;
        PointLocalReconstructionData pointLocalReconstructionData;
    };

    // 7.3.7.4  Skip patch data unit syntax
    struct SkipPatchDataUnit
    {
        size_t spduPatchIndex = 0;
        size_t spduFrameIndex = 0;
    };

    // 7.3.7.7  raw patch data unit syntax
    struct RawPatchDataUnit
    {
        bool rpduPatchInRawVideoFlag = false;
        size_t rpdu2dPosX            = 0;
        size_t rpdu2dPosY            = 0;
        int64_t rpdu2dDeltaSizeX     = 0;
        int64_t rpdu2dDeltaSizeY     = 0;
        size_t rpdu3dPosX            = 0;
        size_t rpdu3dPosY            = 0;
        size_t rpdu3dPosZ            = 0;
        uint32_t rpduRawPoints       = 0;
        size_t rpduPatchIndex        = 0;
        size_t rpduFrameIndex        = 0;
    };

    // 7.3.7.8  EOM patch data unit syntax
    struct EOMPatchDataUnit
    {
        size_t epdu2dPosX                      = 0;
        size_t epdu2dPosY                      = 0;
        int64_t epdu2dDeltaSizeX               = 0;
        int64_t epdu2dDeltaSizeY               = 0;
        size_t epduAssociatedPatcheCountMinus1 = 0;
        size_t epduPatchIndex                  = 0;
        size_t epduFrameIndex                  = 0;
        std::vector<size_t> epduAssociatedPatches;
        std::vector<size_t> epduEomPointsPerPatch;
    };

    // 7.3.7.2  Patch information data syntax (pid)
    struct PatchInformationData
    {
        size_t frameIndex = 0;
        size_t patchIndex = 0;
        uint8_t patchMode;
        PatchDataUnit patchDataUnit;
        InterPatchDataUnit interPatchDataUnit;
        MergePatchDataUnit mergePatchDataUnit;
        SkipPatchDataUnit skipPatchDataUnit;
        RawPatchDataUnit rawPatchDataUnit;
        EOMPatchDataUnit eomPatchDataUnit;
    };

    // 7.3.7.1  General atlas tile group data unit syntax
    // atgduPatchMode_?)
    struct AtlasTileGroupDataUnit
    {
        PatchInformationData& addPatchInformationData(uint8_t patchMode)
        {
            PatchInformationData pid;
            pid.patchMode = patchMode;
            patchInformationData.push_back(pid);
            return patchInformationData.back();
        }
		
        size_t frameIndex;
        size_t patchCount;
        size_t atgduPatchMode;
        std::vector<PatchInformationData> patchInformationData;
    };

    // 7.3.6.10  Atlas tile group layer RBSP syntax
    struct AtlasTileGroupLayerRbsp
    {
        uint8_t frameIndex = 0;
        AtlasTileGroupHeader atlasTileGroupHeader;
        AtlasTileGroupDataUnit atlasTileGroupDataUnit;
    };

    struct PatchSequenceParameterSet
    {
        uint8_t patchSequenceParameterSetId     = 0;
        uint8_t log2PatchPackingBlockSize       = 0;
        uint8_t log2MaxPatchFrameOrderCntLsb    = 0;
        uint8_t maxDecPatchFrameBufferingMinus1 = 0;
        uint8_t numRefPatchFrameListsInPsps     = 0;

        bool longTermRefPatchFramesFlag              = false;
        bool useEightOrientationsFlag                = false;
        bool normalAxisLimitsQuantizationEnabledFlag = false;
        bool normalAxisMaxDeltaValueEnabledFlag      = false;

        std::vector<RefListStruct> refListStruct;
    };

    struct GeometryPatchParams
    {
        bool geometryPatchScaleParamsPresentFlag    = false;
        bool geometryPatchOffsetParamsPresentFlag   = false;
        bool geometryPatchRotationParamsPresentFlag = false;
        bool geometryPatchPointSizeInfoPresentFlag  = false;
        bool geometryPatchPointShapeInfoPresentFlag = false;

        uint32_t geometryPatchScaleOnAxis[3] = {0};
        int32_t geometryPatchOffsetOnAxis[3] = {0};
        int32_t geometryPatchRotationXYZW[4] = {0};

        uint16_t geometryPatchPointSizeInfo  = 0;
        uint32_t geometryPatchPointShapeInfo = 0;
    };

    struct GeometryPatchParameterSet
    {
        uint8_t geometryPatchParameterSetId      = 0;
        uint8_t patchFrameGeometryParameterSetId = 0;

        bool geometryPatchParamsPresentFlag = false;

        GeometryPatchParams geometryPatchParams;
    };

    struct AttributePatchParams
    {
        bool attributePatchScaleParamsPresentFlag = false;
        std::vector<uint32_t> attributePatchScale;

        bool attributePatchOffsetParamsPresentFlag = false;
        std::vector<int32_t> attributePatchOffset;
    };

    struct AttributePatchParameterSet
    {
        uint8_t attributePatchParameterSetId      = 0;
        uint8_t patchFrameAttributeParameterSetId = 0;
        uint8_t attributeDimensionMinus1          = 0;

        bool attributePatchParamsPresentFlag = false;

        AttributePatchParams attributePatchParams;
    };
	
	// 7.3.5.7 Attribute frame paramS syntax (afp)
	struct AttributeFrameParams
    {
        AttributeFrameParams()
        : attributeSmoothingParamsPresentFlag(false)
        , attributeScaleParamsPresentFlag(false)
        , attributeOffsetParamsPresentFlag(false)
        {
        }

        void allocate(size_t size)
        {
            attributeScale.resize(size, 0);
            attributeOffset.resize(size, 0);
            attributeSmoothingParamsPresentFlag.resize(size, 0);
            attributeSmoothingGridSizeMinus2.resize(size, 0);
            attributeSmoothingThreshold.resize(size, 0);
            attributeSmoothingThresholdAttributeDifference.resize(size, 0);
            attributeSmoothingThresholdAttributeVariation.resize(size, 0);
            attributeSmoothingLocalEntropyThreshold.resize(size, 0);
        }

        std::vector<bool> attributeSmoothingParamsPresentFlag;
        std::vector<uint8_t> attributeSmoothingGridSizeMinus2;
        std::vector<uint8_t> attributeSmoothingThreshold;
        std::vector<uint32_t> attributeSmoothingLocalEntropyThreshold;
        std::vector<uint8_t> attributeSmoothingThresholdAttributeVariation;
        std::vector<uint8_t> attributeSmoothingThresholdAttributeDifference;

        bool attributeScaleParamsPresentFlag  = false;
        bool attributeOffsetParamsPresentFlag = false;

        std::vector<uint32_t> attributeScale;
        std::vector<int32_t> attributeOffset;
    };

    // 7.3.5.6 Patch frame attribute parameter set syntax
    struct PatchFrameAttributeParameterSet
    {
        uint8_t patchFrameAttributeParameterSetId = 0;
        uint8_t patchSequencParameterSetId        = 0;
        uint8_t attributeDimensionMinus1          = 3;

        bool attributePatchScaleParamsEnabledFlag  = false;
        bool attributePatchOffsetParamsEnabledFlag = false;

        AttributeFrameParams attributeFrameParams;
    };

    struct PatchFrameTileInformation
    {
        bool singleTileInPatchFrameFlag = false;
        bool uniformTileSpacingFlag     = false;

        uint32_t numTileColumnsMinus1 = 0;
        uint32_t numTileRowsMinus1    = 0;

        uint32_t singleTilePerTileGroupFlag      = 0;
        uint32_t numTileGroupsInPatchFrameMinus1 = 0;

        bool signalledTileGroupIdFlag             = false;
        uint32_t signalledTileGroupIdLengthMinus1 = 0;

        std::vector<uint32_t> tileColumnWidthMinus1;
        std::vector<uint32_t> tileRowHeightMinus1;
        std::vector<uint32_t> topLeftTileIdx;
        std::vector<uint32_t> bottomRightTileIdxDelta;
        std::vector<uint32_t> tileGroupId;

        PatchFrameTileInformation()
        {
            tileColumnWidthMinus1.resize(1, 0);
            tileRowHeightMinus1.resize(1, 0);
            topLeftTileIdx.resize(1, 0);
            bottomRightTileIdxDelta.resize(1, 0);
            tileGroupId.resize(1, 0);
        }
    };

    struct PatchFrameParameterSet
    {
        uint8_t patchFrameParameterSetId         = 0;
        uint8_t patchSequenceParameterSetId      = 0;
        uint8_t geometryPatchFrameParameterSetId = 0;

        std::vector<uint8_t> attributePatchFrameParameterSetId;

        uint8_t additionalLtPfocLsbLen = 0;

        bool localOverrideGeometryPatchEnabledFlag = false;
        std::vector<bool> localOverrideAttributePatchEnabledFlag;

        bool projection45DegreeEnabledFlag = false;

        PatchFrameTileInformation patchFrameTileInformation;
    };

    struct GeometryFrameParams
    {
        bool geometrySmoothingParamsPresentFlag = false;
        bool geometryScaleParamsPresentFlag     = false;
        bool geometryOffsetParamsPresentFlag    = false;
        bool geometryRotationParamsPresentFlag  = false;
        bool geometryPointSizeInfoPresentFlag   = false;
        bool geometryPointShapeInfoPresentFlag  = false;
        bool geometrySmoothingEnabledFlag       = false;

        uint8_t geometrySmoothingGridSizeMinus2 = 0;
        uint8_t geometrySmoothingThreshold      = 0;

        uint32_t geometryScaleOnAxis[3] = {0};
        int32_t geometryOffsetOnAxis[3] = {0};
        int32_t geometryRotationXYZW[4] = {0};

        uint16_t geometryPointSizeInfo  = 0;
        uint32_t geometryPointShapeInfo = 0;
    };

    struct PatchFrameGeometryParameterSet
    {
        uint8_t patchFrameGeometryParameterSetId = 0;
        uint8_t patchSequenceParameterSetId      = 0;

        bool geometryPatchParamsEnabledFlag         = false;
        bool overrideGeometryPatchParamsFlag        = false;
        bool geometryPatchScaleParamsEnabledFlag    = false;
        bool geometryPatchOffsetParamsEnabledFlag   = false;
        bool geometryPatchRotationParamsEnabledFlag = false;
        bool geometryPatchPointSizeInfoEnabledFlag  = false;
        bool geometryPatchPointShapeInfoEnabledFlag = false;

        GeometryFrameParams geometryFrameParams;
    };

    struct PatchTileGroupHeader
    {
        uint8_t frameIndex               = 0;
        uint8_t patchFrameParameterSetId = 0;
        uint8_t type                     = 0;
        uint32_t address                 = 0;
        uint8_t patchFrameOrderCntLsb    = 0;
        uint8_t refPatchFrameListIdx     = 0;

        bool refPatchFrameListSpsFlag = false;

        std::vector<bool> additionalPfocLsbPresentFlag;
        std::vector<uint32_t> additionalPfocLsbVal;

        bool numRefIdxActiveOverrideFlag = false;

        uint8_t numRefIdxActiveMinus1 = 0;

        uint8_t normalAxisMinValueQuantizer      = 0;
        uint8_t normalAxisMaxDeltaValueQuantizer = 0;

        uint8_t interPredictPatch2dShiftUBitCountMinus1             = 0;
        uint8_t interPredictPatch2dShiftVBitCountMinus1             = 0;
        uint8_t interPredictPatch2dDeltaSizeDBitCountMinus1         = 0;
        uint8_t interPredictPatch3dShiftTangentAxisBitCountMinus1   = 0;
        uint8_t interPredictPatch3dShiftBitangentAxisBitCountMinus1 = 0;
        uint8_t interPredictPatch3dShiftNormalAxisBitCountMinus1    = 0;
        uint8_t interPredictPatchLodBitCount                        = 0;

        bool interPredictPatchBitCountFlag                     = false;
        bool interPredictPatch2dShiftUBitCountFlag             = false;
        bool interPredictPatch2dShiftVBitCountFlag             = false;
        bool interPredictPatch3dShiftTangentAxisBitCountFlag   = false;
        bool interPredictPatch3dShiftBitangentAxisBitCountFlag = false;
        bool interPredictPatch3dShiftNormalAxisBitCountFlag    = false;
        bool interPredictPatchLodBitCountFlag                  = false;

        uint8_t pcm3dShiftAxisBitCountMinus1 = 9;

        bool pcm3dShiftBitCountPresentFlag = true;

        PatchTileGroupHeader()
        {
            additionalPfocLsbPresentFlag.resize(1, 0);
            additionalPfocLsbVal.resize(1, 0);
        }
    };

    struct DeltaPatchDataUnit
    {
        int64_t deltaPatchIndex = 0;

        int64_t deltaShiftU = 0;
        int64_t deltaShiftV = 0;

        int64_t deltaSizeU = 0;
        int64_t deltaSizeV = 0;

        int64_t deltaShiftTangentAxis   = 0;
        int64_t deltaShiftBiTangentAxis = 0;
        int64_t deltaShiftMinNormalAxis = 0;

        int64_t shiftDeltaMaxNormalAxis = 0;
        Axis6::Enum projectPlane        = Axis6::UNDEFINED;
        uint8_t lod                     = 0;
        size_t patchIndex               = 0;
        size_t frameIndex               = 0;
        PointLocalReconstructionData pointLocalReconstructionData;
    };

    struct PCMPatchDataUnit
    {
        bool patchInPcmVideoFlag  = false;
        size_t shiftU             = 0;
        size_t shiftV             = 0;
        int64_t deltaSizeU        = 0;
        int64_t deltaSizeV        = 0;
        size_t shiftTangentAxis   = 0;
        size_t shiftBiTangentAxis = 0;
        size_t shiftNormalAxis    = 0;
        uint32_t pcmPoints        = 0;
        size_t patchIndex         = 0;
        size_t frameIndex         = 0;
    };

    struct PatchTileGroupDataUnit
    {
        size_t frameIndex = 0;
        std::vector<uint8_t> patchMode;
        std::vector<PatchInformationData> patchInformationData;
    };

    struct PatchTileGroupLayerUnit
    {
        uint8_t frameIndex = 0;

        PatchTileGroupHeader patchTileGroupHeader;
        PatchTileGroupDataUnit patchTileGroupDataUnit;
    };


    // Annex E: Supplemental enhancement information
    // E.2.1	General SEI message syntax  <=> 7.3.8	Supplemental enhancement information message syntax
    struct SEI
    {
        virtual SeiPayloadType::Enum getPayloadType() = 0;
        uint8_t payloadSize;
    };

    // E.2.3  User data registered by Recommendation ITU-T T.35 SEI message syntax
    struct SEIUserDataRegisteredItuTT35 : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::USER_DATAREGISTERED_ITUTT35;
        }

        uint8_t ituTT35CountryCode              = 0;
        uint8_t ituTT35CountryCodeExtensionByte = 0;
        std::vector<uint8_t> ituTT35PayloadByte;
    };

    // E.2.4  User data unregistered SEI message syntax
    struct SEIUserDataUnregistered : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::USER_DATA_UNREGISTERED;
        }

        uint8_t uuidIsoIec11578[16] = {0};
        std::vector<uint8_t> userDataPayloadByte;
    };

    // E.2.5  Recovery point SEI message syntax
    struct SEIRecoveryPoint : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::RECOVERY_POINT;
        }

        int32_t recoveryAfocCnt = 0;
        uint8_t exactMatchFlag  = 0;
        uint8_t brokenLinkFlag  = 0;
    };

    // E.2.6  No display SEI message syntax
    struct SEINoDisplay : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::NO_DISPLAY;
        }

    private:
    };

    // E.2.7  Reserved SEI message syntax
    struct SEIReservedSeiMessage : public SEI
    {
        SeiPayloadType::SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::RESERVED_SEI_MESSAGE;
        }

        std::vector<uint8_t> reservedSeiMessagePayloadByte;
    };

    // E.2.8  SEI manifest SEI message syntax
    struct SEIManifest : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::SEI_MANIFEST;
        }

        void allocate()
        {
            manifestSeiPayloadType.resize(manifestNumSeiMsgTypes, 0);
            manifestSeiDescription.resize(manifestNumSeiMsgTypes, 0);
        }

        uint16_t manifestNumSeiMsgTypes = 0;
        std::vector<uint16_t> manifestSeiPayloadType;
        std::vector<uint8_t> manifestSeiDescription;
    };

    // E.2.9  SEI prefix indication SEI message syntax
    struct SEIPrefixIndication : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::SEI_PREFIX_INDICATION;
        }

        uint16_t prefixSeiPayloadType         = 0;
        uint8_t numSeiPrefixIndicationsMinus1 = 0;
        std::vector<uint16_t> numBitsInPrefixIndicationMinus1;
        std::vector<std::vector<bool>> seiPrefixDataBit;
    };

    // E.2.10  Geometry transformation parameters SEI message syntax
    struct SEIGeometryTransformationParams : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::SEI_PREFIX_INDICATION;
        }
        bool gtpCancelFlag = false;
        bool gtpScaleEnabledFlag = false;
        bool gtpOffsetEnabledFlag = false;
        bool gtpRotationEnabledFlag        = false;
        uint32_t gtpGeometryScaleOnAxis[3] = {0};
        int32_t gtpGeometryOffsetOnAxis[3] = {0};
        int16_t gtpRotationQx              = 0;
        int16_t gtpRotationQy              = 0;
        int16_t gtpRotationQz              = 0;
    };

    // E.2.11  Attribute transformation parameters SEI message syntax
    struct SEIAttributeTransformationParams : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::ATTRIBUTE_TRANSFORMATION_PARAMS;
        }

		void allocate()
        {
            atpAttributeIdx.resize(atpNumAttributeUpdates + 1, 0);
            atpDimensionMinus1.resize(256 + 1, 0);
            atpScaleParamsEnabledFlag.resize(256);
            atpOffsetParamsEnabledFlag.resize(256);
            atpAttributeScale.resize(256);
            atpAttributeOffset.resize(256);
        }
        
        void allocate(size_t index)
        {
            atpScaleParamsEnabledFlag[index].resize(atpDimensionMinus1[index] + 1, false);
            atpOffsetParamsEnabledFlag[index].resize(atpDimensionMinus1[index] + 1, false);
            atpAttributeScale[index].resize(atpDimensionMinus1[index] + 1, 0);
            atpAttributeOffset[index].resize(atpDimensionMinus1[index] + 1, 0);
        }

		bool atpCancelFlag = false;
        int32_t atpNumAttributeUpdates = 0;
        std::vector<uint8_t> atpAttributeIdx;
        std::vector<uint8_t> atpDimensionMinus1;
        std::vector<std::vector<bool>> atpScaleParamsEnabledFlag;
        std::vector<std::vector<bool>> atpOffsetParamsEnabledFlag;
        std::vector<std::vector<uint32_t>> atpAttributeScale;
        std::vector<std::vector<int32_t>> atpAttributeOffset;
    };

    // E.2.12  Active substreams SEI message syntax
    struct SEIActiveSubstreams : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::ACTIVE_SUBSTREAMS;
        }

        bool activeAttributesChangesFlag = false;
        bool activeMapsChangesFlag       = false;
        bool rawPointsSubstreamsActiveFlag = false;
        bool allAttributesActiveFlag       = false;
        bool allMapsActiveFlag             = false;
        uint8_t activeAttributeCountMinus1 = 0;
        uint8_t activeMapCountMinus1 = 0;
        std::vector<uint8_t> activeAttributeIdx;
        std::vector<uint8_t> activeMapIdx;
    };

    // E.2.13  Component codec mapping SEI message syntax
    struct SEIComponentCodecMapping : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::COMPONENT_CODEC_MAPPING;
        }

        void allocate()
        {
            ccmCodecId.resize(ccmCodecMappingsCountMinus1 + 1, 0);
            ccmCodec4cc.resize(256);
        }

		uint8_t ccmCodecMappingsCountMinus1 = 0;
        std::vector<uint8_t> ccmCodecId;
        std::vector<std::string> ccmCodec4cc;
    };

    // E.2.14.2  Volumetric Tiling Info Labels
    struct VolumetricTilingInfoLabels
    {
        void allocate()
        {
            vtiLabelIdx.resize(vtiNumObjectLabelUpdates, 0);
        }

		std::string& getVtiLabel(size_t index)
        {
            if (vtiLabel.size() < index)
            {
                vtiLabel.resize(index);
            }

            return vtiLabel[index];
        }

        void setVtiLabel(size_t index, std::string value)
        {
            if (vtiLabel.size() < index)
            {
                vtiLabel.resize(index + 1);
            }
            
            vtiLabel[index] = value;
        }

        bool vtiObjectLabelLanguagePresentFlag = false;
        uint32_t vtiNumObjectLabelUpdates = 0;
        std::string vtiObjectLabelLanguage;
        std::vector<uint8_t> vtiLabelIdx;
        std::vector<std::string> vtiLabel;
    };

    // E.2.14.3  Volumetric Tiling Info Objects
    struct VolumetricTilingInfoObjects
    {
        void allocate()
        {
            vtiObjectIdx.resize(vtiNumObjectUpdates);
        }

        void allocate(size_t size)
        {
            if (vtiObjectCancelFlag.size() < size)
            {
                vtiObjectCancelFlag.resize(size);
                vtiBoundingBoxUpdateFlag.resize(size);
                vti3dBoundingBoxUpdateFlag.resize(size);
                vtiObjectHiddenFlag.resize(size);
                vtiObjectPriorityUpdateFlag.resize(size);
                vtiObjectLabelUpdateFlag.resize(size);
                vtiObjectCollisionShapeUpdateFlag.resize(size);
                vtiObjectDependencyUpdateFlag.resize(size);
                vtiBoundingBoxTop.resize(size);
                vtiBoundingBoxLeft.resize(size);
                vtiBoundingBoxWidth.resize(size);
                vtiBoundingBoxHeight.resize(size);
                vti3dBoundingBoxX.resize(size);
                vti3dBoundingBoxY.resize(size);
                vti3dBoundingBoxZ.resize(size);
                vti3dBoundingBoxDeltaX.resize(size);
                vti3dBoundingBoxDeltaY.resize(size);
                vti3dBoundingBoxDeltaZ.resize(size);
                vtiObjectPriorityValue.resize(size);
                vtiObjectLabelIdx.resize(size);
                vtiObjectCollisionShapeId.resize(size);
                vtiObjectNumDependencies.resize(size);
                vtiObjectDependencyIdx.resize(size);
            }
        }

		uint32_t vtiNumObjectUpdates = 0;
        std::vector<uint8_t> vtiObjectIdx;
        std::vector<bool> vtiObjectCancelFlag;
        std::vector<bool> vtiBoundingBoxUpdateFlag;
        std::vector<bool> vti3dBoundingBoxUpdateFlag;
        std::vector<bool> vtiObjectHiddenFlag;
        std::vector<bool> vtiObjectPriorityUpdateFlag;
        std::vector<bool> vtiObjectLabelUpdateFlag;
        std::vector<bool> vtiObjectCollisionShapeUpdateFlag;
        std::vector<bool> vtiObjectDependencyUpdateFlag;
        std::vector<uint32_t> vtiBoundingBoxTop;
        std::vector<uint32_t> vtiBoundingBoxLeft;
        std::vector<uint32_t> vtiBoundingBoxWidth;
        std::vector<uint32_t> vtiBoundingBoxHeight;
        std::vector<uint32_t> vti3dBoundingBoxX;
        std::vector<uint32_t> vti3dBoundingBoxY;
        std::vector<uint32_t> vti3dBoundingBoxZ;
        std::vector<uint32_t> vti3dBoundingBoxDeltaX;
        std::vector<uint32_t> vti3dBoundingBoxDeltaY;
        std::vector<uint32_t> vti3dBoundingBoxDeltaZ;
        std::vector<uint32_t> vtiObjectPriorityValue;
        std::vector<uint32_t> vtiObjectLabelIdx;
        std::vector<uint32_t> vtiObjectCollisionShapeId;
        std::vector<uint32_t> vtiObjectNumDependencies;
        std::vector<std::vector<uint32_t>> vtiObjectDependencyIdx;
    };

	// E.2.14.1  General
    struct SEIVolumetricTilingInfo : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::VOLUMETRIC_TILING_INFO;
        }

		bool vtiCancelFlag = false;
        bool vtiObjectLabelPresentFlag = false;
        bool vti3dBoundingBoxPresentFlag = false;
        bool vtiObjectPriorityPresentFlag = false;
        bool vtiObjectHiddenPresentFlag = false;
        bool vtiObjectCollisionShapePresentFlag = false;
        bool vtiObjectDependencyPresentFlag = false;
        uint8_t vtiBoundingBoxScaleLog2 = 0;
        uint8_t vti3dBoundingBoxScaleLog2 = 0;
        uint8_t vti3dBoundingBoxPrecisionMinus8 = 0;
        VolumetricTilingInfoLabels volumetricTilingInfoLabels;
        VolumetricTilingInfoObjects volumetricTilingInfoObjects;
    };


	// E.2.15  Buffering period SEI message syntax
    struct SEIBufferingPeriod : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::BUFFERING_PERIOD;
        }
        void allocate()
        {
            bpNalInitialCabRemovalDelay.resize(bpMaxSubLayersMinus1 + 1);
            bpNalInitialCabRemovalOffset.resize(bpMaxSubLayersMinus1 + 1);
            bpAclInitialCabRemovalDelay.resize(bpMaxSubLayersMinus1 + 1);
            bpAclInitialCabRemovalOffset.resize(bpMaxSubLayersMinus1 + 1);
            bpNalInitialAltCabRemovalDelay.resize(bpMaxSubLayersMinus1 + 1);
            bpNalInitialAltCabRemovalOffset.resize(bpMaxSubLayersMinus1 + 1);
            bpAclInitialAltCabRemovalDelay.resize(bpMaxSubLayersMinus1 + 1);
            bpAclInitialAltCabRemovalOffset.resize(bpMaxSubLayersMinus1 + 1);
        }

        bool bpIrapCabParamsPresentFlag = false;
        bool bpConcatenationFlag = false;
        uint8_t bpAtlasSequenceParameterSetId = 0;
        uint32_t bpCabDelayOffset = 0;
        uint32_t bpDabDelayOffset = 0;
        uint32_t bpAtlasCabRemovalDelayDeltaMinus1 = 0;
        uint32_t bpMaxSubLayersMinus1 = 0;
        std::vector<std::vector<uint32_t>> bpNalInitialCabRemovalDelay;
        std::vector<std::vector<uint32_t>> bpNalInitialCabRemovalOffset;
        std::vector<uint32_t> bpNalInitialAltCabRemovalDelay;
        std::vector<uint32_t> bpNalInitialAltCabRemovalOffset;
        std::vector<std::vector<uint32_t>> bpAclInitialCabRemovalDelay;
        std::vector<std::vector<uint32_t>> bpAclInitialCabRemovalOffset;
        std::vector<uint32_t> bpAclInitialAltCabRemovalDelay;
        std::vector<uint32_t> bpAclInitialAltCabRemovalOffset;
    };

    // E.2.16  Atlas frame timing SEI message syntax
    struct SEIAtlasFrameTiming : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::ATLAS_FRAME_TIMING;
        }

        uint32_t aftCabRemovalDelayMinus1 = 0;
        uint32_t aftDabOutputDelay = 0;
    };

	// E.2.17  Presentation inforomation SEI message syntax
    struct SEIPresentationInformation : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::PRESENTATION_INFORMATION;
        }

        bool piUnitOfLengthFlag = false;
        bool piOrientationPresentFlag = false;
        bool piPivotPresentFlag = false;
        bool piDimensionPresentFlag = false;
        int32_t piUp[3] = { 0 };
        int32_t piFront[3] = { 0 };
        int64_t piPivot[3] = { 0 };
        int64_t piDimension[3] = { 0 };
    };

    // E.2.18  Smoothing parameters SEI message syntax
    struct SEISmoothingParameters : public SEI
    {
        SeiPayloadType::Enum getPayloadType()
        {
            return SeiPayloadType::Enum::SMOOTHING_PARAMETERS;
        }

        void allocate()
        {
            spAttributeIdx.resize(spNumAttributeUpdates);
        }
        
		void allocate(size_t size, size_t dimension)
        {
            if (spDimensionMinus1.size() < size)
            {
                spDimensionMinus1.resize(size);
                spAttrSmoothingParamsEnabledFlag.resize(size);
                spAttrSmoothingGridSizeMinus2.resize(size);
                spAttrSmoothingThreshold.resize(size);
                spAttrSmoothingLocalEntropyThreshold.resize(size);
                spAttrSmoothingThresholdVariation.resize(size);
                spAttrSmoothingThresholdDifference.resize(size);
            }
            
            spAttrSmoothingParamsEnabledFlag[size - 1].resize(dimension);
            spAttrSmoothingGridSizeMinus2[size - 1].resize(dimension);
            spAttrSmoothingThreshold[size - 1].resize(dimension);
            spAttrSmoothingLocalEntropyThreshold[size - 1].resize(dimension);
            spAttrSmoothingThresholdVariation[size - 1].resize(dimension);
            spAttrSmoothingThresholdDifference[size - 1].resize(dimension);
        }

        bool spGeometryCancelFlag = false;
        bool spAttributeCancelFlag = false;
        bool spGeometrySmoothingEnabledFlag = false;
        uint8_t spGeometrySmoothingGridSizeMinus2 = 0;
        uint8_t spGeometrySmoothingThreshold = 0;
        uint8_t spGeometrySmoothingId = 0;
        uint8_t spGeometryPatchBlockFilteringLog2ThresholdMinus1 = 0;
        uint8_t spGeometryPatchBlockFilteringPassesCountMinus1 = 0;
        uint8_t spGeometryPatchBlockFilteringFilterSizeMinus1 = 0;

        uint32_t spNumAttributeUpdates = 0;
        std::vector<uint32_t> spAttributeIdx;
        std::vector<uint32_t> spDimensionMinus1;
        std::vector<std::vector<bool>> spAttrSmoothingParamsEnabledFlag;
        std::vector<std::vector<uint32_t>> spAttrSmoothingGridSizeMinus2;
        std::vector<std::vector<uint32_t>> spAttrSmoothingThreshold;
        std::vector<std::vector<uint32_t>> spAttrSmoothingLocalEntropyThreshold;
        std::vector<std::vector<uint32_t>> spAttrSmoothingThresholdVariation;
        std::vector<std::vector<uint32_t>> spAttrSmoothingThresholdDifference;
    };

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    struct Vector3U
    {
        uint32_t x = 0;
        uint32_t y = 0;
        uint32_t z = 0;
    };

    struct Vector3I
    {
        int32_t x = 0;
        int32_t y = 0;
        int32_t z = 0;
    };

    struct PointShape
    {
        enum Enum
        {
            CIRCLE  = 0,
            SQUARE  = 1,
            DIAMOND = 2,
        };
    };

    struct MetadataEnabledFlags
    {
        bool metadataEnabled   = false;
        bool scaleEnabled      = false;
        bool offsetEnabled     = false;
        bool rotationEnabled   = false;
        bool pointSizeEnabled  = false;
        bool pointShapeEnabled = false;
    };

    struct Metadata
    {
        bool metadataPresent = false;

        bool scalePresent = false;
        Vector3U scale = { 1, 1, 1 };

        bool offsetPresent = false;
        Vector3I offset = { 0, 0, 0 };

        bool rotationPresent = false;
        Vector3I rotation = { 0, 0, 0 };

        bool pointSizePresent = false;
        uint16_t pointSize = 1;

        bool pointShapePresent = false;
        PointShape::Enum pointShape = PointShape::CIRCLE;

        MetadataEnabledFlags metadataEnabledFlags;
        MetadataEnabledFlags lowerLevelMetadataEnabledFlags;

        size_t index = 0;
        MetadataType::Enum metadataType = MetadataType::GOF;
    };

    struct Patch
    {
        size_t index = 0;  // patch index

        uint32_t u1 = 0;  // tangential shift
        uint32_t v1 = 0;  // bitangential shift

        int32_t d1 = 0;  // depth shift

        uint32_t sizeD = 0;  // size for depth
        uint32_t sizeU = 0;  // size for depth
        uint32_t sizeV = 0;  // size for depth

        uint32_t u0 = 0;  // location in packed image
        uint32_t v0 = 0;  // location in packed image

        uint32_t sizeU0 = 0;  // size of occupancy map
        uint32_t sizeV0 = 0;  // size of occupancy map
        
        size_t levelOfDetailX = 0;
        size_t levelOfDetailY = 0;
        
        size_t size2DXInPixel = 0;
        size_t size2DYInPixel = 0;

        uint32_t occupancyResolution = 0;  // occupancy map resolution

        uint32_t normalAxis = 0;  // x
        uint32_t tangentAxis = 0;  // y
        uint32_t bitangentAxis = 0;  // z

        uint32_t patchOrientation = 0;  // patch orientation in canvas atlas
        uint32_t projectionMode = 0;  // 0: related to the min depth value; 1: related to the max value
        uint32_t axisOfAdditionalPlane = 0;

        int32_t bestMatchIndex = 0;  // index of matched patch from pre-frame patch.
        size_t refAtlasFrameIdx = 0;
    };

    struct MissedPointsPatch
    {
        size_t                u1;  // tangential shift
        size_t                v1;  // bitangential shift
        size_t                d1;  // depth shift
        size_t                sizeU;
        size_t                sizeV;
        size_t                u0;
        size_t                v0;
        size_t                sizeV0;
        size_t                sizeU0;
        size_t                occupancyResolution;
        std::vector<bool>     occupancy;
        std::vector<uint16_t> x;
        std::vector<uint16_t> y;
        std::vector<uint16_t> z;
        std::vector<uint16_t> r;
        std::vector<uint16_t> g;
        std::vector<uint16_t> b;

        size_t numberOfEddPoints;
        size_t numberOfMps;
        size_t numberOfMpsColors;

        // GPA.
        size_t preV0;
        size_t tempV0;
    };

    struct EomPatch
    {
        size_t              u0;
        size_t              v0;
        size_t              sizeU;
        size_t              sizeV;
        size_t              eddCount;  // in this EomPatch
        std::vector<size_t> memberPatches;
        std::vector<size_t> eddCountPerPatch;
    };

    struct VideoFramePacket
    {
        size_t offset = 0;
        size_t length = 0;

        size_t sliceIndex = 0; // Total slice index
        size_t pictureOrderCount = 0; // POC
    };

    struct FrameData
    {
        int64_t presentationTimeStamp = 0;

        std::vector<Patch> patches;
        std::vector<MissedPointsPatch> missedPointsPatches;
        std::vector<EomPatch> eomPatches;
        
        std::vector<size_t> blockToPatch;
        
        size_t afOrderCnt = 0;
        size_t index = 0;
        bool rawPatchEnabledFlag = 0;
        
        size_t width = 0;
        size_t height = 0;
        
        std::vector<std::vector<size_t>> refAFOCList;
    };

    struct VideoStream
    {
        VideoType::Enum type = VideoType::INVALID;

        std::vector<uint8_t> buffer;
        std::vector<VideoFramePacket> packets;

        HEVC::DecoderParameters decoderParameters;

        HEVC::VPS vps;
        HEVC::SPS sps;
    };

    struct FrameGroup
    {
        std::vector<FrameData> frames;

        // HEVC streams
        VideoStream videoStream[VPCC::VideoType::COUNT];
    };

    struct ParserContext
    {
		uint32_t ssvhUnitSizePrecisionBytesMinus1 = 0;

		VpccUnitHeader vpccUnitHeader[5];

        std::vector<VpccParameterSet> vpccParameterSets;
        
        uint8_t activeVpsId = 0;
        
        std::vector<std::vector<int32_t>> refAtlasFrameList;

        std::vector<AtlasSequenceParameterSetRBSP> atlasSequenceParameterSet;
        std::vector<AtlasFrameParameterSetRbsp> atlasFrameParameterSet;
        std::vector<AtlasTileGroupLayerRbsp> atlasTileGroupLayer;
        
        std::vector<std::shared_ptr<SEI>> seiPrefix;
        std::vector<std::shared_ptr<SEI>> seiSuffix;

        VpccParameterSet& addVpccParameterSet()
        {
            uint8_t index = vpccParameterSets.size();

            VpccParameterSet vps;
            vps.vpccParameterSetId = index;

            vpccParameterSets.push_back(vps);

            return vpccParameterSets.back();
        }

		void setActiveVps(uint8_t vpsId)
        {
            activeVpsId = vpsId;
        }

		VpccParameterSet& getActiveVps()
        {
            return vpccParameterSets[activeVpsId];
        }
        
		AtlasSequenceParameterSetRBSP& addAtlasSequenceParameterSet()
        {
            AtlasSequenceParameterSetRBSP asps;
            asps.altasSequenceParameterSetId = atlasSequenceParameterSet.size();
            atlasSequenceParameterSet.push_back(asps);

            return atlasSequenceParameterSet.back();
        }

        AtlasFrameParameterSetRbsp& addAtlasFrameParameterSet()
        {
            AtlasFrameParameterSetRbsp afps;
            afps.afpsAtlasFrameParameterSetId = atlasFrameParameterSet.size();

            atlasFrameParameterSet.push_back(afps);

            return atlasFrameParameterSet.back();
        }

        AtlasTileGroupLayerRbsp& addAtlasTileGroupLayer()
        {
            size_t frameIdx = atlasTileGroupLayer.size();

            AtlasTileGroupLayerRbsp atgl;
            atgl.frameIndex = frameIdx;
            atgl.atlasTileGroupDataUnit.frameIndex = frameIdx;

            atlasTileGroupLayer.push_back(atgl);

            return atlasTileGroupLayer.back();
        }
    };
}

////////////////////////////////////////////////////////////////////////////////
/// Functionality
////////////////////////////////////////////////////////////////////////////////
namespace VPCC
{
    bool parseContainerHeader(Bitstream& bitstream, TMC2Header& header);

    bool parseFirstFrameGroup(Bitstream& bitstream, FrameGroup& frameGroup);
    bool parseAllFrameGroups(Bitstream& bitstream, std::vector<FrameGroup>& frameGroups);

    // For rendering purposes
    int32_t patchBlockToCanvasBlock(Patch& patch, const size_t blockU, const size_t blockV, size_t canvasStrideBlk, size_t canvasHeightBlk);
    size_t patchToCanvas(Patch& patch, const size_t u, const size_t v, size_t canvasStride, size_t canvasHeight, size_t& x, size_t& y);
}

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

#include <cstdlib>
#include <cstdint>
#include <cstddef>

#include <assert.h>

#include <vector>
#include <string>

#include "PCCBitstream60.h"
#include "Helpers.h"

////////////////////////////////////////////////////////////////////////////////
/// Data structures
////////////////////////////////////////////////////////////////////////////////
namespace PCC
{
    // TMC2 data types

    const uint32_t TMC2ContainerMagicNumber = 23021981;
    const uint32_t TMC2ContainerVersion = 1;

    struct TMC2Header
    {
        uint32_t magic = 0;
        uint32_t version = 0;
        uint64_t totalSize = 0;
    };

    // PCC data types

    struct ColorTransform
    {
        enum Enum
        {
            NONE = 0,
            RGB_TO_YCBCR = 1,
        };
    };

    struct Axis6
    {
        enum Enum
        {
            UNDEFINED = -1,

            X_NEAR    = 0,
            Y_NEAR    = 1,
            Z_NEAR    = 2,
            X_FAR     = 3,
            Y_FAR     = 4,
            Z_FAR     = 5
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
        };
    };

    struct VideoType
    {
        enum Enum
        {
            OCCUPANCY = 0,
            GEOMETRY,
            GEOMETRY_D0,
            GEOMETRY_D1,
            GEOMETRY_MP,
            TEXTURE,
            TEXTURE_MP,
        };

        static const char* toString(Enum value)
        {
            switch (value)
            {
                case OCCUPANCY:     return "OCCUPANCY";
                case GEOMETRY:      return "GEOMETRY";
                case GEOMETRY_D0:   return "GEOMETRY_D0";
                case GEOMETRY_D1:   return "GEOMETRY_D1";
                case GEOMETRY_MP:   return "GEOMETRY_MP";
                case TEXTURE:       return "TEXTURE";
                case TEXTURE_MP:    return "TEXTURE_MP";

                default:
                    return NULL;
            }
        }
    };

    struct MetadataType
    {
        enum Enum
        {
            GOF = 0,
            FRAME,
            PATCH
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
            MROT270 = 8
        };
    };

    struct VPCCUnitType
    {
        enum Enum
        {
            SPS = 0,  // 0: Sequence parameter set
            PDG,      // 1: Patch Data Group
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
            RSVD_31   // 32: Reserved
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
            RSVD_31      // 32: Reserved
        };
    };

    struct CodecID
    {
        enum Enum
        {
            HEVC = 0,
        };
    };

    struct PatchFrameType
    {
        enum Enum
        {
            I = 0,
            P,
        };
    };

    struct PatchModeI
    {
        enum Enum
        {
            INTRA = 0,
            PCM,
            END = 14,
        };
    };

    struct PatchModeP
    {
        enum Enum
        {
            SKIP = 0,
            INTRA,
            INTER,
            PCM,
            END = 14,
        };
    };

    struct PatchType
    {
        enum Enum
        {
            INTRA = 0,
            INTER,
            SKIP,
            END,
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

    struct VPCCParameterSet
    {
        uint8_t unitType = 0;
        uint8_t sequenceParameterSetId = 0;
        uint8_t attributeIndex = 0;
        uint8_t attributeDimensionIndex = 0;
        uint8_t layerIndex = 0;
        bool pcmVideoFlag = false;
    };

    struct ProfileTierLevel
    {
        bool tierFlag = false;
        uint8_t profileCodecGroupIdc = 0;
        uint8_t profilePccToolsetIdc = 0;
        uint8_t profileReconctructionIdc = 0;
        uint8_t levelIdc = 0;
    };

    struct GeometryInformation
    {
        uint8_t geometryCodecId = 0;
        uint8_t geometryNominal2dBitdepthMinus1 = 10;
        uint8_t geometry3dCoordinatesBitdepthMinus1 = 9;
        uint8_t pcmGeometryCodecId = 0;
        bool geometryParamsEnabledFlag = false;
        bool geometryPatchParamsEnabledFlag = false;
    };

    struct OccupancyInformation
    {
        uint8_t occupancyCodecId = 0;
        uint8_t lossyOccupancyMapCompressionThreshold = 0;
    };

    struct AttributeInformation
    {
        uint8_t attributeCount = 0;
        bool attributeParamsEnabledFlag = false;
        bool attributePatchParamsEnabledFlag = false;
        bool attributeMSBAlignFlag = false;

        std::vector<uint8_t> attributeTypeId;
        std::vector<uint8_t> attributeCodecId;
        std::vector<uint8_t> pcmAttributeCodecId;
        std::vector<uint8_t> attributeDimensionMinus1;
        std::vector<uint8_t> attributeDimensionPartitionsMinus1;
        std::vector<uint8_t> attributeNominal2dBitdepthMinus1;
        std::vector<std::vector<uint8_t>> attributePartitionChannelsMinus1;
    };

    struct PointLocalReconstructionInformation
    {
        uint8_t numberOfModesMinus1 = 0;
        uint8_t blockThresholdPerPatchMinus1 = 0;

        std::vector<uint8_t> minimumDepth;
        std::vector<uint8_t> neighbourMinus1;
        std::vector<bool> interpolateFlag;
        std::vector<bool> fillingFlag;
    };

    struct SequenceParameterSet
    {
        uint32_t sequenceParameterSetId = 0;
        uint16_t frameWidth = 0;
        uint16_t frameHeight = 0;
        uint16_t avgFrameRate = 0;
        uint32_t layerCountMinus1 = 0;

        bool avgFrameRatePresentFlag = false;
        bool enhancedOccupancyMapForDepthFlag = false;
        bool multipleLayerStreamsPresentFlag = false;
        bool pcmPatchEnabledFlag = false;
        bool pcmSeparateVideoPresentFlag = false;
        bool patchInterPredictionEnabledFlag = false;
        bool pixelDeinterleavingFlag = false;
        bool pointLocalReconstructionEnabledFlag = false;
        bool removeDuplicatePointEnabledFlag = false;
        bool projection45DegreeEnabledFlag = false;
        bool patchPrecedenceOrderFlag = false;

        std::vector<bool> layerAbsoluteCodingEnabledFlag;
        std::vector<size_t> layerPredictorIndexDiff;

        ProfileTierLevel profileTierLevel;
        GeometryInformation geometryInformation;
        OccupancyInformation occupancyInformation;
        AttributeInformation attributeInformation;
        PointLocalReconstructionInformation pointLocalReconstructionInformation;

        // THE NEXT PARAMETERS ARE NOT IN THE VPCC CD SYNTAX DOCUMENTS AND WILL BE REMOVE
        bool losslessGeo444;
        bool losslessGeo;
        bool losslessTexture;
        size_t surfaceThickness;
        uint8_t minLevel;
        // THE NEXT PARAMETERS ARE NOT IN THE VPCC CD SYNTAX DOCUMENTS AND WILL BE REMOVE
    };

    struct RefListStruct
    {
        uint8_t numRefEntries = 0;

        std::vector<uint8_t> absDeltaPfocSt;
        std::vector<uint8_t> pfocLsbLt;
        std::vector<bool> stRefPatchFrameFlag;
        std::vector<bool> strpfEntrySignFlag;
    };

    struct PatchSequenceParameterSet
    {
        uint8_t patchSequenceParameterSetId = 0;
        uint8_t log2PatchPackingBlockSize = 0;
        uint8_t log2MaxPatchFrameOrderCntLsb = 0;
        uint8_t maxDecPatchFrameBufferingMinus1 = 0;
        uint8_t numRefPatchFrameListsInPsps = 0;

        bool longTermRefPatchFramesFlag = false;
        bool useEightOrientationsFlag = false;
        bool normalAxisLimitsQuantizationEnabledFlag = false;
        bool normalAxisMaxDeltaValueEnabledFlag = false;

        std::vector<RefListStruct> refListStruct;
    };

    struct GeometryPatchParams
    {
        bool geometryPatchScaleParamsPresentFlag = false;
        bool geometryPatchOffsetParamsPresentFlag = false;
        bool geometryPatchRotationParamsPresentFlag = false;
        bool geometryPatchPointSizeInfoPresentFlag = false;
        bool geometryPatchPointShapeInfoPresentFlag = false;

        uint32_t geometryPatchScaleOnAxis[3] = { 0 };
        int32_t geometryPatchOffsetOnAxis[3] = { 0 };
        int32_t geometryPatchRotationXYZW[4] = { 0 };

        uint16_t geometryPatchPointSizeInfo = 0;
        uint32_t geometryPatchPointShapeInfo = 0;
    };

    struct GeometryPatchParameterSet
    {
        uint8_t geometryPatchParameterSetId = 0;
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
        uint8_t attributePatchParameterSetId = 0;
        uint8_t patchFrameAttributeParameterSetId = 0;
        uint8_t attributeDimensionMinus1 = 0;

        bool attributePatchParamsPresentFlag = false;

        AttributePatchParams attributePatchParams;
    };

    struct AttributeFrameParams
    {
        std::vector<bool> attributeSmoothingParamsPresentFlag;
        std::vector<uint8_t> attributeSmoothingGridSizeMinus2;
        std::vector<uint8_t> attributeSmoothingThreshold;
        std::vector<uint32_t> attributeSmoothingLocalEntropyThreshold;
        std::vector<uint8_t> attributeSmoothingThresholdAttributeVariation;
        std::vector<uint8_t> attributeSmoothingThresholdAttributeDifference;

        bool attributeScaleParamsPresentFlag = false;
        bool attributeOffsetParamsPresentFlag = false;

        std::vector<uint32_t> attributeScale;
        std::vector<int32_t> attributeOffset;
    };

    struct PatchFrameAttributeParameterSet
    {
        uint8_t patchFrameAttributeParameterSetId = 0;
        uint8_t patchSequencParameterSetId = 0;
        uint8_t attributeDimensionMinus1 = 3;

        bool attributePatchScaleParamsEnabledFlag = false;
        bool attributePatchOffsetParamsEnabledFlag = false;

        AttributeFrameParams attributeFrameParams;
    };

    struct PatchFrameTileInformation
    {
        bool singleTileInPatchFrameFlag = false;
        bool uniformTileSpacingFlag = false;

        uint32_t numTileColumnsMinus1 = 0;
        uint32_t numTileRowsMinus1 = 0;

        uint32_t singleTilePerTileGroupFlag = 0;
        uint32_t numTileGroupsInPatchFrameMinus1 = 0;

        bool signalledTileGroupIdFlag = false;
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
        uint8_t patchFrameParameterSetId = 0;
        uint8_t patchSequenceParameterSetId = 0;
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
        bool geometryScaleParamsPresentFlag = false;
        bool geometryOffsetParamsPresentFlag = false;
        bool geometryRotationParamsPresentFlag = false;
        bool geometryPointSizeInfoPresentFlag = false;
        bool geometryPointShapeInfoPresentFlag = false;
        bool geometrySmoothingEnabledFlag = false;

        uint8_t geometrySmoothingGridSizeMinus2 = 0;
        uint8_t geometrySmoothingThreshold = 0;

        uint32_t geometryScaleOnAxis[3] = { 0 };
        int32_t geometryOffsetOnAxis[3] = { 0 };
        int32_t geometryRotationXYZW[4] = { 0 };

        uint16_t geometryPointSizeInfo = 0;
        uint32_t geometryPointShapeInfo = 0;
    };

    struct PatchFrameGeometryParameterSet
    {
        uint8_t patchFrameGeometryParameterSetId = 0;
        uint8_t patchSequenceParameterSetId = 0;

        bool geometryPatchParamsEnabledFlag = false;
        bool overrideGeometryPatchParamsFlag = false;
        bool geometryPatchScaleParamsEnabledFlag = false;
        bool geometryPatchOffsetParamsEnabledFlag = false;
        bool geometryPatchRotationParamsEnabledFlag = false;
        bool geometryPatchPointSizeInfoEnabledFlag = false;
        bool geometryPatchPointShapeInfoEnabledFlag = false;

        GeometryFrameParams geometryFrameParams;
    };

    struct PatchTileGroupHeader
    {
        uint8_t frameIndex = 0;
        uint8_t patchFrameParameterSetId = 0;
        uint8_t type = 0;
        uint32_t address = 0;
        uint8_t patchFrameOrderCntLsb = 0;
        uint8_t refPatchFrameListIdx = 0;

        bool refPatchFrameListSpsFlag = false;

        std::vector<bool> additionalPfocLsbPresentFlag;
        std::vector<uint32_t> additionalPfocLsbVal;

        bool numRefIdxActiveOverrideFlag = false;

        uint8_t numRefIdxActiveMinus1 = 0;

        uint8_t normalAxisMinValueQuantizer = 0;
        uint8_t normalAxisMaxDeltaValueQuantizer = 0;

        uint8_t interPredictPatch2dShiftUBitCountMinus1 = 0;
        uint8_t interPredictPatch2dShiftVBitCountMinus1 = 0;
        uint8_t interPredictPatch2dDeltaSizeDBitCountMinus1 = 0;
        uint8_t interPredictPatch3dShiftTangentAxisBitCountMinus1 = 0;
        uint8_t interPredictPatch3dShiftBitangentAxisBitCountMinus1 = 0;
        uint8_t interPredictPatch3dShiftNormalAxisBitCountMinus1 = 0;
        uint8_t interPredictPatchLodBitCount = 0;

        bool interPredictPatchBitCountFlag = false;
        bool interPredictPatch2dShiftUBitCountFlag = false;
        bool interPredictPatch2dShiftVBitCountFlag = false;
        bool interPredictPatch3dShiftTangentAxisBitCountFlag = false;
        bool interPredictPatch3dShiftBitangentAxisBitCountFlag = false;
        bool interPredictPatch3dShiftNormalAxisBitCountFlag = false;
        bool interPredictPatchLodBitCountFlag = false;

        uint8_t pcm3dShiftAxisBitCountMinus1 = 9;

        bool pcm3dShiftBitCountPresentFlag = true;

        PatchTileGroupHeader()
        {
            additionalPfocLsbPresentFlag.resize(1, 0);
            additionalPfocLsbVal.resize(1, 0);
        }
    };

    struct PointLocalReconstructionData
    {
        size_t blockToPatchMapHeight = 0;
        size_t blockToPatchMapWidth = 0;
        bool levelFlag = false;
        bool presentFlag = false;
        uint8_t modeMinus1 = 0;
        std::vector<bool> blockPresentFlag;
        std::vector<uint8_t> blockModeMinus1;
    };

    struct PatchDataUnit
    {
        size_t shiftU = 0;  // sizes need to be determined
        size_t shiftV = 0;
        int64_t deltaSizeU = 0;
        int64_t deltaSizeV = 0;
        size_t shiftTangentAxis = 0;
        size_t shiftBiTangentAxis = 0;
        size_t shiftMinNormalAxis = 0;
        size_t shiftDeltaMaxNormalAxis = 255;
        Axis6::Enum projectPlane = Axis6::UNDEFINED;
        uint8_t orientationIndex = 0;
        uint8_t lod = 0;
        PointLocalReconstructionData pointLocalReconstructionData;
        bool projection45DegreePresentFlag = false;
        uint8_t projection45DegreeRotationAxis = 0;
        size_t patchIndex = 0;
        size_t frameIndex = 0;
    };

    struct DeltaPatchDataUnit
    {
        int64_t deltaPatchIndex = 0;

        int64_t deltaShiftU = 0;
        int64_t deltaShiftV = 0;

        int64_t deltaSizeU = 0;
        int64_t deltaSizeV = 0;

        int64_t deltaShiftTangentAxis = 0;
        int64_t deltaShiftBiTangentAxis = 0;
        int64_t deltaShiftMinNormalAxis = 0;

        int64_t shiftDeltaMaxNormalAxis = 0;
        Axis6::Enum projectPlane = Axis6::UNDEFINED;
        uint8_t lod = 0;
        size_t patchIndex = 0;
        size_t frameIndex = 0;
        PointLocalReconstructionData pointLocalReconstructionData;
    };

    struct PCMPatchDataUnit
    {
        bool patchInPcmVideoFlag = false;
        size_t shiftU = 0;
        size_t shiftV = 0;
        int64_t deltaSizeU = 0;
        int64_t deltaSizeV = 0;
        size_t shiftTangentAxis = 0;
        size_t shiftBiTangentAxis = 0;
        size_t shiftNormalAxis = 0;
        uint32_t pcmPoints = 0;
        size_t patchIndex = 0;
        size_t frameIndex = 0;
    };

    struct PatchInformationData
    {
        size_t frameIndex = 0;
        size_t patchIndex = 0;
        bool overrideGeometryPatchFlag = false;
        uint8_t geometryPatchParameterSetId = 0;

        std::vector<bool> overrideAttributePatchFlag;
        std::vector<uint8_t> attributePatchParameterSetId;

        PatchDataUnit patchDataUnit;
        DeltaPatchDataUnit deltaPatchDataUnit;
        PCMPatchDataUnit pcmPatchDataUnit;
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

    struct SeiPayload
    {
        // TODO: Defines as empty?
    };

    struct SeiMessage
    {
        uint8_t payloadTypeByte = 0;
        uint8_t payloadSizeByte = 0;

        std::vector<SeiPayload> seiPayload;
    };

    struct PatchDataGroup
    {
        PatchSequenceParameterSet patchSequenceParameterSet[16] = { 0 };
        GeometryPatchParameterSet geometryPatchParameterSet[64] = { 0 };
        AttributePatchParameterSet attributePatchParameterSet[64] = { 0 };
        PatchFrameParameterSet patchFrameParameterSet[64] = { 0 };
        PatchFrameAttributeParameterSet patchFrameAttributeParameterSet[64] = { 0 };
        PatchFrameGeometryParameterSet patchFrameGeometryParameterSet[64] = { 0 };

        std::vector<PatchTileGroupLayerUnit> patchTileGroupLayerUnit;
        std::vector<SeiMessage> seiMessagePrefix;
        std::vector<SeiMessage> seiMessageSuffix;

        size_t patchSequenceParameterSetSize = 0;
        size_t geometryPatchParameterSetSize = 0;
        size_t attributePatchParameterSetSize = 0;
        size_t patchFrameParameterSetSize = 0;
        size_t patchFrameAttributeParameterSetSize = 0;
        size_t patchFrameGeometryParameterSetSize = 0;
    };

    // Application data types

    struct Vector3U
    {
        uint32_t x;
        uint32_t y;
        uint32_t z;
    };

    struct Vector3I
    {
        int32_t x;
        int32_t y;
        int32_t z;
    };

    struct PointShape
    {
        enum Enum
        {
            CIRCLE = 0,
            SQUARE = 1,
            DIAMOND = 2,
        };
    };

    struct MetadataEnabledFlags
    {
        bool metadataEnabled = false;
        bool scaleEnabled = false;
        bool offsetEnabled = false;
        bool rotationEnabled = false;
        bool pointSizeEnabled = false;
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

    struct GPAPatchData
    {
        bool isMatched = false;
        bool isGlobalPatch = false;
        int globalPatchIndex = -1;
        size_t sizeU0 = 0;
        size_t sizeV0 = 0;
        size_t u0 = -1;
        size_t v0 = -1;
        size_t patchOrientation = -1;
        std::vector<bool> occupancy;
    };

    struct Patch
    {
        size_t index = 0;                       // patch index

        uint32_t u1 = 0;                        // tangential shift
        uint32_t v1 = 0;                        // bitangential shift

        int32_t d1 = 0;                         // depth shift

        uint32_t sizeD = 0;                     // size for depth
        uint32_t sizeU = 0;                     // size for depth
        uint32_t sizeV = 0;                     // size for depth

        uint32_t u0 = 0;                        // location in packed image
        uint32_t v0 = 0;                        // location in packed image

        uint32_t sizeU0 = 0;                    // size of occupancy map
        uint32_t sizeV0 = 0;                    // size of occupancy map

        uint32_t occupancyResolution = 0;       // occupancy map resolution

        uint32_t normalAxis = 0;                // x
        uint32_t tangentAxis = 0;               // y
        uint32_t bitangentAxis = 0;             // z

        uint32_t patchOrientation = 0;          // patch orientation in canvas atlas
        uint32_t projectionMode = 0;            // 0: related to the min depth value; 1: related to the max value
        uint32_t axisOfAdditionalPlane = 0;

        int32_t bestMatchIndex = 0;             // index of matched patch from pre-frame patch.
    };

    struct VideoFrame
    {
        size_t offset = 0;
        size_t length = 0;
    };

    struct Frame
    {
        size_t index;

        uint16_t width = 0;
        uint16_t height = 0;

        VideoFrame occupancy;

        VideoFrame geometry;

#if 0

        VideoFrame geometryD0;
        VideoFrame geometryD1;
        VideoFrame geometryMP;

#endif

        VideoFrame texture;

#if 0

        VideoFrame textureMP;

#endif

        int64_t presentationTimeUs = 0;

        std::vector<Patch> patches;
        std::vector<size_t> blockToPatch;

        // std::vector<MissedPointsPatch> missedPointsPatches;
    };

    typedef std::vector<uint8_t> VideoStream;
    typedef std::vector<Frame> FrameStream;

    struct FrameGroup
    {
        VideoStream occupancy;

        VideoStream geometry;
        VideoStream geometryD0;
        VideoStream geometryD1;
        VideoStream geometryMP;

        VideoStream texture;
        VideoStream textureMP;

        FrameStream frames;

        SequenceParameterSet sps;
        PatchDataGroup pdg;
    };

    struct ParserContext
    {
        SequenceParameterSet sps;
        PatchDataGroup pdg;
        VPCCParameterSet vpcc;

        FrameGroup* currentFrameGroup;

        int32_t previousPatchSizeU;
        int32_t previousPatchSizeV;
        
        int32_t predictionPatchIndex;
        int32_t predictionFramePatchTileGroupLayerUnitIndex;
    };
}

////////////////////////////////////////////////////////////////////////////////
/// Functionality
////////////////////////////////////////////////////////////////////////////////
namespace PCC
{
    bool parseContainerHeader(Bitstream& bitstream, TMC2Header& header);
    bool parse(Bitstream& bitstream, FrameGroup& frameGroup);

    int32_t patchBlockToCanvasBlock(Patch& patch, const size_t blockU, const size_t blockV, size_t canvasStrideBlk, size_t canvasHeightBlk);
    size_t patchToCanvas(Patch& patch, const size_t u, const size_t v, size_t canvasStride, size_t canvasHeight, size_t& x, size_t& y);
}

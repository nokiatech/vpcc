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

#include "VPCC/VPCCDecoder110.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <assert.h>

#include <string>
#include <vector>

#include "Logger.h"
#include "FileSystem.h"

#undef min
#undef max

#include "PCCCommon.h"
#include "PCCHighLevelSyntax.h"
#include "PCCBitstream.h"
#include "PCCVideoBitstream.h"
#include "PCCFrameContext.h"
#include "PCCPatch.h"
#include "PCCGroupOfFrames.h"
#include "PCCContext.h"

using namespace pcc;

namespace VPCC110
{
    int decode(PCCContext& context, int32_t atlasIndex);

    void generateBlockToPatchFromBoundaryBox(PCCContext& context, PCCFrameContext& frame, const size_t occupancyResolution);
    void generateBlockToPatchFromBoundaryBox(PCCContext& context, const size_t occupancyResolution);

    void setPointLocalReconstruction(PCCContext& context);
    void createPointLocalReconstructionData(PCCFrameContext& frame, PCCPatch& patch, PLRData& plrd, size_t occupancyPackingBlockSize);

    void createPatchFrameDataStructure(PCCContext& context );
    void createPatchFrameDataStructure(PCCContext& context, PCCFrameContext& frame, size_t frameIndex);

    int32_t patchBlockToCanvasBlock(Patch& patch, const size_t blockU, const size_t blockV, size_t canvasStrideBlock, size_t canvasHeightBlock)
    {
        size_t x = 0;
        size_t y = 0;

        switch (patch.patchOrientation)
        {
            case PatchOrientation::DEFAULT:
            {
                x = blockU + patch.u0;
                y = blockV + patch.v0;
                
                break;
            }

            case PatchOrientation::ROT90:
            {
                x = (patch.sizeV0 - 1 - blockV) + patch.u0;
                y = blockU + patch.v0;
                
                break;
            }

            case PatchOrientation::ROT180:
            {
                x = (patch.sizeU0 - 1 - blockU) + patch.u0;
                y = (patch.sizeV0 - 1 - blockV) + patch.v0;
                
                break;
            }

            case PatchOrientation::ROT270:
            {
                x = blockV + patch.u0;
                y = (patch.sizeU0 - 1 - blockU) + patch.v0;
                
                break;
            }

            case PatchOrientation::MIRROR:
            {
                x = (patch.sizeU0 - 1 - blockU) + patch.u0;
                y = blockV + patch.v0;
                
                break;
            }

            case PatchOrientation::MROT90:
            {
                x = (patch.sizeV0 - 1 - blockV) + patch.u0;
                y = (patch.sizeU0 - 1 - blockU) + patch.v0;
                
                break;
            }

            case PatchOrientation::MROT180:
            {
                x = blockU + patch.u0;
                y = (patch.sizeV0 - 1 - blockV) + patch.v0;
                
                break;
            }

            case PatchOrientation::MROT270:
            {
                x = blockV + patch.u0;
                y = blockU + patch.v0;
                
                break;
            }

            case PatchOrientation::SWAP:
            {
                x = blockV + patch.u0;
                y = blockU + patch.v0;
                
                break;
            }

            default:
            {
                return -1;
            }
        }

        if (x < 0) return -1;
        if (y < 0) return -1;
        if (x >= canvasStrideBlock) return -1;
        if (y >= canvasHeightBlock) return -1;

        return (x + canvasStrideBlock * y);
    }

    int decode(PCCContext* context, int32_t atlasIndex)
    {
        return decode(*context, atlasIndex);
    }

    int decode(PCCContext& context, int32_t atlasIndex)
    {
        createPatchFrameDataStructure( context );
        
        V3CParameterSet& sps = context.getVps();
        AttributeInformation& ai = sps.getAttributeInformation(atlasIndex);

        PCCCodecId attributeCodecId = UNKNOWN_CODEC;
        
        for (uint32_t i = 0; i < ai.getAttributeCount(); i++)
        {
            attributeCodecId = (PCCCodecId)ai.getAttributeCodecId(i);
        }

        std::vector<std::vector<bool>> absoluteT1List;
        absoluteT1List.resize(ai.getAttributeCount());
        
        for (uint8_t attributeIndex = 0; attributeIndex < ai.getAttributeCount(); ++attributeIndex)
        {
            absoluteT1List[attributeIndex].resize(sps.getMapCountMinus1(atlasIndex) + 1);
            
            if (ai.getAttributeMapAbsoluteCodingPersistenceFlag(attributeIndex) != 0u)
            {
                for (uint32_t mapIndex = 0; mapIndex < sps.getMapCountMinus1(attributeIndex) + 1; ++mapIndex)
                {
                    absoluteT1List[attributeIndex][mapIndex] = true;
                }
            }
            else
            {
                for (uint32_t mapIndex = 0; mapIndex < sps.getMapCountMinus1(atlasIndex) + 1; ++mapIndex)
                {
                    absoluteT1List[attributeIndex][mapIndex] = sps.getMapAbsoluteCodingEnableFlag(atlasIndex, mapIndex);
                }
            }
        }
        
        for (PCCFrameContext& frame : context.getFrames())
        {
            generateBlockToPatchFromBoundaryBox(context, frame, context.getOccupancyPackingBlockSize());
        }
        
        return 0;
    }

    void generateBlockToPatchFromBoundaryBox(PCCContext& context, PCCFrameContext& frame, const size_t occupancyResolution)
    {
        std::vector<PCCPatch>& patches = frame.getPatches();
        const size_t patchCount = patches.size();
        
        const size_t blockToPatchWidth  = frame.getWidth() / occupancyResolution;
        const size_t blockToPatchHeight = frame.getHeight() / occupancyResolution;
        
        const size_t blockCount = blockToPatchWidth * blockToPatchHeight;
        
        std::vector<size_t>& blockToPatch = frame.getBlockToPatch();
        blockToPatch.resize(blockCount);
        
        std::fill(blockToPatch.begin(), blockToPatch.end(), 0);
        
        for (size_t patchIndex = 0; patchIndex < patchCount; ++patchIndex)
        {
            PCCPatch& patch = patches[patchIndex];
            
            for (size_t v0 = 0; v0 < patch.getSizeV0(); ++v0)
            {
                for (size_t u0 = 0; u0 < patch.getSizeU0(); ++u0)
                {
                    const size_t blockIndex = patch.patchBlock2CanvasBlock(u0, v0, blockToPatchWidth, blockToPatchHeight);
                    
                    if (context.getAtlasSequenceParameterSet(0).getPatchPrecedenceOrderFlag())
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

    void generateBlockToPatchFromBoundaryBox(PCCContext& context, const size_t occupancyResolution)
    {
        for (PCCFrameContext& frame : context.getFrames())
        {
            generateBlockToPatchFromBoundaryBox(context, frame, occupancyResolution);
        }
    }

    void setPointLocalReconstruction(PCCContext& context)
    {
        AtlasSequenceParameterSetRbsp& asps = context.getAtlasSequenceParameterSet(0);
        
        PointLocalReconstructionMode mode = { false, false, 0, 1 };
        context.addPointLocalReconstructionMode(mode);
        
        if (asps.getPLREnabledFlag())
        {
            PLRInformation& plri = asps.getPLRInformation(0);
            
            for (size_t i = 0; i < plri.getNumberOfModesMinus1(); i++)
            {
                mode.interpolate_ = plri.getInterpolateFlag(i);
                mode.filling_ = plri.getFillingFlag(i);
                mode.minD1_ = plri.getMinimumDepth(i);
                mode.neighbor_ = plri.getNeighbourMinus1(i) + 1;
                
                context.addPointLocalReconstructionMode(mode);
            }
        }
    }

    void createPointLocalReconstructionData(PCCFrameContext& frame, PCCPatch& patch, PLRData& plrd, size_t occupancyPackingBlockSize)
    {
        patch.allocOneLayerData();
        patch.getPointLocalReconstructionLevel() = (uint8_t)plrd.getLevelFlag();
        
        if (plrd.getLevelFlag())
        {
            if (plrd.getPresentFlag())
            {
                patch.getPointLocalReconstructionMode() = plrd.getModeMinus1() + 1;
            }
            else
            {
                patch.getPointLocalReconstructionMode() = 0;
            }
        }
        else
        {
            for (size_t v0 = 0; v0 < plrd.getBlockToPatchMapHeight(); ++v0)
            {
                for (size_t u0 = 0; u0 < plrd.getBlockToPatchMapWidth(); ++u0)
                {
                    size_t index = v0 * plrd.getBlockToPatchMapWidth() + u0;
                    
                    if (plrd.getBlockPresentFlag(index))
                    {
                        patch.getPointLocalReconstructionMode(u0, v0) = plrd.getBlockModeMinus1(index) + 1;
                    }
                    else
                    {
                        patch.getPointLocalReconstructionMode(u0, v0) = 0;
                    }
                }
            }
        }
    }

    void createPatchFrameDataStructure(PCCContext& context)
    {
        V3CParameterSet& sps = context.getVps();
        
        size_t atlasIndex = context.getAtlasIndex();
        AtlasSequenceParameterSetRbsp& asps = context.getAtlasSequenceParameterSet();
        
        std::vector<std::vector<AtlasTileLayerRbsp>>& atglulist  = context.getAtlasTileLayerList();
        
        context.resize(atglulist.size());
        
        setPointLocalReconstruction(context);
        
        context.setRawGeoWidth(64);
        context.setRawAttWidth(0);
        context.setRawGeoHeight(0);
        context.setRawAttHeight(0);
        
        for (size_t i = 0; i < context.size(); i++)
        {
            PCCFrameContext& frame = context.getFrame(i);
            frame.setAFOC(i);
            frame.setIndex(i);
            frame.setWidth(sps.getFrameWidth(atlasIndex));
            frame.setHeight(sps.getFrameHeight(atlasIndex));
            frame.setUseRawPointsSeparateVideo(sps.getAuxiliaryVideoPresentFlag(atlasIndex));
            frame.setRawPatchEnabledFlag(asps.getRawPatchEnabledFlag());
            
            createPatchFrameDataStructure(context, frame, i);
        }
    }

    void createPatchFrameDataStructure(PCCContext& context, PCCFrameContext& frame, size_t frameIndex)
    {
        V3CParameterSet& sps = context.getVps();
        
        size_t atlasIndex = context.getAtlasIndex();
        GeometryInformation& gi = sps.getGeometryInformation(atlasIndex);
        
        for (size_t tileIndex = 0; tileIndex < 1; tileIndex++)
        {
            AtlasTileLayerRbsp& atglu = context.getAtlasTileLayer(frameIndex, tileIndex);
            AtlasTileHeader& ath = atglu.getHeader();
            AtlasFrameParameterSetRbsp& afps = context.getAtlasFrameParameterSet(ath.getAtlasFrameParameterSetId());
            AtlasSequenceParameterSetRbsp& asps = context.getAtlasSequenceParameterSet(afps.getAtlasSequenceParameterSetId());
            AtlasTileDataUnit& atgdu = atglu.getDataUnit();
            
            if (frameIndex > 0 && ath.getType() != I_TILE)
            {
                frame.setRefAfocList( context, ath, ath.getAtlasFrameParameterSetId() );
            }
            
            std::vector<PCCPatch>& patches = frame.getPatches();
            std::vector<PCCRawPointsPatch>& pcmPatches = frame.getRawPointsPatches();
            std::vector<PCCEomPatch>& eomPatches = frame.getEomPatches();
            
            int64_t previousSizeU0 = 0;
            int64_t previousSizeV0 = 0;
            
            int64_t previousPatchSize2DXInPixel = 0;
            int64_t previousPatchSize2DYInPixel = 0;
            
            int64_t predictionIndex = 0;
            
            const size_t minLevel = ::pow(2.0, ath.getPosMinZQuantizer());
            
            size_t numRawPatches = 0;
            size_t numNonRawPatch = 0;
            size_t numEomPatch = 0;
            
            PCCTileType tileType = ath.getType();
            size_t patchCount = atgdu.getPatchCount();
            
            for (size_t i = 0; i < patchCount; i++)
            {
                PCCPatchType currentPatchType = getPatchType(tileType, atgdu.getPatchMode(i));
                
                if (currentPatchType == RAW_PATCH)
                {
                    numRawPatches++;
                }
                else if (currentPatchType == EOM_PATCH)
                {
                    numEomPatch++;
                }
            }
            
            numNonRawPatch = patchCount - numRawPatches - numEomPatch;
            
            eomPatches.reserve(numEomPatch);
            patches.resize(numNonRawPatch);
            pcmPatches.resize(numRawPatches);
            
            size_t totalNumberOfRawPoints = 0;
            
            int32_t packingBlockSize = context.getOccupancyPackingBlockSize();
            
            int32_t quantizerSizeX = 1 << ath.getPatchSizeXinfoQuantizer();
            int32_t quantizerSizeY = 1 << ath.getPatchSizeYinfoQuantizer();
            
            frame.setLog2PatchQuantizerSizeX(ath.getPatchSizeXinfoQuantizer());
            frame.setLog2PatchQuantizerSizeY(ath.getPatchSizeYinfoQuantizer());
            
            for (size_t patchIndex = 0; patchIndex < patchCount; patchIndex++)
            {
                PatchInformationData& pid = atgdu.getPatchInformationData(patchIndex);
                PCCPatchType currentPatchType = getPatchType(tileType, atgdu.getPatchMode(patchIndex));
                
                if (currentPatchType == INTRA_PATCH)
                {
                    PatchDataUnit& pdu = pid.getPatchDataUnit();
                    
                    PCCPatch& patch = patches[patchIndex];
                    patch.getOccupancyResolution() = context.getOccupancyPackingBlockSize();
                    patch.getU0() = pdu.get2dPosX();
                    patch.getV0() = pdu.get2dPosY();
                    patch.getU1() = pdu.get3dOffsetX();
                    patch.getV1() = pdu.get3dOffsetY();
                    
                    bool lodEnableFlag = pdu.getLodEnableFlag();
                    
                    if (lodEnableFlag)
                    {
                        patch.setLodScaleX(pdu.getLodScaleXminus1() + 1);
                        patch.setLodScaleYIdc(pdu.getLodScaleYIdc() + (patch.getLodScaleX() > 1 ? 1 : 2));
                    }
                    else
                    {
                        patch.setLodScaleX(1);
                        patch.setLodScaleYIdc(1);
                    }
                    
                    patch.getSizeD() = (std::min)(pdu.get3dRangeZ() * minLevel, (size_t)255);
                    
                    if (asps.getPatchSizeQuantizerPresentFlag())
                    {
                        patch.setPatchSize2DXInPixel(pdu.get2dSizeXMinus1() * quantizerSizeX + 1);
                        patch.setPatchSize2DYInPixel(pdu.get2dSizeYMinus1() * quantizerSizeY + 1);
                        
                        patch.getSizeU0() = ::ceil((double)patch.getPatchSize2DXInPixel() / (double)packingBlockSize);
                        patch.getSizeV0() = ::ceil((double)patch.getPatchSize2DYInPixel() / (double)packingBlockSize);
                    }
                    else
                    {
                        patch.getSizeU0() = pdu.get2dSizeXMinus1() + 1;
                        patch.getSizeV0() = pdu.get2dSizeYMinus1() + 1;
                    }
                    
                    patch.getPatchOrientation() = pdu.getOrientationIndex();
                    patch.setViewId(pdu.getProjectionId());

                    const size_t max3DCoordinate = size_t(1) << (gi.getGeometry3dCoordinatesBitdepthMinus1() + 1);
                    
                    if (patch.getProjectionMode() == 0)
                    {
                        patch.getD1() = pdu.get3dOffsetMinZ() * minLevel;
                    }
                    else
                    {
                        if (asps.getExtendedProjectionEnabledFlag() == false)
                        {
                            patch.getD1() = max3DCoordinate - pdu.get3dOffsetMinZ() * minLevel;
                        }
                        else
                        {
#if EXPAND_RANGE_ENCODER
                            
                            patch.getD1() = (max3DCoordinate) - pdu.get3dOffsetMinZ() * minLevel;
                            
#else
                            
                            patch.getD1() = (max3DCoordinate << 1) - pdu.get3dOffsetMinZ() * minLevel;
                            
#endif
                        }
                    }
                    
                    previousSizeU0 = patch.getSizeU0();
                    previousSizeV0 = patch.getSizeV0();
                    
                    previousPatchSize2DXInPixel = patch.getPatchSize2DXInPixel();
                    previousPatchSize2DYInPixel = patch.getPatchSize2DYInPixel();
                    
                    if (patch.getNormalAxis() == 0)
                    {
                        patch.getTangentAxis() = 2;
                        patch.getBitangentAxis() = 1;
                    }
                    else if (patch.getNormalAxis() == 1)
                    {
                        patch.getTangentAxis() = 2;
                        patch.getBitangentAxis() = 0;
                    }
                    else
                    {
                        patch.getTangentAxis() = 0;
                        patch.getBitangentAxis() = 1;
                    }

                    patch.allocOneLayerData();
                    
                    if (asps.getPLREnabledFlag())
                    {
                        createPointLocalReconstructionData(frame, patch, pdu.getPLRData(), context.getOccupancyPackingBlockSize());
                    }
                }
                else if (currentPatchType == INTER_PATCH)
                {
                    InterPatchDataUnit& ipdu = pid.getInterPatchDataUnit();
                    
                    PCCPatch& patch = patches[patchIndex];
                    patch.setBestMatchIdx((int32_t)(ipdu.getRefPatchIndex() + predictionIndex));
                    predictionIndex += ipdu.getRefPatchIndex() + 1;
                    patch.setRefAtlasFrameIndex(ipdu.getRefIndex());

                    size_t refFrameIndex = (size_t)frame.getRefAfoc(patch.getRefAtlasFrameIndex());
                    
                    PCCFrameContext& refFrame = context.getFrame(refFrameIndex);
                    std::vector<PCCPatch>& refPatchList = refFrame.getPatches();
                    
                    const PCCPatch& refPatch = refPatchList.at(patch.getBestMatchIdx());
                    
                    patch.getOccupancyResolution() = context.getOccupancyPackingBlockSize();
                    patch.getProjectionMode() = refPatch.getProjectionMode();
                    patch.getU0() = ipdu.get2dPosX() + refPatch.getU0();
                    patch.getV0() = ipdu.get2dPosY() + refPatch.getV0();
                    patch.getPatchOrientation() = refPatch.getPatchOrientation();
                    patch.getU1() = ipdu.get3dOffsetX() + refPatch.getU1();
                    patch.getV1() = ipdu.get3dOffsetY() + refPatch.getV1();
                    
                    if (asps.getPatchSizeQuantizerPresentFlag())
                    {
                        patch.setPatchSize2DXInPixel(refPatch.getPatchSize2DXInPixel() + (ipdu.get2dDeltaSizeX()) * quantizerSizeX);
                        patch.setPatchSize2DYInPixel(refPatch.getPatchSize2DYInPixel() + (ipdu.get2dDeltaSizeY()) * quantizerSizeY);
                        
                        patch.getSizeU0() = ::ceil((double)patch.getPatchSize2DXInPixel() / (double)packingBlockSize);
                        patch.getSizeV0() = ::ceil((double)patch.getPatchSize2DYInPixel() / (double)packingBlockSize);
                    }
                    else
                    {
                        patch.getSizeU0() = ipdu.get2dDeltaSizeX() + refPatch.getSizeU0();
                        patch.getSizeV0() = ipdu.get2dDeltaSizeY() + refPatch.getSizeV0();
                    }
                    
                    patch.getNormalAxis() = refPatch.getNormalAxis();
                    patch.getTangentAxis() = refPatch.getTangentAxis();
                    patch.getBitangentAxis() = refPatch.getBitangentAxis();
                    patch.getAxisOfAdditionalPlane() = refPatch.getAxisOfAdditionalPlane();
                    
                    const size_t max3DCoordinate = size_t(1) << (gi.getGeometry3dCoordinatesBitdepthMinus1() + 1);
                    
                    if (patch.getProjectionMode() == 0)
                    {
                        patch.getD1() = (ipdu.get3dOffsetMinZ() + (refPatch.getD1() / minLevel)) * minLevel;
                    }
                    else
                    {
                        if (asps.getExtendedProjectionEnabledFlag() == false)
                        {
                            patch.getD1() = max3DCoordinate - (ipdu.get3dOffsetMinZ() + ((max3DCoordinate - refPatch.getD1()) / minLevel)) * minLevel;
                        }
                        else
                        {
#if EXPAND_RANGE_ENCODER

                            patch.getD1() = (max3DCoordinate) - (ipdu.get3dOffsetMinZ() + ((max3DCoordinate - refPatch.getD1()) / minLevel)) * minLevel;

#else

                            patch.getD1() = (max3DCoordinate << 1) - (ipdu.get3dOffsetMinZ() + (((max3DCoordinate << 1) - refPatch.getD1()) / minLevel)) * minLevel;

#endif
                        }
                    }
                    
                    const int64_t deltaDD = ipdu.get3dRangeZ();
                    size_t previousDD = refPatch.getSizeD() / minLevel;
                    
                    if (previousDD * minLevel != refPatch.getSizeD())
                    {
                        previousDD += 1;
                    }
                    
                    patch.getSizeD() = (std::min)(size_t((deltaDD + previousDD) * minLevel), (size_t)255);
                    
                    patch.setLodScaleX(refPatch.getLodScaleX());
                    patch.setLodScaleYIdc(refPatch.getLodScaleYIdc());
                    
                    previousSizeU0 = patch.getSizeU0();
                    previousSizeV0 = patch.getSizeV0();
                    
                    previousPatchSize2DXInPixel = patch.getPatchSize2DXInPixel();
                    previousPatchSize2DYInPixel = patch.getPatchSize2DYInPixel();
                    
                    patch.allocOneLayerData();
                    
                    if (asps.getPLREnabledFlag())
                    {
                        createPointLocalReconstructionData(frame, patch, ipdu.getPLRData(), context.getOccupancyPackingBlockSize());
                    }
                }
                else if (currentPatchType == MERGE_PATCH)
                {
                    assert(false);
                }
                else if (currentPatchType == SKIP_PATCH)
                {
                    assert(false);
                }
                else if (currentPatchType == RAW_PATCH)
                {
                    RawPatchDataUnit& rpdu = pid.getRawPatchDataUnit();
                    
                    PCCRawPointsPatch& rawPointsPatch = pcmPatches[patchIndex - numNonRawPatch];
                    rawPointsPatch.u0_ = rpdu.get2dPosX();
                    rawPointsPatch.v0_ = rpdu.get2dPosY();
                    rawPointsPatch.sizeU0_ = rpdu.get2dSizeXMinus1() + 1;
                    rawPointsPatch.sizeV0_ = rpdu.get2dSizeYMinus1() + 1;
                    
                    if (afps.getRaw3dPosBitCountExplicitModeFlag())
                    {
                        rawPointsPatch.u1_ = rpdu.get3dOffsetX();
                        rawPointsPatch.v1_ = rpdu.get3dOffsetY();
                        rawPointsPatch.d1_ = rpdu.get3dOffsetZ();
                    }
                    else
                    {
                        const size_t pcmU1V1D1Level = size_t(1) << (gi.getGeometry2dBitdepthMinus1() + 1);
                        
                        rawPointsPatch.u1_ = rpdu.get3dOffsetX() * pcmU1V1D1Level;
                        rawPointsPatch.v1_ = rpdu.get3dOffsetY() * pcmU1V1D1Level;
                        rawPointsPatch.d1_ = rpdu.get3dOffsetZ() * pcmU1V1D1Level;
                    }
                    
                    rawPointsPatch.setNumberOfRawPoints(rpdu.getRawPointsMinus1() + 1);
                    rawPointsPatch.occupancyResolution_ = context.getOccupancyPackingBlockSize();
                    
                    totalNumberOfRawPoints += rawPointsPatch.getNumberOfRawPoints();
                }
                else if (currentPatchType == EOM_PATCH)
                {
                    EOMPatchDataUnit& epdu = pid.getEomPatchDataUnit();
                    
                    PCCEomPatch eomPatch;
                    eomPatch.u0_ = epdu.get2dPosX();
                    eomPatch.v0_ = epdu.get2dPosY();
                    eomPatch.sizeU_ = epdu.get2dSizeXMinus1() + 1;
                    eomPatch.sizeV_ = epdu.get2dSizeYMinus1() + 1;
                    eomPatch.eomCount_ = 0;
                    
                    eomPatch.memberPatches.resize(epdu.getPatchCountMinus1() + 1);
                    eomPatch.eomCountPerPatch.resize(epdu.getPatchCountMinus1() + 1);
                    
                    for (size_t i = 0; i < eomPatch.memberPatches.size(); i++)
                    {
                        eomPatch.memberPatches[i] = epdu.getAssociatedPatchesIdx(i);
                        eomPatch.eomCountPerPatch[i] = epdu.getPoints(i);
                        eomPatch.eomCount_ += eomPatch.eomCountPerPatch[i];
                    }
                    
                    std::vector<PCCEomPatch>& eomPatches = frame.getEomPatches();
                    eomPatches.push_back(eomPatch);
                    
                    frame.setTotalNumberOfEOMPoints(eomPatch.eomCount_);
                }
                else if (currentPatchType == END_PATCH)
                {
                    break;
                }
                else
                {
                    assert(false);
                }
            }

            frame.setTotalNumberOfRawPoints(totalNumberOfRawPoints);
        }
    }
}

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

#include "VPCC/VPCCDecoder130.h"

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

namespace VPCC130
{
    int decodePatches(PCCContext& context, int32_t atlasIndex);

    void generateBlockToPatchFromBoundaryBox(PCCContext& context, PCCFrameContext& frame, const size_t occupancyResolution);
    void generateBlockToPatchFromBoundaryBox(PCCContext& context, const size_t occupancyResolution);

    void createPatchFrameDataStructure(PCCContext& context);
    void createPatchFrameDataStructure(PCCContext& context, size_t atglOrder);

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

    int decodePatches(PCCContext* context, int32_t atlasIndex)
    {
        return decodePatches(*context, atlasIndex);
    }

    int decodePatches(PCCContext& context, int32_t atlasIndex)
    {
        createPatchFrameDataStructure(context);
        
        AtlasSequenceParameterSetRbsp& asps = context.getAtlasSequenceParameterSet(0);
        
        const int32_t occupancyResolution = size_t(1) << asps.getLog2PatchPackingBlockSize();
        const size_t frameCount = context.size();
        
        for (size_t frameIndex = 0; frameIndex < frameCount; frameIndex++)
        {
            for (size_t tileIndex = 0; tileIndex < context[frameIndex].getNumTilesInAtlasFrame(); tileIndex++)
            {
                PCCFrameContext& tile = context[frameIndex].getTile(tileIndex);
                
                generateBlockToPatchFromBoundaryBox(context, tile, occupancyResolution);
            }
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

    void createPatchFrameDataStructure(PCCContext& context)
    {
        std::vector<AtlasTileLayerRbsp>& atglulist = context.getAtlasTileLayerList();
        
        size_t frameCount = 0;

        for (size_t i = 0; i < atglulist.size(); i++)
        {
            frameCount = std::max(frameCount, (context.calculateAFOCval(atglulist, i) + 1));
        }

        context.resize(frameCount);

        for (size_t atglOrder = 0; atglOrder < atglulist.size(); atglOrder++)
        {
            createPatchFrameDataStructure(context, atglOrder);
        }
    }

    size_t setTileSizeAndLocation(PCCContext& context, size_t frameIndex, AtlasTileHeader& ath)
    {
        size_t afpsId = ath.getAtlasFrameParameterSetId();
        
        AtlasFrameParameterSetRbsp& afps = context.getAtlasFrameParameterSet(afpsId);
        AtlasSequenceParameterSetRbsp& asps = context.getAtlasSequenceParameterSet(afps.getAtlasSequenceParameterSetId());
        AtlasFrameTileInformation& afti = afps.getAtlasFrameTileInformation();
        
        size_t tileIndex = 0;
        
        PCCAtlasFrameContext& atlasFrameContext = context.getFrame(frameIndex);

        if (afti.getSingleTileInAtlasFrameFlag())
        {
            if (atlasFrameContext.getNumTilesInAtlasFrame() == 0)
            {
                atlasFrameContext.setAtlasFrameWidth(asps.getFrameWidth());
                atlasFrameContext.setAtlasFrameHeight(asps.getFrameHeight());
                atlasFrameContext.setNumTilesInAtlasFrame(1);
                
                atlasFrameContext.updatePartitionInfoPerFrame(frameIndex,
                                                                asps.getFrameWidth(),
                                                                asps.getFrameHeight(),
                                                                afti.getNumTilesInAtlasFrameMinus1() + 1,
                                                                afti.getUniformPartitionSpacingFlag(),
                                                                afti.getPartitionColumnWidthMinus1(0) + 1,
                                                                afti.getPartitionRowHeightMinus1(0) + 1,
                                                                afti.getNumPartitionColumnsMinus1() + 1,
                                                                afti.getNumPartitionRowsMinus1() + 1,
                                                                afti.getSinglePartitionPerTileFlag(),
                                                                afti.getSignalledTileIdFlag());
            }
            
            PCCFrameContext& tile = atlasFrameContext.getTile(0);
            tile.setTileIndex(tileIndex);
            tile.setLeftTopXInFrame(0);
            tile.setLeftTopYInFrame(0);
            tile.setWidth(asps.getFrameWidth());
            tile.setHeight(asps.getFrameHeight());
        }
        else
        {
            if (atlasFrameContext.getNumTilesInAtlasFrame() == 0)
            {
                atlasFrameContext.updatePartitionInfoPerFrame(frameIndex,
                                                                asps.getFrameWidth(),
                                                                asps.getFrameHeight(),
                                                                afti.getNumTilesInAtlasFrameMinus1() + 1,
                                                                afti.getUniformPartitionSpacingFlag(),
                                                                afti.getPartitionColumnWidthMinus1(0) + 1,
                                                                afti.getPartitionRowHeightMinus1(0) + 1,
                                                                afti.getNumPartitionColumnsMinus1() + 1,
                                                                afti.getNumPartitionRowsMinus1() + 1,
                                                                afti.getSinglePartitionPerTileFlag(),
                                                                afti.getSignalledTileIdFlag());
                
                atlasFrameContext.initNumTiles(atlasFrameContext.getNumTilesInAtlasFrame());
            }

            tileIndex = afti.getSignalledTileIdFlag() ? afti.getTileId(ath.getId()) : ath.getId();
            
            PCCFrameContext& tile = atlasFrameContext.getTile(tileIndex);
            
            const size_t topLeftPartitionColumn = afti.getTopLeftPartitionIdx(tileIndex) % (afti.getNumPartitionColumnsMinus1() + 1);
            const size_t topLeftPartitionRow = afti.getTopLeftPartitionIdx(tileIndex) / (afti.getNumPartitionColumnsMinus1() + 1);
            const size_t bottomRightPartitionColumn = topLeftPartitionColumn + afti.getBottomRightPartitionColumnOffset(tileIndex);
            const size_t bottomRightPartitionRow = topLeftPartitionRow + afti.getBottomRightPartitionRowOffset(tileIndex);

            size_t tileStartX = atlasFrameContext.getPartitionPosX(topLeftPartitionColumn);
            size_t tileStartY = atlasFrameContext.getPartitionPosY(topLeftPartitionRow);
            
            size_t tileWidth = 0;
            size_t tileHeight = 0;
            
            for (size_t j = topLeftPartitionColumn; j <= bottomRightPartitionColumn; j++)
            {
                tileWidth += atlasFrameContext.getPartitionWidth(j);
            }
            
            for (size_t j = topLeftPartitionRow; j <= bottomRightPartitionRow; j++)
            {
                tileHeight += atlasFrameContext.getPartitionHeight(j);
            }
            
            tile.setLeftTopXInFrame(tileStartX);
            tile.setLeftTopYInFrame(tileStartY);

            if ((tile.getLeftTopXInFrame() + tileWidth) >= atlasFrameContext.getAtlasFrameWidth())
            {
                tileWidth = context[0].getAtlasFrameWidth() - tile.getLeftTopXInFrame();
            }
                
            if ((tile.getLeftTopYInFrame() + tileHeight) >= atlasFrameContext.getAtlasFrameHeight())
            {
                tileHeight = context[0].getAtlasFrameHeight() - tile.getLeftTopYInFrame();
            }

            tile.setWidth(tileWidth);
            tile.setHeight(tileHeight);
        }

        return tileIndex;
    }

    void createPatchFrameDataStructure(PCCContext& context, size_t atglOrder)
    {
        V3CParameterSet& sps = context.getVps();
        
        size_t atlasIndex = context.getAtlasIndex();
        GeometryInformation& gi = sps.getGeometryInformation(atlasIndex);
        
        AtlasTileLayerRbsp& atlu = context.getAtlasTileLayer(atglOrder);
        AtlasTileHeader& ath = atlu.getHeader();

        AtlasFrameParameterSetRbsp& afps = context.getAtlasFrameParameterSet(ath.getAtlasFrameParameterSetId());
        AtlasSequenceParameterSetRbsp& asps = context.getAtlasSequenceParameterSet(afps.getAtlasSequenceParameterSetId());
        AtlasTileDataUnit& atgdu = atlu.getDataUnit();

        size_t frameIndex = atlu.getAtlasFrmOrderCntVal();
        size_t tileIndex = setTileSizeAndLocation(context, frameIndex, ath);

        PCCFrameContext& tile = context[frameIndex].getTile(tileIndex);
        tile.setFrameIndex(atlu.getAtlasFrmOrderCntVal());
        tile.setTileIndex(tileIndex);
        tile.setAtlIndex(atglOrder);
        tile.setUseRawPointsSeparateVideo(sps.getAuxiliaryVideoPresentFlag(atlasIndex) && asps.getAuxiliaryVideoEnabledFlag());
        tile.setRawPatchEnabledFlag(asps.getRawPatchEnabledFlag());

        if (tile.getFrameIndex() > 0 && ath.getType() != I_TILE)
        {
            tile.setRefAfocList(context, ath, ath.getAtlasFrameParameterSetId());
        }
        
        std::vector<PCCPatch>& patches = tile.getPatches();
        std::vector<PCCRawPointsPatch>& pcmPatches = tile.getRawPointsPatches();
        std::vector<PCCEomPatch>& eomPatches = tile.getEomPatches();
        
        int64_t previousSizeU0 = 0;
        int64_t previousSizeV0 = 0;
        
        int64_t previousPatchSize2DXInPixel = 0;
        int64_t previousPatchSize2DYInPixel = 0;
        
        int64_t predictionIndex = 0;
        
        const size_t minLevel = ::pow(2.0, ath.getPosMinDQuantizer());
        
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
        size_t totalNumberOfEomPoints = 0;
        
        int32_t packingBlockSize = 1 << asps.getLog2PatchPackingBlockSize();
        
        int32_t quantizerSizeX = 1 << ath.getPatchSizeXinfoQuantizer();
        int32_t quantizerSizeY = 1 << ath.getPatchSizeYinfoQuantizer();
        
        tile.setLog2PatchQuantizerSizeX(ath.getPatchSizeXinfoQuantizer());
        tile.setLog2PatchQuantizerSizeY(ath.getPatchSizeYinfoQuantizer());
        
        for (size_t patchIndex = 0; patchIndex < patchCount; patchIndex++)
        {
            PatchInformationData& pid = atgdu.getPatchInformationData(patchIndex);
            PCCPatchType currentPatchType = getPatchType(tileType, atgdu.getPatchMode(patchIndex));
            
            if (currentPatchType == INTRA_PATCH)
            {
                PatchDataUnit& pdu = pid.getPatchDataUnit();
                
                PCCPatch& patch = patches[patchIndex];
                patch.getOccupancyResolution() = size_t(1) << asps.getLog2PatchPackingBlockSize();
                patch.getU0() = pdu.get2dPosX();
                patch.getV0() = pdu.get2dPosY();
                patch.getU1() = pdu.get3dOffsetU();
                patch.getV1() = pdu.get3dOffsetV();
                
                bool lodEnableFlag = pdu.getLodEnableFlag();
                
                if (lodEnableFlag)
                {
                    patch.setLodScaleX(pdu.getLodScaleXMinus1() + 1);
                    patch.setLodScaleYIdc(pdu.getLodScaleYIdc() + (patch.getLodScaleX() > 1 ? 1 : 2));
                }
                else
                {
                    patch.setLodScaleX(1);
                    patch.setLodScaleYIdc(1);
                }
                
                patch.getSizeD() = (std::min)(pdu.get3dRangeD() * minLevel, (size_t)255);
                
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
                    patch.getD1() = pdu.get3dOffsetD() * minLevel;
                }
                else
                {
                    if (asps.getExtendedProjectionEnabledFlag() == false)
                    {
                        patch.getD1() = max3DCoordinate - pdu.get3dOffsetD() * minLevel;
                    }
                    else
                    {
                        patch.getD1() = (max3DCoordinate) - pdu.get3dOffsetD() * minLevel;
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
            }
            else if (currentPatchType == INTER_PATCH)
            {
                InterPatchDataUnit& ipdu = pid.getInterPatchDataUnit();
                
                PCCPatch& patch = patches[patchIndex];
                patch.getOccupancyResolution() = size_t(1) << asps.getLog2PatchPackingBlockSize();
                patch.setBestMatchIdx((int32_t)(ipdu.getRefPatchIndex() + predictionIndex));
                patch.setRefAtlasFrameIndex(ipdu.getRefIndex());
                
                predictionIndex += ipdu.getRefPatchIndex() + 1;
                
                size_t refFrameIndex = (size_t)tile.getRefAfoc(patch.getRefAtlasFrameIndex());
                
                PCCAtlasFrameContext refAtlasFrame = context.getFrame(refFrameIndex);
                PCCFrameContext refFrame = refAtlasFrame.getTile(tileIndex);
                std::vector<PCCPatch>& refPatchList = refFrame.getPatches();
                
                const PCCPatch& refPatch = refPatchList.at(patch.getBestMatchIdx());
                
                patch.getOccupancyResolution() = size_t(1) << asps.getLog2PatchPackingBlockSize();
                patch.getProjectionMode() = refPatch.getProjectionMode();
                patch.getU0() = ipdu.get2dPosX() + refPatch.getU0();
                patch.getV0() = ipdu.get2dPosY() + refPatch.getV0();
                patch.getPatchOrientation() = refPatch.getPatchOrientation();
                patch.getU1() = ipdu.get3dOffsetU() + refPatch.getU1();
                patch.getV1() = ipdu.get3dOffsetV() + refPatch.getV1();
                
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
                    patch.getD1() = (ipdu.get3dOffsetD() + (refPatch.getD1() / minLevel)) * minLevel;
                }
                else
                {
                    if (asps.getExtendedProjectionEnabledFlag() == false)
                    {
                        patch.getD1() = max3DCoordinate - (ipdu.get3dOffsetD() + ((max3DCoordinate - refPatch.getD1()) / minLevel)) * minLevel;
                    }
                    else
                    {
                        patch.getD1() = (max3DCoordinate) - (ipdu.get3dOffsetD() + ((max3DCoordinate - refPatch.getD1()) / minLevel)) * minLevel;
                    }
                }
                
                const int64_t deltaDD = ipdu.get3dRangeD();
                size_t previousDD = refPatch.getSizeD() / minLevel;
                
                if (previousDD * minLevel != refPatch.getSizeD())
                {
                    previousDD += 1;
                }
                
                patch.getSizeD() = (size_t((deltaDD + previousDD) * minLevel));
                
                patch.setLodScaleX(refPatch.getLodScaleX());
                patch.setLodScaleYIdc(refPatch.getLodScaleY());
                
                previousSizeU0 = patch.getSizeU0();
                previousSizeV0 = patch.getSizeV0();
                
                previousPatchSize2DXInPixel = patch.getPatchSize2DXInPixel();
                previousPatchSize2DYInPixel = patch.getPatchSize2DYInPixel();
                
                patch.allocOneLayerData();
            }
            else if (currentPatchType == RAW_PATCH)
            {
                RawPatchDataUnit& rpdu = pid.getRawPatchDataUnit();
                
                PCCRawPointsPatch& rawPointsPatch = pcmPatches[patchIndex - numNonRawPatch];
                rawPointsPatch.isPatchInAuxVideo_ = rpdu.getPatchInAuxiliaryVideoFlag();
                rawPointsPatch.u0_ = rpdu.get2dPosX();
                rawPointsPatch.v0_ = rpdu.get2dPosY();
                rawPointsPatch.sizeU0_ = rpdu.get2dSizeXMinus1() + 1;
                rawPointsPatch.sizeV0_ = rpdu.get2dSizeYMinus1() + 1;
                
                if (afps.getRaw3dOffsetBitCountExplicitModeFlag())
                {
                    rawPointsPatch.u1_ = rpdu.get3dOffsetU();
                    rawPointsPatch.v1_ = rpdu.get3dOffsetV();
                    rawPointsPatch.d1_ = rpdu.get3dOffsetD();
                }
                else
                {
                    const size_t pcmU1V1D1Level = size_t(1) << (gi.getGeometry2dBitdepthMinus1() + 1);
                    
                    rawPointsPatch.u1_ = rpdu.get3dOffsetU() * pcmU1V1D1Level;
                    rawPointsPatch.v1_ = rpdu.get3dOffsetV() * pcmU1V1D1Level;
                    rawPointsPatch.d1_ = rpdu.get3dOffsetD() * pcmU1V1D1Level;
                }
                
                rawPointsPatch.setNumberOfRawPoints(rpdu.getRawPointsMinus1() + 1);
                rawPointsPatch.occupancyResolution_ = size_t(1) << asps.getLog2PatchPackingBlockSize();
                
                totalNumberOfRawPoints += rawPointsPatch.getNumberOfRawPoints();
            }
            else if (currentPatchType == EOM_PATCH)
            {
                EOMPatchDataUnit& epdu = pid.getEomPatchDataUnit();
                
                PCCEomPatch eomPatch;
                eomPatch.isPatchInAuxVideo_ = epdu.getPatchInAuxiliaryVideoFlag();
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
                
                eomPatch.occupancyResolution_ = size_t(1) << asps.getLog2PatchPackingBlockSize();
                eomPatches.push_back(eomPatch);
                
                totalNumberOfEomPoints += eomPatch.eomCount_;
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

        tile.setTotalNumberOfRawPoints(totalNumberOfRawPoints);
        tile.setTotalNumberOfEOMPoints(totalNumberOfEomPoints);
    }
}

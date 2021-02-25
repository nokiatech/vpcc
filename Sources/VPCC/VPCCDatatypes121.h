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

#include <string>
#include <vector>

#include "Helpers.h"
#include "HEVC.h"

namespace VPCC121
{
    // Data structures for rendering
    struct PatchOrientation
    {
        enum Enum
        {
            DEFAULT = 0,
            SWAP = 1,
            ROT90 = 2,
            ROT180 = 3,
            ROT270 = 4,
            MIRROR = 5,
            MROT90 = 6,
            MROT180 = 7,
            MROT270 = 8,

            COUNT
        };
    };

    struct Patch
    {
        uint32_t u1 = 0;  // tangential shift
        uint32_t v1 = 0;  // bitangential shift

        int32_t d1 = 0;  // depth shift
        
        uint32_t u0 = 0;  // location in packed image
        uint32_t v0 = 0;  // location in packed image

        uint32_t sizeU0 = 0;  // size of occupancy map
        uint32_t sizeV0 = 0;  // size of occupancy map

        uint32_t normalAxis = 0;  // x
        uint32_t tangentAxis = 0;  // y
        uint32_t bitangentAxis = 0;  // z

        uint32_t patchOrientation = 0;  // patch orientation in canvas atlas
        uint32_t projectionMode = 0;  // 0: related to the min depth value; 1: related to the max value
        uint32_t occupancyResolution = 0;  // occupancy map resolution
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
        std::vector<size_t> blockToPatch;
        
        size_t afOrderCnt = 0;
        size_t index = 0;
        
        size_t width = 0;
        size_t height = 0;
    };

    struct VideoType
    {
        enum Enum
        {
            INVALID = -1,

            OCCUPANCY,
            GEOMETRY,
            TEXTURE,

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
                    
                case TEXTURE:
                    return "TEXTURE";

                default:
                    return "INVALID";
            }
        }
    };

    struct VideoStream
    {
        VideoType::Enum type = VideoType::INVALID;

        // Raw HEVC data streams
        std::vector<uint8_t> buffer;
        
        // HEVC video packets for decoder input queue
        std::vector<VideoFramePacket> packets;

        HEVC::DecoderParameters decoderParameters;

        HEVC::VPS vps;
        HEVC::SPS sps;
    };

    struct FrameGroup
    {
        std::vector<FrameData> frames;
        VideoStream videoStream[VideoType::COUNT];
    };
}

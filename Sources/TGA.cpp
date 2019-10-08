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

#include "TGA.h"

#include <cstdint>

#if PLATFORM_IOS || PLATFORM_MACOS || PLATFORM_ANDROID

#include <unistd.h>
#include <fcntl.h>

#endif

#include "FileSystem.h"

namespace TGA
{
    #pragma pack(push, 1)

    struct TGAHeader
    {
        uint8_t identsize;
        uint8_t colorMapType;
        uint8_t imageType;
        uint16_t colorMapStart;
        uint16_t colorMapLength;
        uint8_t colorMapBits;
        uint16_t x;
        uint16_t y;
        uint16_t width;
        uint16_t height;
        uint8_t bitsPerPixel;
        uint8_t descriptor;
    };

    #pragma pack(pop)

    bool saveToDisk(const std::string &filename, const uint8_t* data, size_t bytes, uint16_t width, uint16_t height, uint8_t bytesPerPixel)
    {
        TGAHeader header;
        header.identsize = 0;
        header.colorMapType = 0;
        header.imageType = 2;
        header.colorMapStart = 0;
        header.colorMapLength = 0;
        header.colorMapBits = 0;
        header.x = 0;
        header.y = 0;
        header.width = width;
        header.height = height;
        header.bitsPerPixel = bytesPerPixel * 8;
        header.descriptor = 0;

        std::string outputPath = FileSystem::config().internalStoragePath;
        outputPath.append(filename);

        FILE* handle = fopen(outputPath.c_str(), "wb");

        if (handle)
        {
            fwrite(&header, sizeof(header), 1, handle);
            fwrite(data, bytes, 1, handle);
            fflush(handle);
            fclose(handle);

            return true;
        }

        return false;
    }

    bool loadFromDisk(const std::string& filename, uint8_t** data, size_t& bytes, uint16_t& width, uint16_t& height, uint8_t& bytesPerPixel)
    {
        std::string inputPath = FileSystem::config().internalStoragePath;
        inputPath.append(filename);

        FILE* handle = fopen(inputPath.c_str(), "rb");

        if (handle)
        {
            TGAHeader header;
            fread(&header, sizeof(header), 1, handle);

            width = header.width;
            height = header.height;
            bytesPerPixel = (header.bitsPerPixel / 8);
            bytes = width * height * bytesPerPixel;

            uint8_t* buffer = (uint8_t*)malloc(bytes);
            fread(buffer, bytes, 1, handle);

            *(data) = buffer;

            fclose(handle);

            return true;
        }

        return false;
    }
}

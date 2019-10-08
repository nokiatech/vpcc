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

#include <cstdint>
#include <string>

struct IOBuffer
{
    uint8_t* data = NULL;
    uint64_t size = 0;

    static IOBuffer alloc(size_t bytes)
    {
        IOBuffer buffer;
        buffer.data = new uint8_t[bytes];
        buffer.size = bytes;

        return buffer;
    }

    static void free(IOBuffer* buffer)
    {
        delete[] buffer->data;
        buffer->data = NULL;

        buffer->size = 0;
    }
};

namespace FileSystem
{
    struct Config
    {
        void* assetManager = NULL;
        const char* internalStoragePath = "";
    };

    bool initialize(Config config);
    bool shutdown();

    Config config();

    bool saveToDisk(const std::string& filename, const uint8_t* data, size_t bytes);

    IOBuffer loadFromDisk(const std::string& filename);
    IOBuffer loadFromBundle(const std::string& filename);
}

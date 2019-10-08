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

#include "FileSystem.h"

#include <cstdint>

#include <unistd.h>
#include <fcntl.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace FileSystem
{
    static Config _config = Config();

    bool initialize(Config config)
    {
        _config = config;

        return true;
    }

    bool shutdown()
    {
        _config = Config();

        return true;
    }

    Config config()
    {
        return _config;
    }

    bool saveToDisk(const std::string& filename, const uint8_t* data, size_t bytes)
    {
        std::string outputPath = FileSystem::config().internalStoragePath;
        outputPath.append("/");
        outputPath.append(filename);

        FILE* handle = fopen(outputPath.c_str(), "wb");

        if (handle)
        {
            fwrite(data, bytes, 1, handle);
            fflush(handle);
            fclose(handle);

            return true;
        }

        return false;
    }

    IOBuffer loadFromDisk(const std::string& filename)
    {
        IOBuffer buffer;
        std::string outputPath = filename;

        FILE *handle = fopen(filename.c_str(), "rb");

        if (handle)
        {
            // Get file length
            fseek(handle, 0, SEEK_END);
            size_t length = ftell(handle);
            fseek(handle, 0, SEEK_SET);

            buffer = IOBuffer::alloc(length);

            fread(buffer.data, length, 1, handle);

            fclose(handle);
        }

        return buffer;
    }

    IOBuffer loadFromBundle(const std::string& filename)
    {
        std::string tmp = "Assets/";
        size_t l0 = tmp.size();
        size_t l1 = filename.size() - tmp.size();

        std::string hack_filename = filename.substr(l0, l1);

        AAsset* asset = AAssetManager_open((AAssetManager*)_config.assetManager, hack_filename.c_str(), AASSET_MODE_BUFFER);
        off64_t length = AAsset_getLength64(asset);

        IOBuffer buffer;
        buffer.data = new uint8_t[length];
        buffer.size = length;

        int bytesRead = AAsset_read(asset, buffer.data, length);

        if (length != bytesRead)
        {
            delete[] buffer.data;
            buffer.data = NULL;
        }

        AAsset_close(asset);

        return buffer;
    }
}

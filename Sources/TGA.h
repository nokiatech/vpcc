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
#include <cstddef>

#include <string>

namespace TGA
{
    bool saveToDisk(const std::string &filename, const uint8_t* data, size_t bytes, uint16_t width, uint16_t height, uint8_t bytesPerPixel);
    bool loadFromDisk(const std::string& filename, uint8_t** data, size_t& bytes, uint16_t& width, uint16_t& height, uint8_t& bytesPerPixel);
}

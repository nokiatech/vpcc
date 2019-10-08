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
#include <cassert>

#include <string>

#include "GraphicsAPI.h"

struct Texture2D
{
    struct Type
    {
        enum Enum
        {
            INVALID = 0,
            TEXTURE_RGB = 1,
            TEXTURE_RGBA = 2,
            VIDEO_TEXTURE = 5,
        };
    };

    GLuint handle = 0;

    GLuint width = 0;
    GLuint height = 0;

    uint8_t numChannels = 0;

    Type::Enum type = Type::TEXTURE_RGB;
};

bool loadTexture(std::string filename, Texture2D& texture);
bool freeTexture(Texture2D& texture);

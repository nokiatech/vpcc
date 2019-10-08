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

#include "Texture2D.h"

#include "Logger.h"
#include "FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool loadTexture(std::string filename, Texture2D& texture)
{
    int32_t width = 0;
    int32_t height = 0;
    int32_t numChannels = 0;

    IOBuffer buffer = FileSystem::loadFromBundle(filename);

    if (buffer.data)
    {
        stbi_set_flip_vertically_on_load(false);
        stbi_uc* data = stbi_load_from_memory(buffer.data, buffer.size, &width, &height, &numChannels, 0);

        pushDebugMarker("loadTexture");

        glGenTextures(1, &texture.handle);
        glBindTexture(GL_TEXTURE_2D, texture.handle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLint internalformat = 0;
        GLenum format = 0;

        if (numChannels == 3)
        {
            internalformat = GL_RGB;
            format = GL_RGB;
        }
        else if (numChannels == 4)
        {
            internalformat = GL_RGBA;
            format = GL_RGBA;
        }
        else
        {
            assert(false);
        }

        glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glBindTexture(GL_TEXTURE_2D, 0);

        popDebugMarker();

        GL_CHECK_ERRORS();

        stbi_image_free(data);

        texture.width = width;
        texture.height = height;
        texture.numChannels = numChannels;
        texture.type = Texture2D::Type::TEXTURE_RGB;

        IOBuffer::free(&buffer);
    }
    else
    {
        LOG_E("TextureLoader::Could not load TGA texture: %s", filename.c_str());

        return false;
    }

    return true;
}

bool freeTexture(Texture2D& texture)
{
    glDeleteTextures(1, &texture.handle);

    texture.handle = 0;
    texture.width = 0;
    texture.height = 0;
    texture.numChannels = 0;
    texture.type = Texture2D::Type::INVALID;

    return true;
}

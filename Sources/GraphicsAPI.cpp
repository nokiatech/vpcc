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

#include "GraphicsAPI.h"

#if PLATFORM_IOS || PLATFORM_MACOS || PLATFORM_ANDROID

#include <unistd.h>

#endif

#include <sstream>
#include <string>

#include "FileSystem.h"

// OpenGL function pointers.
#define GL_DECLARE(a, b) a b;
#include "GLDeclarations.inc"
#undef GL_DECLARE

void* getProcAddress(const char* name)
{
#if PLATFORM_ANDROID

	return (void*)::eglGetProcAddress(name);

#elif PLATFORM_WINDOWS

	return (void*)::wglGetProcAddress(name);

#else

    return NULL;

#endif
}

const char* errorStringGL(GLenum error)
{
    switch(error)
    {
        case GL_NO_ERROR:               return "GL_NO_ERROR";
        case GL_INVALID_ENUM:           return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:          return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:      return "GL_INVALID_OPERATION";

#if defined __gl_h_

        case GL_STACK_OVERFLOW:         return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW:        return "GL_STACK_UNDERFLOW";

        case GL_CONTEXT_LOST:           return "GL_CONTEXT_LOST";
        case GL_TABLE_TOO_LARGE:        return "GL_TABLE_TOO_LARGE";

#endif

#if defined __gl_h_ || defined __gl3_h_

        case GL_OUT_OF_MEMORY:                      return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:      return "GL_INVALID_FRAMEBUFFER_OPERATION";

#endif
    }

    return "GL_UNKNOWN_ERROR";
}

GLuint createShader(GLenum shaderType, const char* shaderSource)
{
    GLuint shader = glCreateShader(shaderType);

    if (!shader)
    {
        return shader;
    }

    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        char buffer[1024] = { 0 };
        glGetShaderInfoLog(shader, 1024, NULL, buffer);

        LOG_E("Could not compile shader %d:\n%s", shaderType, buffer);

        glDeleteShader(shader);
        shader = 0;
    }

    GL_CHECK_ERRORS();

    return shader;
}

std::string loadShader(const char* filename, GLint type, bool manualVideoTextureUpload)
{
    // Common functions
    std::string shaderLibraryFilename = "Assets/Shaders/common.glsl";
    std::string shaderLibrary;

    {
        IOBuffer buffer = FileSystem::loadFromBundle(shaderLibraryFilename);

        if (buffer.data == NULL)
        {
            LOG_E("Failed to load file: %s", shaderLibraryFilename.c_str());

            return 0;
        }

        shaderLibrary.append((char*)buffer.data, buffer.size);

        IOBuffer::free(&buffer);
    }

    // Shader
    std::string shaderContent;

    {
        IOBuffer buffer = FileSystem::loadFromBundle(filename);

        if (buffer.data == NULL)
        {
            LOG_E("Failed to load file: %s", filename);

            return 0;
        }

        shaderContent.append((char*)buffer.data, buffer.size);

        IOBuffer::free(&buffer);
    }

    // Pre-process shader
    std::string version;

#if PLATFORM_ANDROID || PLATFORM_IOS

    #if PLATFORM_ANDROID

        int32_t apiVersion = 310;

    #elif PLATFORM_IOS

        int32_t apiVersion = 300;

    #endif

    if (apiVersion == 310)
    {
        version.append("#version 310 es\n");
    }
    else if (apiVersion)
    {
        version.append("#version 300 es\n");
    }
    else
    {
        version.append("#version 100\n");
    }

#elif PLATFORM_MACOS || PLATFORM_WINDOWS

    version.append("#version 410\n");

#else

    #error Unsupported platform

#endif

    std::string extensions;

#if PLATFORM_ANDROID

    if (apiVersion >= 300)
    {
        extensions.append("#extension GL_OES_EGL_image_external_essl3 : require\n");
        extensions.append("#extension GL_EXT_YUV_target : require\n");

        extensions.append("#extension GL_ANDROID_extension_pack_es31a : require\n");
        //extensions.append("#extension GL_EXT_texture_buffer : require\n");
    }
    else
    {
        extensions.append("#extension GL_OES_EGL_image_external : require\n");
    }

#endif

    std::string defines;

    if (type == GL_VERTEX_SHADER)
    {
        defines.append("#define VERTEX_SHADER\n");
    }
    else if (type == GL_FRAGMENT_SHADER)
    {
        defines.append("#define FRAGMENT_SHADER\n");
    }

#if PLATFORM_ANDROID

    defines.append("#define PLATFORM_ANDROID\n");

#elif PLATFORM_IOS

    defines.append("#define PLATFORM_IOS\n");

#elif PLATFORM_MACOS

    defines.append("#define PLATFORM_MACOS\n");

#elif PLATFORM_WINDOWS

    defines.append("#define PLATFORM_WINDOWS\n");

#endif

    if (manualVideoTextureUpload)
    {
        defines.append("#define ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD\n");
    }
    else
    {
#if PLATFORM_ANDROID

    defines.append("#define ENABLE_MEDIA_CODEC\n");

#elif PLATFORM_IOS || PLATFORM_MACOS

    defines.append("#define ENABLE_VIDEO_TOOLBOX\n");

#elif PLATFORM_WINDOWS

    defines.append("#define ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD\n");

#endif
    }

#if 0

    const char* precision = R"(
    #ifdef GL_ES
        #ifdef GL_FRAGMENT_PRECISION_HIGH
            precision highp float;
        #else
            precision mediump float;
        #endif
    #else
        #define lowp
        #define mediump
        #define highp
    #endif
    )";

#endif

    std::string result;
    result.append(version);
    result.append(extensions);
    //result.append(precision);
    result.append(defines);
    result.append(shaderLibrary);
    result.append(shaderContent);

    return result;
}

GLuint createProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename, bool manualVideoTextureUpload, const char* varyings[], uint8_t numVaryings)
{
    std::string preprocessedVS = loadShader(vertexShaderFilename, GL_VERTEX_SHADER, manualVideoTextureUpload);
    std::string preprocessedFS = loadShader(fragmentShaderFilename, GL_FRAGMENT_SHADER, manualVideoTextureUpload);

    GLuint vertexShader = createShader(GL_VERTEX_SHADER, preprocessedVS.c_str());

    if (!vertexShader)
    {
        return 0;
    }

    GLuint fragment_shader = createShader(GL_FRAGMENT_SHADER, preprocessedFS.c_str());

    if (!fragment_shader)
    {
        return 0;
    }

    GLuint program = glCreateProgram();

    if (program)
    {
        glAttachShader(program, vertexShader);
        GL_CHECK_ERRORS();

        glAttachShader(program, fragment_shader);
        GL_CHECK_ERRORS();

        if (varyings != NULL && numVaryings > 0)
        {
            glTransformFeedbackVaryings(program, numVaryings, varyings, GL_SEPARATE_ATTRIBS);
            GL_CHECK_ERRORS();
        }

        glLinkProgram(program);

        GLint link_status = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);

        if (link_status != GL_TRUE)
        {
            GLint buf_length = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &buf_length);

            if (buf_length)
            {
                char* buf = reinterpret_cast<char*>(malloc(buf_length));

                if (buf)
                {
                    glGetProgramInfoLog(program, buf_length, NULL, buf);
                    LOG_E("Could not link program:\n%s", buf);

                    free(buf);
                }
            }

            glDeleteProgram(program);
            program = 0;
        }
    }

    GL_CHECK_ERRORS();

    return program;
}

void pushDebugMarker(const char* name)
{
#if PLATFORM_WINDOWS

	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION_KHR, 1, -1, name);

#elif PLATFORM_ANDROID

    // TODO

#elif GL_KHR_debug

	glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION_KHR, 1, -1, name);

#elif GL_EXT_debug_marker

	glPushGroupMarkerEXT(0, name);

#endif
}

void popDebugMarker()
{
#if PLATFORM_WINDOWS

	glPopDebugGroup();

#elif PLATFORM_ANDROID

    // TODO

#elif GL_KHR_debug

	glPopDebugGroupKHR();

#elif GL_EXT_debug_marker

	glPopGroupMarkerEXT();

#endif
}

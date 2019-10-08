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

GLuint createShader(GLenum shader_type, const char* shader_source)
{
    GLuint shader = glCreateShader(shader_type);

    if (!shader)
    {
        return shader;
    }

    glShaderSource(shader, 1, &shader_source, NULL);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        char buffer[1024] = { 0 };
        glGetShaderInfoLog(shader, 1024, NULL, buffer);

        LOG_E("Could not compile shader %d:\n%s", shader_type, buffer);

        glDeleteShader(shader);
        shader = 0;
    }

    GL_CHECK_ERRORS();

    return shader;
}

GLuint createProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename, bool manualVideoTextureUpload, const char* varyings[], uint8_t num_varyings)
{
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

    std::string vertexShaderContent;

    {
        IOBuffer buffer = FileSystem::loadFromBundle(vertexShaderFilename);

        if (buffer.data == NULL)
        {
            LOG_E("Failed to load file: %s", vertexShaderFilename);

            return 0;
        }

        vertexShaderContent.append((char*)buffer.data, buffer.size);

        IOBuffer::free(&buffer);
    }

    std::string fragmentShaderContent;

    {
        IOBuffer buffer = FileSystem::loadFromBundle(fragmentShaderFilename);

        if (buffer.data == NULL)
        {
            LOG_E("Failed to load file: %s", fragmentShaderFilename);

            return 0;
        }

        fragmentShaderContent.append((char*)buffer.data, buffer.size);

        IOBuffer::free(&buffer);
    }

    std::string version;

#if PLATFORM_ANDROID || PLATFORM_IOS

    bool oes3 = true;

    if (oes3)
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

    if (oes3)
    {
        extensions.append("#extension GL_OES_EGL_image_external_essl3 : require\n");
        extensions.append("#extension GL_EXT_YUV_target : require\n");
    }
    else
    {
        extensions.append("#extension GL_OES_EGL_image_external : require\n");
    }

#endif

    std::string definesVS;
    definesVS.append("#define VERTEX_SHADER\n");

    std::string definesFS;
    definesFS.append("#define FRAGMENT_SHADER\n");

#if PLATFORM_ANDROID

    definesVS.append("#define PLATFORM_ANDROID\n");
    definesFS.append("#define PLATFORM_ANDROID\n");

#elif PLATFORM_IOS

    definesVS.append("#define PLATFORM_IOS\n");
    definesFS.append("#define PLATFORM_IOS\n");

#elif PLATFORM_MACOS

    definesVS.append("#define PLATFORM_MACOS\n");
    definesFS.append("#define PLATFORM_MACOS\n");

#elif PLATFORM_WINDOWS

	definesVS.append("#define PLATFORM_WINDOWS\n");
	definesFS.append("#define PLATFORM_WINDOWS\n");

#endif

    if (manualVideoTextureUpload)
    {
        definesVS.append("#define ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD\n");
        definesFS.append("#define ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD\n");
    }

#if PLATFORM_ANDROID

    definesVS.append("#define ENABLE_MEDIA_CODEC\n");
    definesFS.append("#define ENABLE_MEDIA_CODEC\n");

#elif PLATFORM_IOS || PLATFORM_MACOS

    definesVS.append("#define ENABLE_VIDEO_TOOLBOX\n");
    definesFS.append("#define ENABLE_VIDEO_TOOLBOX\n");

#elif PLATFORM_WINDOWS

	definesVS.append("#define ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD\n");
	definesFS.append("#define ENABLE_MANUAL_VIDEO_TEXTURE_UPLOAD\n");

#endif

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

    std::string preprocessedVS;
    preprocessedVS.append(version);
    preprocessedVS.append(extensions);
    //preprocessedVS.append(precision);
    preprocessedVS.append(definesVS);
    preprocessedVS.append(shaderLibrary);
    preprocessedVS.append(vertexShaderContent);

    std::string preprocessedFS;
    preprocessedFS.append(version);
    preprocessedFS.append(extensions);
    //preprocessedFS.append(precision);
    preprocessedFS.append(definesFS);
    preprocessedFS.append(shaderLibrary);
    preprocessedFS.append(fragmentShaderContent);

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

        if (varyings != NULL && num_varyings > 0)
        {
            glTransformFeedbackVaryings(program, num_varyings, varyings, GL_SEPARATE_ATTRIBS);
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

    return program;
}

void pushDebugMarker(const char* name)
{
#if PLATFORM_ANDROID

    return;

#endif

#ifndef PLATFORM_WINDOWS

#if GL_KHR_debug

    glPushDebugGroupKHR(GL_DEBUG_SOURCE_APPLICATION_KHR, 1, -1, name);

#elif GL_EXT_debug_marker

    glPushGroupMarkerEXT(0, name);

#endif

#endif
}

void popDebugMarker()
{
#if PLATFORM_ANDROID

    return;

#endif

#ifndef PLATFORM_WINDOWS

#if GL_KHR_debug

    glPopDebugGroupKHR();

#elif GL_EXT_debug_marker

    glPopGroupMarkerEXT();

#endif

#endif
}

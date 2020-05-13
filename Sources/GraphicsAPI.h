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

#include "Logger.h"

// OpenGL ES 3.0 / 3.1 / 3.2
#if PLATFORM_ANDROID

    #include <EGL/egl.h>
    #include <EGL/eglext.h>

    // #include <GLES3/gl3.h>
    // #include <GLES3/gl31.h>
    #include <GLES3/gl32.h>

    #include <GLES2/gl2ext.h>

#elif PLATFORM_IOS

    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>

#elif PLATFORM_MACOS

    #include <OpenGL/OpenGL.h>

    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>

#elif PLATFORM_WINDOWS

	#define NOMINMAX 1
	#include <windows.h>

	#include <gl/gl.h>

	#include "Khronos/gl/glext.h"
	#include "Khronos/wgl/wglext.h"

#else

	#error Unsupported platform

#endif

#include <assert.h>

#include <string>

// OpenGL function pointers.
#define GL_DECLARE(a, b) extern a b;
#include "GLDeclarations.inc"
#undef GL_DECLARE

void* getProcAddress(const char* name);

struct GLRegistryLoader
{
	static void initialize()
    {
		// OpenGL / WGL function pointers.
		#define GL_DECLARE(a, b) { b = (a)getProcAddress(#b); }
		#include "GLDeclarations.inc"
		#undef GL_DECLARE
    }
};

struct ShaderType
{
    enum Enum
    {
        VERTEX_SHADER = 0,
        FRAGMENT_SHADER = 1,
    };
};

const char* errorStringGL(GLenum error);

std::string loadShader(const char* filename, GLint type, bool manualVideoTextureUpload = false);
GLuint createProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename, bool manualVideoTextureUpload = false, const char* varyings[] = NULL, uint8_t num_varyings = 0);

void pushDebugMarker(const char* name);
void popDebugMarker();

#define GL_BUFFER_OFFSET(i) ((uint8_t*)NULL + (i))

#define GL_ERROR_CHECKS_ENABLED 0

#if GL_ERROR_CHECKS_ENABLED
        
    #define GL_CHECK_ERRORS()                               \
    {                                                       \
        do                                                  \
        {                                                   \
            GLenum error = glGetError();                    \
            const char* str = errorStringGL(error);         \
														    \
            if (error != GL_NO_ERROR)                       \
            {                                               \
                LOG_E("OpenGL: 0x%x, %s", error, str);      \
                assert(false);    		                    \
            }                                               \
        }                                                   \
        while(false);                                       \
    }
            
    #define GL_CHECK(call)                                  \
    {                                                       \
        do                                                  \
        {                                                   \
            call;                                           \
                                                            \
            GLenum error = glGetError();                    \
            const char* str = errorStringGL(error);         \
                                                            \
            if (error != GL_NO_ERROR)                       \
            {                                               \
                LOG_E("OpenGL: 0x%x, %s", error, str);      \
                assert(false);                              \
            }                                               \
        }                                                   \
        while(false);                                       \
    }
        
#else
        
    #define GL_CHECK_ERRORS()
    #define GL_CHECK(call) call
        
#endif

// Compability
#ifndef GL_OES_EGL_image_external
#define GL_OES_EGL_image_external 1
#define GL_TEXTURE_EXTERNAL_OES           0x8D65
#define GL_TEXTURE_BINDING_EXTERNAL_OES   0x8D67
#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES 0x8D68
#define GL_SAMPLER_EXTERNAL_OES           0x8D66
#endif /* GL_OES_EGL_image_external */

#ifndef GL_OES_EGL_image_external_essl3
#define GL_OES_EGL_image_external_essl3 1
#endif /* GL_OES_EGL_image_external_essl3 */

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

#include "Logger.h"

#include <cstdio>

#include <unistd.h>
#include <fcntl.h>

#include <jni.h>
#include <android/log.h>

#define MAX_LOG_LINE_LENGTH 1024

namespace LogDispatcher
{
    #define ANDROID_LOG_TAG "ARPlayer"

    void logVerbose(const char* format, va_list args)
    {
        va_list copy;
        va_copy(copy, args);

        __android_log_vprint(ANDROID_LOG_VERBOSE, ANDROID_LOG_TAG, format, copy);

        va_end(copy);
    }

    void logDebug(const char* format, va_list args)
    {
        va_list copy;
        va_copy(copy, args);

        __android_log_vprint(ANDROID_LOG_DEBUG, ANDROID_LOG_TAG, format, copy);

        va_end(copy);
    }

    void logInfo(const char* format, va_list args)
    {
        va_list copy;
        va_copy(copy, args);

        __android_log_vprint(ANDROID_LOG_INFO, ANDROID_LOG_TAG, format, copy);

        va_end(copy);
    }

    void logWarning(const char* format, va_list args)
    {
        va_list copy;
        va_copy(copy, args);

        __android_log_vprint(ANDROID_LOG_WARN, ANDROID_LOG_TAG, format, copy);

        va_end(copy);
    }

    void logError(const char* format, va_list args)
    {
        va_list copy;
        va_copy(copy, args);

        __android_log_vprint(ANDROID_LOG_ERROR, ANDROID_LOG_TAG, format, copy);

        va_end(copy);
    }

    void logVerbose(const char* format, ...)
    {
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_LEVEL_VERBOSE

        va_list args;
        va_start(args, format);

        logVerbose(format, args);

        va_end(args);

#endif
    }

    void logDebug(const char* format, ...)
    {
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_LEVEL_DEBUG

        va_list args;
        va_start(args, format);

        logDebug(format, args);

        va_end(args);

#endif
    }

    void logInfo(const char* format, ...)
    {
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_LEVEL_INFO

        va_list args;
        va_start(args, format);

        logInfo(format, args);

        va_end(args);

#endif
    }

    void logWarning(const char* format, ...)
    {
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_LEVEL_WARNING

        va_list args;
        va_start(args, format);

        logWarning(format, args);

        va_end(args);

#endif
    }

    void logError(const char* format, ...)
    {
#if DEBUG_LOG_LEVEL >= DEBUG_LOG_LEVEL_ERROR

        va_list args;
        va_start(args, format);

        logError(format, args);

        va_end(args);

#endif
    }
}

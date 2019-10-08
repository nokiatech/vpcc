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
#include <cstdarg>

#define DEBUG_LOG_LEVEL_VERBOSE     5
#define DEBUG_LOG_LEVEL_DEBUG       4
#define DEBUG_LOG_LEVEL_INFO        3
#define DEBUG_LOG_LEVEL_WARNING     2
#define DEBUG_LOG_LEVEL_ERROR       1

// Set wanted log level
#ifndef DEBUG_LOG_LEVEL

    #define DEBUG_LOG_LEVEL DEBUG_LOG_LEVEL_INFO

#endif

#define LOG_V(...) LogDispatcher::logVerbose(__VA_ARGS__)
#define LOG_D(...) LogDispatcher::logDebug(__VA_ARGS__)
#define LOG_I(...) LogDispatcher::logInfo(__VA_ARGS__)
#define LOG_W(...) LogDispatcher::logWarning(__VA_ARGS__)
#define LOG_E(...) LogDispatcher::logError(__VA_ARGS__)

namespace LogDispatcher
{
    void logVerbose(const char* format, ...);
    void logDebug(const char* format, ...);
    void logInfo(const char* format, ...);
    void logWarning(const char* format, ...);
    void logError(const char* format, ...);

    void logVerboseV(const char* format, va_list args);
    void logDebugV(const char* format, va_list args);
    void logInfoV(const char* format, va_list args);
    void logWarningV(const char* format, va_list args);
    void logErrorV(const char* format, va_list args);
}

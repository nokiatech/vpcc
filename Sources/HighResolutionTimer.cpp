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

#include "HighResolutionTimer.h"

#include <time.h>

namespace HighResolutionTimer
{
#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_MACOS || PLATFORM_WINDOWS

    static int64_t getNs(const timespec &ts)
    {
        return (ts.tv_sec * 1000000000ll + ts.tv_nsec);
    }

#endif

#if PLATFORM_WINDOWS

	int clock_gettime(int, struct timespec* tv)
	{
		return timespec_get(tv, TIME_UTC);
	}

#endif

    int64_t getTimeUs()
    {
#if PLATFORM_ANDROID || PLATFORM_IOS || PLATFORM_MACOS

        timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);

        static int64_t initial = getNs(ts);

        return (getNs(ts) - initial) / 1000ll;

#else

		timespec ts;
		clock_gettime(0, &ts);

		static int64_t initial = getNs(ts);

		return (getNs(ts) - initial) / 1000ll;

#endif
    }

    int64_t getTimeMs()
    {
        return (int64_t) (getTimeUs() / 1000ll);
    }
}

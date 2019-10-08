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

#undef min
#undef max
#undef abs
#undef clamp

namespace Math
{
	template<typename T>
	const T& min(const T& a, const T& b)
	{
		return (a < b) ? a : b;
	}

	template<typename T>
	const T& max(const T& a, const T& b)
	{
		return (a > b) ? a : b;
	}

	template<typename T>
	T abs(const T& a)
	{
		return (a >= 0) ? a : -a;
	}

	template<typename T>
	const T& clamp(const T& value, const T& min, const T& max)
	{
		if(value < min)
		{
			return min;
		}
		else if(value > max)
		{
			return max;
		}
		else
		{
			return value;
		}
	}
}

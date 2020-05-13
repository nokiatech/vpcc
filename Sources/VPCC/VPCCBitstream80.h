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

#include <sys/types.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include "Logger.h"
#include <string>

namespace VPCC
{
    struct Bitstream
    {
        size_t position;
        uint8_t bitmask;

        uint8_t* data;
        size_t length;

        Bitstream();
        Bitstream(const Bitstream& bs);
        Bitstream(uint8_t* data, size_t length);

        ~Bitstream();

        Bitstream& operator = (const Bitstream& bs);
    };

    namespace BitstreamReader
    {
        bool isAligned(Bitstream& bs);
        void align(Bitstream& bs);

        void shiftBitmask(Bitstream& bs);
        void seek(Bitstream& bs, ssize_t count);

        void skipBytes(Bitstream& bs, size_t count);
        void skipBits(Bitstream& bs, size_t count);

        uint32_t readBits(Bitstream& bs, size_t count = 1);
        int32_t readBitsS(Bitstream& bs, size_t count);
        size_t readBytes(Bitstream& bs, uint8_t* buffer, size_t size);
		std::string readString(Bitstream& bs);

        int8_t readInt8(Bitstream& bs);
        uint8_t readUInt(Bitstream& bs);

        int16_t readInt16(Bitstream& bs);
        uint16_t readUInt16(Bitstream& bs);

        int32_t readInt32(Bitstream& bs);
        uint32_t readUInt32(Bitstream& bs);

        int64_t readInt64(Bitstream& bs);
        uint64_t readUInt64(Bitstream& bs);

        template <typename T>
        bool readValue(Bitstream& bs, T* result)
        {
            align(bs);

            size_t valueSize = sizeof(T);

            if (bs.position + valueSize < bs.length)
            {
                // LOG_V("PCCBitstream::read = bits: %d, position: %lu", valueSize * 8, bs.position);
                LOG_V("PCCBitstream::read = bits: %d", valueSize * 8);

                T* ptr = (T*)(bs.data + bs.position);
                *result = ptr[0];

                bs.position += valueSize;

                return true;
            }

            return false;
        }

        size_t bytesAvailable(Bitstream& bs);

        uint32_t readUVLC(Bitstream& bs);
        int32_t readSVLC(Bitstream& bs);
    }
}

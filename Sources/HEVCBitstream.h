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

namespace HEVC
{
    class Bitstream
    {
    public:

        Bitstream();
        Bitstream(const Bitstream& bitstream);
        Bitstream(uint8_t* data, size_t length);

        ~Bitstream();

        Bitstream& operator = (const Bitstream& bitstream);

        void setData(uint8_t* data, size_t length);

        void setPosition(size_t position);
        size_t position() const;

        void setBitmask(uint8_t bitmask);
        uint8_t bitmask() const;

        uint8_t* data();

        size_t length() const;

        void align();
        void resetPosition();

        void shiftBitmask();

        uint32_t readBits(size_t count = 1);

        void seek(ssize_t count);

        void skipBytes(size_t count);
        void skipBits(size_t count);

        int8_t readInt8();
        uint8_t readUInt();

        int16_t readInt16();
        uint16_t readUInt16();

        int32_t readInt32();
        uint32_t readUInt32();

        int64_t readInt64();
        uint64_t readUInt64();

        float readFloat();
        double readDouble();

        template <typename T>
        bool readValue(T* result)
        {
            align();

            size_t valueSize = sizeof(T);

            if (_position + valueSize < _length)
            {
                T* ptr = (T*)(_data + _position);
                *result = ptr[0];

                _position += valueSize;

                return true;
            }

            return false;
        }

        size_t readBytes(uint8_t* buffer, size_t size);
        size_t bytesAvailable() const;

        uint32_t readUGolomb();
        int32_t readSGolomb();

        static size_t bitsNeeded(size_t value)
        {
            size_t bitsNeeded = 1;

            while (value > (size_t)(1 << bitsNeeded))
            {
                ++bitsNeeded;
            }

            return bitsNeeded;
        }

    private:

        void handleEPB();

    private:

        static const uint8_t INITIAL_BITMASK = 0x80;

        size_t _position;
        uint8_t _bitmask;

        uint8_t* _data;
        size_t _length;
    };
}

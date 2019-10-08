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

#include "PCCBitstream60.h"

#include "Helpers.h"

#include <assert.h>
#include <cstring>

namespace PCC
{
    static const uint8_t INITIAL_BITMASK = 0x80;

    Bitstream::Bitstream()
    : position(0)
    , bitmask(INITIAL_BITMASK)
    , data(NULL)
    , length(0)
    {
    }

    Bitstream::Bitstream(const Bitstream& bs)
    : position(bs.position)
    , bitmask(bs.bitmask)
    , data(bs.data)
    , length(bs.length)
    {
    }

    Bitstream::Bitstream(uint8_t* data, size_t length)
    : position(0)
    , bitmask(INITIAL_BITMASK)
    , data(data)
    , length(length)
    {
    }

    Bitstream::~Bitstream()
    {
    }

    Bitstream& Bitstream::operator = (const Bitstream& bs)
    {
        position = bs.position;
        bitmask = bs.bitmask;
        data = bs.data;
        length = bs.length;

        return *this;
    }

    namespace BitstreamReader
    {
        uint16_t swapUInt16(uint16_t value)
        {
            return ((value & 0x00FF) << 8) |
                   ((value & 0xFF00) >> 8);
        }

        uint32_t swapUInt32(uint32_t value)
        {
            return ((value & 0x000000FF) << 24) |
                   ((value & 0x0000FF00) << 8) |
                   ((value & 0x00FF0000) >> 8) |
                   ((value & 0xFF000000) >> 24);
        }

        uint64_t swapUInt64(uint64_t value)
        {
            return ((value & 0x00000000000000FF) << 56) |
                   ((value & 0x000000000000FF00) << 40) |
                   ((value & 0x0000000000FF0000) << 24) |

                   ((value & 0x00000000FF000000) << 8)  |
                   ((value & 0x000000FF00000000) >> 8)  |

                   ((value & 0x0000FF0000000000) >> 24) |
                   ((value & 0x00FF000000000000) >> 40) |
                   ((value & 0xFF00000000000000) >> 56);
        }

        int8_t readInt8(Bitstream& bs)
        {
            int8_t value = 0;
            readValue(bs, &value);

            return value;
        }

        uint8_t readUInt(Bitstream& bs)
        {
            uint8_t value = 0;
            readValue(bs, &value);

            return value;
        }

        int16_t readInt16(Bitstream& bs)
        {
            int16_t value = 0;
            readValue(bs, &value);

            value = (int16_t)swapUInt16((uint16_t)value);

            return value;
        }

        uint16_t readUInt16(Bitstream& bs)
        {
            uint16_t value = 0;
            readValue(bs, &value);

            value = swapUInt16(value);

            return value;
        }

        int32_t readInt32(Bitstream& bs)
        {
            int32_t value = 0;
            readValue(bs, &value);

            value = (int32_t)swapUInt32((uint32_t)value);

            return value;
        }

        uint32_t readUInt32(Bitstream& bs)
        {
            uint32_t value = 0;
            readValue(bs, &value);

            value = swapUInt32(value);

            return value;
        }

        int64_t readInt64(Bitstream& bs)
        {
            int64_t value = 0;
            readValue(bs, &value);

            value = (int64_t)swapUInt64((uint64_t)value);

            return value;
        }

        uint64_t readUInt64(Bitstream& bs)
        {
            uint64_t value = 0;
            readValue(bs, &value);

            value = swapUInt64(value);

            return value;
        }

        bool isAligned(Bitstream& bs)
        {
            return (bs.bitmask == INITIAL_BITMASK);
        }

        void align(Bitstream& bs)
        {
            if (bs.bitmask != INITIAL_BITMASK)
            {
                bs.bitmask = INITIAL_BITMASK;
                bs.position++;
                bs.position = Math::clamp<size_t>(bs.position, 0, bs.length);
            }
        }

        void shiftBitmask(Bitstream& bs)
        {
            bs.bitmask = (uint8_t)(bs.bitmask >> 1);

            if (bs.bitmask == 0)
            {
                bs.bitmask = INITIAL_BITMASK;
                bs.position++;

                bs.position = Math::clamp<size_t>(bs.position, 0, bs.length);
            }
        }

        uint32_t readBits(Bitstream& bs, size_t count)
        {
            assert(count <= 32);

            LOG_V("PCCBitstream::read = bits: %d, position: %lu", count, bs.position);

            uint32_t result = 0;

            for (size_t i = 0; i < count; i++)
            {
                uint8_t byte = bs.data[bs.position];
                bool bit = ((byte & bs.bitmask) != 0);

                if (bit)
                {
                    result = (result << 1) | 1;
                }
                else
                {
                    result = (result << 1);
                }

                shiftBitmask(bs);
            }

            return result;
        }

        void seek(Bitstream& bs, ssize_t count)
        {
            bs.bitmask = INITIAL_BITMASK;
            bs.position += count;
            bs.position = Math::clamp<size_t>(bs.position, 0, bs.length);
        }

        void skipBytes(Bitstream& bs, size_t count)
        {
            LOG_E("PCCBitstream::skipBytes = bytes: %d, position: %lu", count, bs.position);

            align(bs);

            bs.position += count;
            bs.position = Math::clamp<size_t>(bs.position, 0, bs.length);
        }

        void skipBits(Bitstream& bs, size_t count)
        {
            size_t skipBytes = (count / 8);

            while(skipBytes)
            {
                skipBytes--;
                bs.position++;
            }

            uint8_t num = count % 8;

            for (uint8_t i = 0; i < num; i++)
            {
                shiftBitmask(bs);
            }
        }

        size_t readBytes(Bitstream& bs, uint8_t* buffer, size_t size)
        {
            align(bs);

            size_t remainingBytes = bytesAvailable(bs);
            size_t bytesRead = Math::min(remainingBytes, size);

            memcpy(buffer, (bs.data + bs.position), bytesRead);

            bs.position += bytesRead;

            return bytesRead;
        }

        size_t bytesAvailable(Bitstream& bs)
        {
            size_t result = bs.length - bs.position;

            return result;
        }

        uint32_t readUVLC(Bitstream& bs)
        {
            uint32_t value = 0;
            uint32_t length = 0;

            uint32_t code = readBits(bs, 1);

            if (code == 0)
            {
                length = 0;

                while (!(code & 1))
                {
                    code = readBits(bs, 1);
                    length++;
                }

                value = readBits(bs, length);
                value += (1 << length) - 1;
            }

            return value;
        }

        int32_t readSVLC(Bitstream& bs)
        {
            uint32_t bits = readUVLC(bs);

            if (bits & 1)
            {
                return (int32_t)(bits >> 1) + 1;
            }

            return -(int32_t)(bits >> 1);
        }
    }
}

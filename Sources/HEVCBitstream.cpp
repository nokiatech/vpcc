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

#include "HEVCBitstream.h"

#include "Helpers.h"
#include "Logger.h"

#include <assert.h>
#include <cstring>

namespace HEVC
{
    Bitstream::Bitstream()
    : _position(0)
    , _bitmask(INITIAL_BITMASK)
    , _data(NULL)
    , _length(0)
    {
    }

    Bitstream::Bitstream(const Bitstream& bitstream)
    : _position(bitstream._position)
    , _bitmask(bitstream._bitmask)
    , _data(bitstream._data)
    , _length(bitstream._length)
    {
    }

    Bitstream::Bitstream(uint8_t* data, size_t length)
    : _position(0)
    , _bitmask(INITIAL_BITMASK)
    , _data(data)
    , _length(length)
    {
    }

    Bitstream::~Bitstream()
    {
    }

    Bitstream& Bitstream::operator = (const Bitstream& bitstream)
    {
        _position = bitstream._position;
        _bitmask = bitstream._bitmask;
        _data = bitstream._data;
        _length = bitstream._length;

        return *this;
    }

    void Bitstream::setData(uint8_t *data, size_t length)
    {
        resetPosition();

        _data = data;
        _length = length;
    }

    int8_t Bitstream::readInt8()
    {
        int8_t value = 0;
        readValue(&value);

        return value;
    }

    uint8_t Bitstream::readUInt()
    {
        uint8_t value = 0;
        readValue(&value);

        return value;
    }

    int16_t Bitstream::readInt16()
    {
        int16_t value = 0;
        readValue(&value);

        return value;
    }

    uint16_t Bitstream::readUInt16()
    {
        uint16_t value = 0;
        readValue(&value);

        return value;
    }

    int32_t Bitstream::readInt32()
    {
        int32_t value = 0;
        readValue(&value);

        return value;
    }

    uint32_t Bitstream::readUInt32()
    {
        uint32_t value = 0;
        readValue(&value);

        return value;
    }

    int64_t Bitstream::readInt64()
    {
        int64_t value = 0;
        readValue(&value);

        return value;
    }

    uint64_t Bitstream::readUInt64()
    {
        uint64_t value = 0;
        readValue(&value);

        return value;
    }

    float Bitstream::readFloat()
    {
        float value = 0;
        readValue(&value);

        return value;
    }

    double Bitstream::readDouble()
    {
        double value = 0;
        readValue(&value);

        return value;
    }

    void Bitstream::setPosition(size_t position)
    {
        _position = position;
    }

    size_t Bitstream::position() const
    {
        return _position;
    }

    void Bitstream::setBitmask(uint8_t bitmask)
    {
       _bitmask = bitmask;
    }

    uint8_t Bitstream::bitmask() const
    {
        return _bitmask;
    }

    size_t Bitstream::length() const
    {
        return _length;
    }

    uint8_t* Bitstream::data()
    {
        return _data;
    }

    void Bitstream::align()
    {
        if (_bitmask != INITIAL_BITMASK)
        {
            _bitmask = INITIAL_BITMASK;
            
            _position++;
            _position = Math::clamp<size_t>(_position, 0, _length);
        }
    }

    void Bitstream::resetPosition()
    {
        _position = 0;
        _bitmask = INITIAL_BITMASK;
    }

    void Bitstream::shiftBitmask()
    {
        _bitmask = (uint8_t)(_bitmask >> 1);

        if (_bitmask == 0)
        {
            _bitmask = INITIAL_BITMASK;
            _position++;

            handleEPB();

            _position = Math::clamp<size_t>(_position, 0, _length);
        }
    }

    uint32_t Bitstream::readBits(size_t count)
    {
        assert(count <= 32);

        uint32_t result = 0;

        for (size_t i = 0; i < count; i++)
        {
            uint8_t byte = _data[_position];
            bool bit = ((byte & _bitmask) != 0);

            if (bit)
            {
                result = (result << 1) | 1;
            }
            else
            {
                result = (result << 1);
            }

            shiftBitmask();
        }

        return result;
    }

    void Bitstream::seek(ssize_t count)
    {
        setBitmask(INITIAL_BITMASK);

        _position += count;
        _position = Math::clamp<size_t>(_position, 0, _length);
    }

    void Bitstream::skipBytes(size_t count)
    {
        align();

        _position += count;
        _position = Math::clamp<size_t>(_position, 0, _length);
    }

    void Bitstream::skipBits(size_t count)
    {
        handleEPB();

        size_t skipBytes = (count / 8);

        while(skipBytes)
        {
            skipBytes--;
            _position++;

            handleEPB();
        }

        // Shift bit-mask
        uint8_t num = count % 8;

        for (uint8_t i = 0; i < num; i++)
        {
            shiftBitmask();
        }
    }

    size_t Bitstream::readBytes(uint8_t* buffer, size_t size)
    {
        align();

        size_t remainingBytes = bytesAvailable();
        size_t bytesRead = Math::min(remainingBytes, size);

        memcpy(buffer, (_data + _position), bytesRead);

        _position += bytesRead;

        return bytesRead;
    }

    size_t Bitstream::bytesAvailable() const
    {
        size_t result = _length - _position;

        return result;
    }

    void Bitstream::handleEPB()
    {
        if(_position >= 2)
        {
            if(_data[_position - 2] == 0x00 && _data[_position - 1] == 0x00 && _data[_position] == 0x03)
            {
                _position++;
            }
        }
    }

    uint32_t Bitstream::readUGolomb()
    {
        uint32_t numZeroBits = -1;

        for (uint32_t bit = 0; !bit; numZeroBits++)
        {
            bit = readBits(1);
        }

        if (numZeroBits >= 32)
        {
            return 0;
        }

        uint32_t bits = readBits(numZeroBits);
        uint32_t result = (1 << numZeroBits) - 1 + bits;

        return result;
    }

    int32_t Bitstream::readSGolomb()
    {
        int32_t bits = readUGolomb();

        if (bits & 1)
        {
            bits = (bits + 1) >> 1;
        }
        else
        {
            bits = -(bits >> 1);
        }

        return bits;
    }
}

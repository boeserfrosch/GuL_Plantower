#ifndef GUL_PLANTOWER_TEST_ARDUINO_H
#define GUL_PLANTOWER_TEST_ARDUINO_H

#include <cstddef>
#include <cstdint>

using size_t = std::size_t;
using uint8_t = std::uint8_t;
using uint16_t = std::uint16_t;
using int32_t = std::int32_t;

class Print
{
public:
    virtual ~Print() {}
    virtual size_t write(uint8_t value) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size)
    {
        size_t written = 0;
        for (size_t i = 0; i < size; i++)
        {
            written += write(buffer[i]);
        }
        return written;
    }
};

class Stream : public Print
{
public:
    virtual ~Stream() {}
    virtual int available() = 0;
    virtual int read() = 0;
    virtual int peek() = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
};

#endif

#ifndef GUL_PLANTOWER_UARTINTERFACE_H
#define GUL_PLANTOWER_UARTINTERFACE_H

#include <stdint.h>
#include <stddef.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

class UARTInterface
{
public:
    virtual ~UARTInterface() = default;
    virtual void begin(unsigned long baud, uint8_t bits, uint8_t parity, uint8_t stop) {}
    virtual int available() = 0;
    virtual int peek() = 0;
    virtual int read() = 0;
    virtual size_t write(uint8_t byte) = 0;
    virtual size_t write(const uint8_t *buffer, size_t size) = 0;
};

#ifdef ARDUINO
class ArduinoHardwareSerialWrapper : public UARTInterface
{
public:
    ArduinoHardwareSerialWrapper() = default;
    explicit ArduinoHardwareSerialWrapper(HardwareSerial &stream) : _stream(&stream) {}

    void init(HardwareSerial &stream) { _stream = &stream; }
    void begin(unsigned long baud, uint8_t bits, uint8_t parity, uint8_t stop) override
    {
        if (_stream)
            _stream->begin(baud, bits, parity, stop);
    }
    int available() override { return _stream ? _stream->available() : 0; }
    int peek() override { return _stream ? _stream->peek() : -1; }
    int read() override { return _stream ? _stream->read() : -1; }
    size_t write(uint8_t byte) override { return _stream ? _stream->write(byte) : 0; }
    size_t write(const uint8_t *buffer, size_t size) override { return _stream ? _stream->write(buffer, size) : 0; }

private:
    HardwareSerial *_stream = nullptr;
};

class StreamWrapper : public UARTInterface
{
public:
    StreamWrapper() = default;
    explicit StreamWrapper(Stream &stream) : _stream(&stream) {}

    void init(Stream &stream) { _stream = &stream; }
    void begin(unsigned long, uint8_t, uint8_t, uint8_t) override {}
    int available() override { return _stream ? _stream->available() : 0; }
    int peek() override { return _stream ? _stream->peek() : -1; }
    int read() override { return _stream ? _stream->read() : -1; }
    size_t write(uint8_t byte) override { return _stream ? _stream->write(byte) : 0; }
    size_t write(const uint8_t *buffer, size_t size) override { return _stream ? _stream->write(buffer, size) : 0; }

private:
    Stream *_stream = nullptr;
};
#endif

#endif
#include "UARTInterface.h"

class MockUartInterface : public UARTInterface
{
public:
    MockUartInterface() : readBufferLen(0), readBufferIndex(0), outputLen(0) {}

    int available() override { return (int)(readBufferLen - readBufferIndex); }
    int peek() override { return (readBufferIndex < readBufferLen) ? readBuffer[readBufferIndex] : -1; }
    int read() override { return (readBufferIndex < readBufferLen) ? readBuffer[readBufferIndex++] : -1; }
    size_t write(uint8_t byte) override { return (outputLen < sizeof(output)) ? (output[outputLen++] = byte, 1) : 0; }
    size_t write(const uint8_t *buffer, size_t size) override
    {
        size_t copyLen = (size + outputLen <= sizeof(output)) ? size : (sizeof(output) - outputLen);
        for (size_t i = 0; i < copyLen; i++)
        {
            output[outputLen + i] = buffer[i];
        }
        outputLen += copyLen;
        return copyLen;
    }
    const uint8_t *getOutput() const { return output; }
    size_t getOutputLen() const { return outputLen; }
    void writeToReadBuffer(const uint8_t *data, size_t len)
    {
        size_t copyLen = (len + readBufferWriteIndex < sizeof(readBuffer)) ? len : sizeof(readBuffer) - readBufferWriteIndex;
        for (size_t i = 0; i < copyLen; i++)
        {
            readBuffer[i + readBufferWriteIndex] = data[i];
        }
        readBufferWriteIndex += copyLen;
        readBufferLen = readBufferWriteIndex;
        readBufferIndex = 0;
    }

private:
    uint8_t readBuffer[128];
    size_t readBufferWriteIndex = 0;
    size_t readBufferLen = 0;
    size_t readBufferIndex = 0;
    uint8_t output[128];
    size_t outputLen = 0;
};
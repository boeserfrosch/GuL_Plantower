#include <unity.h>
#include "Plantower.h"

class FakeStream : public Stream
{
public:
    FakeStream(const uint8_t *data, size_t len)
    {
        inputLen = len > sizeof(input) ? sizeof(input) : len;
        for (size_t i = 0; i < inputLen; i++)
        {
            input[i] = data[i];
        }
        inputPos = 0;
        outputLen = 0;
    }

    int available() override
    {
        return (inputPos < inputLen) ? (int)(inputLen - inputPos) : 0;
    }

    int read() override
    {
        if (inputPos >= inputLen)
        {
            return -1;
        }
        return input[inputPos++];
    }

    int peek() override
    {
        if (inputPos >= inputLen)
        {
            return -1;
        }
        return input[inputPos];
    }

    size_t write(uint8_t value) override
    {
        if (outputLen < sizeof(output))
        {
            output[outputLen++] = value;
        }
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) override
    {
        outputLen = size > sizeof(output) ? sizeof(output) : size;
        for (size_t i = 0; i < outputLen; i++)
        {
            output[i] = buffer[i];
        }
        return size;
    }

    const uint8_t *getOutput() const { return output; }
    size_t getOutputLen() const { return outputLen; }

private:
    uint8_t input[64];
    size_t inputLen;
    size_t inputPos;

    uint8_t output[64];
    size_t outputLen;
};

static void setWord(uint8_t *buffer, size_t index, uint16_t value)
{
    buffer[index] = (value >> 8) & 0xFF;
    buffer[index + 1] = value & 0xFF;
}

static void buildFrame(uint8_t *frame, size_t frameLen)
{
    for (size_t i = 0; i < frameLen; i++)
    {
        frame[i] = 0;
    }

    frame[0] = 0x42;
    frame[1] = 0x4D;
    frame[2] = 0x00;
    frame[3] = 0x1C; // 28 bytes

    setWord(frame, 4, 12);   // PM1_STD
    setWord(frame, 6, 34);   // PM2_5_STD
    setWord(frame, 8, 56);   // PM10_STD
    setWord(frame, 10, 78);  // PM1_ATM
    setWord(frame, 12, 90);  // PM2_5_ATM
    setWord(frame, 14, 123); // PM10_ATM

    setWord(frame, 16, 1000); // PN300
    setWord(frame, 18, 2000); // PN500
    setWord(frame, 20, 3000); // PN1000
    setWord(frame, 22, 4000); // PN2500
    setWord(frame, 24, 5000); // PN5000
    setWord(frame, 26, 6000); // PN10000

    setWord(frame, 28, 0); // unused/reserved

    uint16_t sum = 0;
    for (size_t i = 0; i < frameLen - 2; i++)
    {
        sum += frame[i];
    }
    frame[frameLen - 2] = (sum >> 8) & 0xFF;
    frame[frameLen - 1] = sum & 0xFF;
}

static void buildFrameShort(uint8_t *frame, size_t frameLen)
{
    for (size_t i = 0; i < frameLen; i++)
    {
        frame[i] = 0;
    }

    frame[0] = 0x42;
    frame[1] = 0x4D;
    frame[2] = 0x00;
    frame[3] = 0x14; // 20 bytes

    setWord(frame, 4, 12);   // PM1_STD
    setWord(frame, 6, 34);   // PM2_5_STD
    setWord(frame, 8, 56);   // PM10_STD
    setWord(frame, 10, 78);  // PM1_ATM
    setWord(frame, 12, 90);  // PM2_5_ATM
    setWord(frame, 14, 123); // PM10_ATM

    uint16_t sum = 0;
    for (size_t i = 0; i < frameLen - 2; i++)
    {
        sum += frame[i];
    }
    frame[frameLen - 2] = (sum >> 8) & 0xFF;
    frame[frameLen - 1] = sum & 0xFF;
}

void test_read_parses_frame()
{
    uint8_t frame[32];
    buildFrame(frame, sizeof(frame));

    FakeStream stream(frame, sizeof(frame));
    GuL::Plantower plantower(stream);

    TEST_ASSERT_TRUE(plantower.read());
    TEST_ASSERT_EQUAL(12, plantower.getPM1_STD());
    TEST_ASSERT_EQUAL(34, plantower.getPM2_5_STD());
    TEST_ASSERT_EQUAL(56, plantower.getPM10_STD());
    TEST_ASSERT_EQUAL(78, plantower.getPM1_ATM());
    TEST_ASSERT_EQUAL(90, plantower.getPM2_5_ATM());
    TEST_ASSERT_EQUAL(123, plantower.getPM10_ATM());
    TEST_ASSERT_EQUAL(1000, plantower.getCntBeyond300nm());
    TEST_ASSERT_EQUAL(2000, plantower.getCntBeyond500nm());
    TEST_ASSERT_EQUAL(3000, plantower.getCntBeyond1000nm());
    TEST_ASSERT_EQUAL(4000, plantower.getCntBeyond2500nm());
    TEST_ASSERT_EQUAL(5000, plantower.getCntBeyond5000nm());
    TEST_ASSERT_EQUAL(6000, plantower.getCntBeyond10000nm());
}

void test_poll_writes_checksum()
{
    uint8_t empty[1] = {0};
    FakeStream stream(empty, 0);
    GuL::Plantower plantower(stream);

    TEST_ASSERT_TRUE(plantower.poll());
    TEST_ASSERT_EQUAL(7, stream.getOutputLen());
    const uint8_t *out = stream.getOutput();
    TEST_ASSERT_EQUAL_HEX8(0x42, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x4D, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xE2, out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[3]);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[4]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[5]);
    TEST_ASSERT_EQUAL_HEX8(0x71, out[6]);
}

void test_read_rejects_bad_checksum()
{
    uint8_t frame[32];
    buildFrame(frame, sizeof(frame));
    frame[31] ^= 0xFF;

    FakeStream stream(frame, sizeof(frame));
    GuL::Plantower plantower(stream);

    TEST_ASSERT_FALSE(plantower.read());
}

void test_read_skips_garbage_before_frame()
{
    uint8_t frame[32];
    buildFrame(frame, sizeof(frame));

    uint8_t buffer[35];
    buffer[0] = 0x10;
    buffer[1] = 0x20;
    buffer[2] = 0x30;
    for (size_t i = 0; i < sizeof(frame); i++)
    {
        buffer[i + 3] = frame[i];
    }

    FakeStream stream(buffer, sizeof(buffer));
    GuL::Plantower plantower(stream);

    TEST_ASSERT_TRUE(plantower.read());
    TEST_ASSERT_EQUAL(12, plantower.getPM1_STD());
}

void test_read_length20_parses_base_pm_only()
{
    uint8_t frame[24];
    buildFrameShort(frame, sizeof(frame));

    FakeStream stream(frame, sizeof(frame));
    GuL::Plantower plantower(stream);

    TEST_ASSERT_TRUE(plantower.read());
    TEST_ASSERT_EQUAL(12, plantower.getPM1_STD());
    TEST_ASSERT_EQUAL(34, plantower.getPM2_5_STD());
    TEST_ASSERT_EQUAL(56, plantower.getPM10_STD());
    TEST_ASSERT_EQUAL(-1, plantower.getCntBeyond300nm());
    TEST_ASSERT_EQUAL(-1, plantower.getCntBeyond500nm());
    TEST_ASSERT_EQUAL(-1, plantower.getCntBeyond1000nm());
    TEST_ASSERT_EQUAL(-1, plantower.getCntBeyond2500nm());
    TEST_ASSERT_EQUAL(-1, plantower.getCntBeyond5000nm());
    TEST_ASSERT_EQUAL(-1, plantower.getCntBeyond10000nm());
}

void setUp() {}
void tearDown() {}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_read_parses_frame);
    RUN_TEST(test_poll_writes_checksum);
    RUN_TEST(test_read_rejects_bad_checksum);
    RUN_TEST(test_read_skips_garbage_before_frame);
    RUN_TEST(test_read_length20_parses_base_pm_only);
    return UNITY_END();
}

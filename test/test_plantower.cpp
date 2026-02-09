#include <unity.h>
#include <Plantower.h>

class FakeStream : public Stream
{
public:
    FakeStream(const uint8_t *data, size_t length)
        : _data(data), _length(length), _index(0), _outputLen(0)
    {
    }

    int available() override
    {
        return (_index < _length) ? (int)(_length - _index) : 0;
    }

    int read() override
    {
        if (_index >= _length)
        {
            return -1;
        }
        return _data[_index++];
    }

    int peek() override
    {
        if (_index >= _length)
        {
            return -1;
        }
        return _data[_index];
    }

    size_t write(uint8_t value) override
    {
        if (_outputLen < sizeof(_output))
        {
            _output[_outputLen++] = value;
        }
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size) override
    {
        _outputLen = (size > sizeof(_output)) ? sizeof(_output) : size;
        for (size_t i = 0; i < _outputLen; i++)
        {
            _output[i] = buffer[i];
        }
        return size;
    }

    const uint8_t *getOutput() const { return _output; }
    size_t getOutputLen() const { return _outputLen; }

private:
    const uint8_t *_data;
    size_t _length;
    size_t _index;
    uint8_t _output[64];
    size_t _outputLen;
};

static uint16_t calcChecksum(const uint8_t *data, size_t count)
{
    uint16_t sum = 0;
    for (size_t i = 0; i < count; i++)
    {
        sum += data[i];
    }
    return sum;
}

static void buildFrame(uint8_t *frame, size_t frameLength)
{
    const size_t payloadLength = frameLength + 4;
    for (size_t i = 0; i < payloadLength; i++)
    {
        frame[i] = 0;
    }

    frame[0] = 0x42;
    frame[1] = 0x4D;
    frame[2] = (uint8_t)((frameLength >> 8) & 0xFF);
    frame[3] = (uint8_t)(frameLength & 0xFF);

    // Base PM values
    const uint16_t pm1_std = 11;
    const uint16_t pm2_5_std = 22;
    const uint16_t pm10_std = 33;
    const uint16_t pm1_atm = 44;
    const uint16_t pm2_5_atm = 55;
    const uint16_t pm10_atm = 66;

    // Base PN values
    const uint16_t pn300 = 77;
    const uint16_t pn500 = 88;
    const uint16_t pn1000 = 99;
    const uint16_t pn2500 = 111;
    const uint16_t pn5000 = 122;
    const uint16_t pn10000 = 133;

    frame[4] = (uint8_t)(pm1_std >> 8);
    frame[5] = (uint8_t)(pm1_std & 0xFF);
    frame[6] = (uint8_t)(pm2_5_std >> 8);
    frame[7] = (uint8_t)(pm2_5_std & 0xFF);
    frame[8] = (uint8_t)(pm10_std >> 8);
    frame[9] = (uint8_t)(pm10_std & 0xFF);
    frame[10] = (uint8_t)(pm1_atm >> 8);
    frame[11] = (uint8_t)(pm1_atm & 0xFF);
    frame[12] = (uint8_t)(pm2_5_atm >> 8);
    frame[13] = (uint8_t)(pm2_5_atm & 0xFF);
    frame[14] = (uint8_t)(pm10_atm >> 8);
    frame[15] = (uint8_t)(pm10_atm & 0xFF);

    if (frameLength >= 28)
    {
        frame[16] = (uint8_t)(pn300 >> 8);
        frame[17] = (uint8_t)(pn300 & 0xFF);
        frame[18] = (uint8_t)(pn500 >> 8);
        frame[19] = (uint8_t)(pn500 & 0xFF);
        frame[20] = (uint8_t)(pn1000 >> 8);
        frame[21] = (uint8_t)(pn1000 & 0xFF);
        frame[22] = (uint8_t)(pn2500 >> 8);
        frame[23] = (uint8_t)(pn2500 & 0xFF);
        frame[24] = (uint8_t)(pn5000 >> 8);
        frame[25] = (uint8_t)(pn5000 & 0xFF);
        frame[26] = (uint8_t)(pn10000 >> 8);
        frame[27] = (uint8_t)(pn10000 & 0xFF);

        // Version & error (not unpacked by library, but part of payload)
        frame[28] = 0x00;
        frame[29] = 0x00;
    }

    const uint16_t checksum = calcChecksum(frame, payloadLength - 2);
    frame[payloadLength - 2] = (uint8_t)((checksum >> 8) & 0xFF);
    frame[payloadLength - 1] = (uint8_t)(checksum & 0xFF);
}

void test_read_frame_parses_values()
{
    const size_t frameLength = 28;
    uint8_t frame[frameLength + 4];
    buildFrame(frame, frameLength);

    FakeStream stream(frame, sizeof(frame));
    GuL::Plantower sensor(stream);

    bool gotFrame = false;
    for (int i = 0; i < 10; i++)
    {
        if (sensor.read())
        {
            gotFrame = true;
            break;
        }
    }

    TEST_ASSERT_TRUE(gotFrame);
    TEST_ASSERT_EQUAL(11, sensor.getPM1_STD());
    TEST_ASSERT_EQUAL(22, sensor.getPM2_5_STD());
    TEST_ASSERT_EQUAL(33, sensor.getPM10_STD());
    TEST_ASSERT_EQUAL(44, sensor.getPM1_ATM());
    TEST_ASSERT_EQUAL(55, sensor.getPM2_5_ATM());
    TEST_ASSERT_EQUAL(66, sensor.getPM10_ATM());
    TEST_ASSERT_EQUAL(77, sensor.getCntBeyond300nm());
    TEST_ASSERT_EQUAL(88, sensor.getCntBeyond500nm());
    TEST_ASSERT_EQUAL(99, sensor.getCntBeyond1000nm());
    TEST_ASSERT_EQUAL(111, sensor.getCntBeyond2500nm());
    TEST_ASSERT_EQUAL(122, sensor.getCntBeyond5000nm());
    TEST_ASSERT_EQUAL(133, sensor.getCntBeyond10000nm());
}

void test_read_rejects_bad_checksum()
{
    const size_t frameLength = 28;
    uint8_t frame[frameLength + 4];
    buildFrame(frame, frameLength);

    frame[frameLength + 3] ^= 0xFF;

    FakeStream stream(frame, sizeof(frame));
    GuL::Plantower sensor(stream);

    TEST_ASSERT_FALSE(sensor.read());
}

void test_read_skips_garbage_before_frame()
{
    const size_t frameLength = 28;
    uint8_t frame[frameLength + 4];
    buildFrame(frame, frameLength);

    uint8_t buffer[frameLength + 7];
    buffer[0] = 0x00;
    buffer[1] = 0x11;
    buffer[2] = 0x22;
    for (size_t i = 0; i < sizeof(frame); i++)
    {
        buffer[i + 3] = frame[i];
    }

    FakeStream stream(buffer, sizeof(buffer));
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.read());
    TEST_ASSERT_EQUAL(11, sensor.getPM1_STD());
}

void test_read_length20_parses_base_pm_only()
{
    const size_t frameLength = 20;
    uint8_t frame[frameLength + 4];
    buildFrame(frame, frameLength);

    FakeStream stream(frame, sizeof(frame));
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.read());
    TEST_ASSERT_EQUAL(11, sensor.getPM1_STD());
    TEST_ASSERT_EQUAL(22, sensor.getPM2_5_STD());
    TEST_ASSERT_EQUAL(33, sensor.getPM10_STD());
    TEST_ASSERT_EQUAL(-1, sensor.getCntBeyond300nm());
    TEST_ASSERT_EQUAL(-1, sensor.getCntBeyond500nm());
    TEST_ASSERT_EQUAL(-1, sensor.getCntBeyond1000nm());
    TEST_ASSERT_EQUAL(-1, sensor.getCntBeyond2500nm());
    TEST_ASSERT_EQUAL(-1, sensor.getCntBeyond5000nm());
    TEST_ASSERT_EQUAL(-1, sensor.getCntBeyond10000nm());
}

void test_poll_writes_checksum()
{
    uint8_t empty[1] = {0};
    FakeStream stream(empty, 0);
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.poll());
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

void setUp() {}
void tearDown() {}

int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_read_frame_parses_values);
    RUN_TEST(test_read_rejects_bad_checksum);
    RUN_TEST(test_read_skips_garbage_before_frame);
    RUN_TEST(test_read_length20_parses_base_pm_only);
    RUN_TEST(test_poll_writes_checksum);
    return UNITY_END();
}

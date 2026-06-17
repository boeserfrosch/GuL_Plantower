#include <unity.h>
#include <Plantower.h>

#include "../mock/MockSerial.h"

class PlantowerTest : public GuL::Plantower
{
public:
    PlantowerTest(GuL::HAL::ISerial &uart) : Plantower(uart) {}
    static uint16_t publicCalcChecksum(const uint8_t *cmd, size_t cnt)
    {
        return calcChecksum(cmd, cnt);
    }
};

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

    const uint16_t checksum = PlantowerTest::publicCalcChecksum(frame, payloadLength - 2);
    frame[payloadLength - 2] = (uint8_t)((checksum >> 8) & 0xFF);
    frame[payloadLength - 1] = (uint8_t)(checksum & 0xFF);
}

void test_read_frame_parses_values()
{
    const size_t frameLength = 28;
    uint8_t frame[frameLength + 4];
    buildFrame(frame, frameLength);

    MockSerial stream;
    stream.setInput(frame, sizeof(frame));
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

    MockSerial stream;
    stream.setInput(frame, sizeof(frame));
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

    MockSerial stream;
    stream.setInput(buffer, sizeof(buffer));
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.read());
    TEST_ASSERT_EQUAL(11, sensor.getPM1_STD());
}

void test_read_length20_parses_base_pm_only()
{
    const size_t frameLength = 20;
    uint8_t frame[frameLength + 4];
    buildFrame(frame, frameLength);

    MockSerial stream;
    stream.setInput(frame, sizeof(frame));
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
    MockSerial stream;
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

void test_checksum_calculation()
{
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    uint16_t checksum = PlantowerTest::publicCalcChecksum(data, sizeof(data));
    TEST_ASSERT_EQUAL_HEX16(0x0A, checksum);
    TEST_ASSERT_EQUAL_HEX8(0x00, (checksum >> 8) & 0xFF);
    TEST_ASSERT_EQUAL_HEX8(0x0A, checksum & 0xFF);
    uint8_t data2[] = {0xFF, 0xFF, 0xFF, 0xFF};
    checksum = PlantowerTest::publicCalcChecksum(data2, sizeof(data2));
    TEST_ASSERT_EQUAL_HEX16(0x3FC, checksum);
    uint8_t data3[] = {0x00, 0x00, 0x00, 0x00};
    checksum = PlantowerTest::publicCalcChecksum(data3, sizeof(data3));
    TEST_ASSERT_EQUAL_HEX16(0x00, checksum);
    uint8_t data4[] = {0x12, 0x34, 0x56, 0x78};
    checksum = PlantowerTest::publicCalcChecksum(data4, sizeof(data4));
    TEST_ASSERT_EQUAL_HEX16(0x114, checksum);
    uint8_t data5[] = {0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
    checksum = PlantowerTest::publicCalcChecksum(data5, sizeof(data5));
    TEST_ASSERT_EQUAL_HEX16(0x9F6, checksum);
    uint8_t data6[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    checksum = PlantowerTest::publicCalcChecksum(data6, sizeof(data6));
    TEST_ASSERT_EQUAL_HEX16(0x01, checksum);
    uint8_t data7[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    checksum = PlantowerTest::publicCalcChecksum(data7, sizeof(data7));
    TEST_ASSERT_EQUAL_HEX16(0xFF0, checksum);
}

void test_failure_states_are_set()
{
    MockSerial stream;
    GuL::Plantower sensor(stream);

    // No data available
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::NO_BYTES_AVAILABLE, sensor.getLastFailureReason());

    // Bad checksum
    uint8_t frame[32];
    buildFrame(frame, 28);
    frame[30] ^= 0xFF;
    stream.setInput(frame, sizeof(frame));
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::CHECKSUM_FAILURE, sensor.getLastFailureReason());

    // Invalid frame length
    uint8_t badFrame[32];
    buildFrame(badFrame, 32);
    badFrame[2] = 0x00; // High byte of length
    badFrame[3] = 0x00; // And low byte of length, making it 0 - which is unexpected for a valid frame
    stream.setInput(badFrame, sizeof(badFrame));
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INVALID_FRAME_LENGTH, sensor.getLastFailureReason());

    // UART write failure
    stream.setInput(frame, sizeof(frame));
    stream.disableWrites();
    TEST_ASSERT_FALSE(sensor.poll());
    TEST_ASSERT_EQUAL(GuL::Plantower::UART_WRITE_FAILURE, sensor.getLastFailureReason());
    stream.enableWrites();

    // Incomplete frame
    uint8_t incompleteFrame[3];
    incompleteFrame[0] = 0x42;
    incompleteFrame[1] = 0x4D;
    incompleteFrame[2] = 0x00;
    stream.setInput(incompleteFrame, sizeof(incompleteFrame));

    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());

    // Missing bytes (payload length exceeds buffer)
    uint8_t longFrame[40];
    buildFrame(longFrame, 36);
    stream.setInput(longFrame, sizeof(longFrame));
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INVALID_FRAME_LENGTH, sensor.getLastFailureReason());
}

void test_read_proceeds_after_incomplete_frame()
{
    uint8_t buffer[40];
    const size_t frameLength = 28;
    buildFrame(buffer, frameLength);

    MockSerial stream;
    // First provide an incomplete frame (only first header byte)
    stream.setInput(buffer, 1);

    GuL::Plantower sensor(stream);
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());
    stream.appendToInput(buffer + 1, 1); // Now append the next byte
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());
    stream.appendToInput(buffer + 2, 1); // Now append the length high byte
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());
    stream.appendToInput(buffer + 3, 1); // Now append the length low byte
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());
    // Now each payload byte without the checksum
    for (size_t i = 4; i < frameLength + 2; i++)
    {
        stream.appendToInput(buffer + i, 1);
        TEST_ASSERT_FALSE(sensor.read());
        TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());
    }
    // Finally append the checksum bytes one by one
    stream.appendToInput(buffer + frameLength + 2, 1);
    TEST_ASSERT_FALSE(sensor.read());
    TEST_ASSERT_EQUAL(GuL::Plantower::INCOMPLETE_FRAME, sensor.getLastFailureReason());
    stream.appendToInput(buffer + frameLength + 3, 1);
    TEST_ASSERT_TRUE(sensor.read());
    TEST_ASSERT_EQUAL(11, sensor.getPM1_STD());
}

void setUp() {}
void tearDown() {}

int processTests()
{
    UNITY_BEGIN();
    RUN_TEST(test_read_frame_parses_values);
    RUN_TEST(test_read_rejects_bad_checksum);
    RUN_TEST(test_read_skips_garbage_before_frame);
    RUN_TEST(test_read_length20_parses_base_pm_only);
    RUN_TEST(test_poll_writes_checksum);
    RUN_TEST(test_checksum_calculation);
    RUN_TEST(test_failure_states_are_set);
    RUN_TEST(test_read_proceeds_after_incomplete_frame);
    return UNITY_END();
}

#if defined(ARDUINO)
void setup()
{
    Serial.begin(115200);
    delay(2000);
    processTests();
}

void loop() {}
#else
int main(int argc, char *argv[])
{
    return processTests();
}
#endif
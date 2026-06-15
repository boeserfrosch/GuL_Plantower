#include <PMS7003.h>

#include <unity.h>
#include "../mock/MockUartInterface.h"

void setUp() {}
void tearDown() {}

void test_sleep_writes_command()
{
    MockUartInterface stream;
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.sleep());
    TEST_ASSERT_EQUAL(7, stream.getOutputLen());
    const uint8_t *out = stream.getOutput();
    TEST_ASSERT_EQUAL_HEX8(0x42, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x4D, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xE4, out[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, out[4]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[5]);
    TEST_ASSERT_EQUAL_HEX8(0x73, out[6]);
}

void test_wakeup_writes_command()
{
    MockUartInterface stream;
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.wakeup());
    TEST_ASSERT_EQUAL(7, stream.getOutputLen());
    const uint8_t *out = stream.getOutput();
    TEST_ASSERT_EQUAL_HEX8(0x42, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x4D, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xE4, out[2]);
    // out[3] is not specified in the protocol and can be anything, so we don't check it here
    TEST_ASSERT_EQUAL_HEX8(0x01, out[4]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[5]);
    TEST_ASSERT_EQUAL_HEX8(0x74, out[6]);
}

void test_setToPassiveReporting_writes_command()
{
    MockUartInterface stream;
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.setToPassiveReporting());
    TEST_ASSERT_EQUAL(7, stream.getOutputLen());
    const uint8_t *out = stream.getOutput();
    TEST_ASSERT_EQUAL_HEX8(0x42, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x4D, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xE1, out[2]);
    // out[3] is not specified in the protocol and can be anything, so we don't check it here
    TEST_ASSERT_EQUAL_HEX8(0x00, out[4]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[5]);
    TEST_ASSERT_EQUAL_HEX8(0x70, out[6]);
}

void test_setToActiveReporting_writes_command()
{
    MockUartInterface stream;
    GuL::Plantower sensor(stream);

    TEST_ASSERT_TRUE(sensor.setToActiveReporting());
    TEST_ASSERT_EQUAL(7, stream.getOutputLen());
    const uint8_t *out = stream.getOutput();
    TEST_ASSERT_EQUAL_HEX8(0x42, out[0]);
    TEST_ASSERT_EQUAL_HEX8(0x4D, out[1]);
    TEST_ASSERT_EQUAL_HEX8(0xE1, out[2]);
    // out[3] is not specified in the protocol and can be anything, so we don't check it here
    TEST_ASSERT_EQUAL_HEX8(0x01, out[4]);
    TEST_ASSERT_EQUAL_HEX8(0x01, out[5]);
    TEST_ASSERT_EQUAL_HEX8(0x71, out[6]);
}

void test_poll_writes_checksum()
{
    MockUartInterface stream;
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

void test_read_rejects_bad_checksum()
{
    const size_t frameLength = 28;
    uint8_t frame[frameLength + 4];
    frame[0] = 0x42;
    frame[1] = 0x4D;
    frame[2] = 0x00;
    frame[3] = 0x1C; // 28 bytes
    frame[4] = 0x00;
    frame[5] = 0x0B; // PM1_STD
    frame[6] = 0x00;
    frame[7] = 0x22; // PM2_5_STD
    frame[8] = 0x00;
    frame[9] = 0x33; // PM10_STD
    for (size_t i = 10; i < frameLength + 4; i++)
    {
        frame[i] = 0;
    }
    frame[frameLength + 2] = 0x71; // Checksum for the above data
    frame[frameLength + 3] = 0xFF; // Corrupt the checksum

    MockUartInterface stream;
    stream.writeToReadBuffer(frame, sizeof(frame));
    GuL::Plantower sensor(stream);

    TEST_ASSERT_FALSE(sensor.read());
}

int processTests()
{
    UNITY_BEGIN();
    RUN_TEST(test_sleep_writes_command);
    RUN_TEST(test_wakeup_writes_command);
    RUN_TEST(test_setToPassiveReporting_writes_command);
    RUN_TEST(test_setToActiveReporting_writes_command);
    RUN_TEST(test_poll_writes_checksum);
    RUN_TEST(test_read_rejects_bad_checksum);
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
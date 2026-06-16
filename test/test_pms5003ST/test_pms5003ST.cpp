#include <unity.h>
#include "../mock/MockUartInterface.h"
#include <PMS5003ST.h>

class PlantowerTest : public GuL::Plantower
{
public:
    static uint16_t publicCalcChecksum(const uint8_t *cmd, size_t cnt)
    {
        return calcChecksum(cmd, cnt);
    }
};

void setUp() {}
void tearDown() {}

void test_read_and_parse_frame()
{
    MockUartInterface stream;
    GuL::PMS5003ST sensor(stream);

    uint8_t frame[40];
    frame[0] = 0x42;
    frame[1] = 0x4D;
    frame[2] = 0x00;
    frame[3] = 0x24; // 36 bytes
    frame[4] = 0x00;
    frame[5] = 0x0B; // PM1_STD
    frame[6] = 0x00;
    frame[7] = 0x22; // PM2_5_STD
    frame[8] = 0x00;
    frame[9] = 0x33; // PM10_STD
    frame[10] = 0x00;
    frame[11] = 0x44; // PM1_ATM
    frame[12] = 0x00;
    frame[13] = 0x55; // PM2_5_ATM
    frame[14] = 0x00;
    frame[15] = 0x66; // PM10_ATM
    frame[16] = 0x00;
    frame[17] = 0x4D; // PN300
    frame[18] = 0x00;
    frame[19] = 0x58; // PN500
    frame[20] = 0x00;
    frame[21] = 0x63; // PN1000
    frame[22] = 0x00;
    frame[23] = 0x78; // PN2500
    frame[24] = 0x00;
    frame[25] = 0x8B; // PN5000
    frame[26] = 0x00;
    frame[27] = 0x85; // PN10000
    frame[28] = 0x00;
    frame[29] = 0x06; // Formaldehyde concentration
    frame[30] = 0x00;
    frame[31] = 0x12; // Temperature
    frame[32] = 0x00;
    frame[33] = 0x34; // Humidity
    frame[34] = 0x00; // Reserved
    frame[35] = 0x00; // Reserved
    frame[36] = 0x11; // Version
    frame[37] = 0x22; // Error code
    frame[38] = 0x00; // Checksum high byte (to be calculated)
    frame[39] = 0x00; // Checksum low byte (to be calculated)
    auto cs = PlantowerTest::publicCalcChecksum(frame, 38);
    frame[38] = (uint8_t)((cs >> 8) & 0xFF);
    frame[39] = (uint8_t)(cs & 0xFF);
    stream.setInput(frame, sizeof(frame));

    TEST_ASSERT_TRUE(sensor.poll());
    TEST_ASSERT_TRUE(sensor.read());
    TEST_ASSERT_EQUAL(11, sensor.getPM1_STD());
    TEST_ASSERT_EQUAL(34, sensor.getPM2_5_STD());
    TEST_ASSERT_EQUAL(51, sensor.getPM10_STD());
    TEST_ASSERT_EQUAL(68, sensor.getPM1_ATM());
    TEST_ASSERT_EQUAL(85, sensor.getPM2_5_ATM());
    TEST_ASSERT_EQUAL(102, sensor.getPM10_ATM());
    TEST_ASSERT_EQUAL(77, sensor.getCntBeyond300nm());
    TEST_ASSERT_EQUAL(88, sensor.getCntBeyond500nm());
    TEST_ASSERT_EQUAL(99, sensor.getCntBeyond1000nm());
    TEST_ASSERT_EQUAL(120, sensor.getCntBeyond2500nm());
    TEST_ASSERT_EQUAL(139, sensor.getCntBeyond5000nm());
    TEST_ASSERT_EQUAL(133, sensor.getCntBeyond10000nm());
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 0.006, sensor.getFormaldehydeConcentration());
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 1.8, sensor.getTemperature());
    TEST_ASSERT_FLOAT_WITHIN(0.00001, 5.2, sensor.getHumidity());
    TEST_ASSERT_EQUAL(17, sensor.getVersion());
    TEST_ASSERT_EQUAL(34, sensor.getError());
}

int processTests()
{
    UNITY_BEGIN();
    RUN_TEST(test_read_and_parse_frame);
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
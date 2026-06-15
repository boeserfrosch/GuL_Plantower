#ifdef ARDUINO
#include <Arduino.h>
#endif
#include "PMS7003.h"
#include <unity.h>

void setUp()
{
#ifdef ARDUINO
    Serial2.begin(9600, SERIAL_8N1, 1, 38);
#endif
}

GuL::PMS7003 getPlantowerForTest()
{
#ifdef ARDUINO
    return GuL::PMS7003(Serial2);
#else
#error "This test is meant to be run on hardware with a connected PMS7003 sensor. Please define ARDUINO and set up the Serial2 stream accordingly."
#endif
}

void test_read_frame_parses_values()
{
    GuL::PMS7003 plantower = getPlantowerForTest();

    TEST_ASSERT_TRUE(plantower.poll());
    delay(100);
    TEST_ASSERT_TRUE(plantower.read());
    TEST_ASSERT_TRUE(plantower.getPM1_STD() >= 0);
    TEST_ASSERT_TRUE(plantower.getPM2_5_STD() >= 0);
    TEST_ASSERT_TRUE(plantower.getPM10_STD() >= 0);
    TEST_ASSERT_TRUE(plantower.getPM1_ATM() >= 0);
    TEST_ASSERT_TRUE(plantower.getPM2_5_ATM() >= 0);
    TEST_ASSERT_TRUE(plantower.getPM10_ATM() >= 0);
    TEST_ASSERT_TRUE(plantower.getCntBeyond300nm() >= 0);
    TEST_ASSERT_TRUE(plantower.getCntBeyond500nm() >= 0);
    TEST_ASSERT_TRUE(plantower.getCntBeyond1000nm() >= 0);
    TEST_ASSERT_TRUE(plantower.getCntBeyond2500nm() >= 0);
    TEST_ASSERT_TRUE(plantower.getCntBeyond5000nm() >= 0);
    TEST_ASSERT_TRUE(plantower.getCntBeyond10000nm() >= 0);
}

int processTests()
{
    UNITY_BEGIN();
    RUN_TEST(test_read_frame_parses_values);
    return UNITY_END();
}

#ifdef ARDUINO
void setup()
{
    Serial.begin(115200);
    delay(2000);
    processTests();
}
void loop() {}
#else
int main()
{
    return processTests();
}
#endif
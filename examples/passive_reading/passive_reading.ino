#include <PMS7003.h>

GuL::PMS7003 pms(Serial2);

#define RX2 1
#define TX2 38

std::string outputFormat = "PM1 (STD) \t= % 6d µg/µ3 \n"
                           "PM2.5 (STD) \t= % 6d µg/µ3 \n"
                           "PM10 (STD) \t= % 6d µg/µ3 \n"
                           "PM1 (ATM) \t= % 6d µg/µ3 \n"
                           "PM2.5 (ATM) \t= % 6d µg/µ3 \n"
                           "PM10 (ATM) \t= % 6d µg/µ3 \n"
                           "\n"
                           "PN300 \t= % 6d #\\0.1 l \n"
                           "PN500 \t= % 6d #\\0.1 l \n"
                           "PN1000 \t= % 6d #\\0.1 l \n"
                           "PN2500 \t= % 6d #\\0.1 l \n"
                           "PN5000 \t= % 6d #\\0.1 l \n"
                           "PN1000 \t= % 6d #\\0.1 l \n"
                           "\n";

void setup()
{
  Serial.begin(9600);

  Serial2.begin(9600, SERIAL_8N1, RX2, TX2);

  pms.setToPassiveReporting();
}

void loop()
{
  pms.poll();
  delay(20);
  pms.read();

  Serial.printf(outputFormat.c_str(),
                pms.getPM1_STD(),
                pms.getPM2_5_STD(),
                pms.getPM10_STD(),
                pms.getPM1_ATM(),
                pms.getPM2_5_ATM(),
                pms.getPM10_ATM(),
                pms.getCntBeyond300nm(),
                pms.getCntBeyond5000nm(),
                pms.getCntBeyond1000nm(),
                pms.getCntBeyond2500nm(),
                pms.getCntBeyond5000nm(),
                pms.getCntBeyond10000nm());
  delay(1000);
}
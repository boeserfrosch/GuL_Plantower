#include <PMS7003.h>

GuL::PMS7003 pms(&Serial1);

#define RX1 10
#define TX1 11

std::string outputFormat = "PM1 (STD) \t= % 3d µg/µ3 \n"
                           "PM2.5 (STD) \t= % 3d µg/µ3 \n"
                           "PM10 (STD) \t= % 3d µg/µ3 \n"
                           "PM1 (STD) \t= % 3d µg/µ3 \n"
                           "PM2.5 (STD) \t= % 3d µg/µ3 \n"
                           "PM10 (STD) \t= % 3d µg/µ3 \n"
                           "\n"
                           "PN300 (STD) \t= % 6d #\\0.1 l \n"
                           "PN500 (STD) \t= % 6d #\\0.1 l \n"
                           "PN1000 (STD) \t= % 6d #\\0.1 l \n"
                           "PN2500 (STD) \t= % 6d #\\0.1 l \n"
                           "PN5000 (STD) \t= % 6d #\\0.1 l \n"
                           "PN1000 (STD) \t= % 6d #\\0.1 l \n"
                           "\n";

void setup()
{
  Serial.begin(9600, SERIAL_8N1);

  Serial1.setPins(RX1, TX1);
  Serial1.begin(9600, SERIAL_8N1);

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

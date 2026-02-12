#include <PMS7003.h>

GuL::PMS7003 pms(Serial2);

#define RX2 1
#define TX2 38

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

  Serial.print("PM1 (STD) \t= ");
  Serial.print(pms.getPM1_STD());
  Serial.println(" \xC2\xB5g/\xC2\xB53");

  Serial.print("PM2.5 (STD) \t= ");
  Serial.print(pms.getPM2_5_STD());
  Serial.println(" \xC2\xB5g/\xC2\xB53");

  Serial.print("PM10 (STD) \t= ");
  Serial.print(pms.getPM10_STD());
  Serial.println(" \xC2\xB5g/\xC2\xB53");

  Serial.print("PM1 (ATM) \t= ");
  Serial.print(pms.getPM1_ATM());
  Serial.println(" \xC2\xB5g/\xC2\xB53");

  Serial.print("PM2.5 (ATM) \t= ");
  Serial.print(pms.getPM2_5_ATM());
  Serial.println(" \xC2\xB5g/\xC2\xB53");

  Serial.print("PM10 (ATM) \t= ");
  Serial.print(pms.getPM10_ATM());
  Serial.println(" \xC2\xB5g/\xC2\xB53");

  Serial.println();

  Serial.print("PN300 \t= ");
  Serial.print(pms.getCntBeyond300nm());
  Serial.println(" #\\0.1 l");

  Serial.print("PN500 \t= ");
  Serial.print(pms.getCntBeyond5000nm());
  Serial.println(" #\\0.1 l");

  Serial.print("PN1000 \t= ");
  Serial.print(pms.getCntBeyond1000nm());
  Serial.println(" #\\0.1 l");

  Serial.print("PN2500 \t= ");
  Serial.print(pms.getCntBeyond2500nm());
  Serial.println(" #\\0.1 l");

  Serial.print("PN5000 \t= ");
  Serial.print(pms.getCntBeyond5000nm());
  Serial.println(" #\\0.1 l");

  Serial.print("PN1000 \t= ");
  Serial.print(pms.getCntBeyond10000nm());
  Serial.println(" #\\0.1 l");

  Serial.println();
  delay(1000);
}
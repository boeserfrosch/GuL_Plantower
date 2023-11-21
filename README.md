# GuL Plantower
**GuL Plantower** is a library for the Arduino-Framework to work with the plantower particulate matter sensors like PMS7003



## Installation

1. [Arduino Library Manager (Recommended)](https://www.arduino.cc/en/Guide/Libraries)  
2. [Download or clone this repository into your arduino libraries directory](https://help.github.com/articles/cloning-a-repository/)  


## Usage
1. Include module  
   
   ```cpp
   #include <PMS7003.h> // Or one of the other moduls
   ```
   
2. Create a instance with the serial port it uses
```cpp
  //The modul uses the namespace GuL, instead of Serial1 you can use Serial, Serial2, or own instantiated HardwareSerial or SoftwareSerial
   GuL::PMS7003 pms(&Serial1); 
```
   
3. In `setup()`, basic setup of the modul.  
   
   ```cpp
   void setup() {
     pms.setToPassiveReporting();
     // Or
     // pms.setToActiveReporting();

     // For power saving reasons, not neccessary
     pms.sleep();
   }
   ```
   
4. In `loop()` of the sketch, run the object's **loop()** method.  
   
   ```cpp
   void loop() {
      // If in passive reporting mode
      pms.poll();
      // A bit delay, so the sensor can respond, not really neccessary
      delay(20);

      // Read the actual data
      pms.read();

      // Fetch the specific data channel, eg.
      int pm1_std = pms.getPM1_STD();
      int pn1000 = pms.getgetCntBeyond1000nm();
   }
   ```   



## APIs
### Constructors

   ```cpp
   PMS7003(Stream* stream);
   ```

### methods

  ```cpp
std::string getSensorName() { return _name; }
  ```
  Return the device name (Like PMS7003 or PMSA003)


  ```cpp
    bool poll();
  ```
  Polls the next dataframe from the sensor, the call is only neccessary if the sensor is in passive reporting mode

  ```cpp
    virtual bool read();
  ```
  Reads the incomming data frame, validates and unpack

  ```cpp
    bool sleep();
  ```
  Switch the sensor to sleep mode (Saves power consumption)

  ```cpp
    bool wakeup();
  ```
  Wake the sensor up (also wakes up if there is a poll() call)

  ```cpp
    bool setToActiveReporting();
  ```
  Like tha name says, set the sensor in active reporting mode, so each second a data frame will be send automatically

  ```cpp
    bool setToPassiveReporting();
  ```
  Set the sensor in passive reporting mode, each data frame have do be polled

  ```cpp
    int getPM1_STD();
  ```
  Get the last received PM1 concentration under the assumption of a standarized particle. Returns a negative number if there is no data

  ```cpp
    int getPM2_5_STD();
  ```
  Get the last received PM2.5 concentration under the assumption of a standarized particle. Returns a negative number if there is no data

  ```cpp
    int getPM10_STD();
  ```
  Get the last received PM10 concentration under the assumption of a standarized particle. Returns a negative number if there is no data

  ```cpp
    int getPM1_ATM();
  ```
  Get the last received PM1 concentration under the assumption of a athmospheric particle. Returns a negative number if there is no data


  ```cpp
    int getPM2_5_ATM();
  ```
  Get the last received PM2.5 concentration under the assumption of a athmospheric particle. Returns a negative number if there is no data

  ```cpp
    int getPM10_ATM();
  ```
  Get the last received PM10 concentration under the assumption of a athmospheric particle. Returns a negative number if there is no data

  ```cpp
    int getCntBeyond300nm();
  ```
  Get the last received number concentration per 0.1 l of particles greater than 300nm. Returns a negative number if there is no data

  ```cpp
    int getCntBeyond500nm();
  ```
  Get the last received number concentration per 0.1 l of particles greater than 500nm. Returns a negative number if there is no data

  ```cpp
    int getCntBeyond1000nm();
  ```
  Get the last received number concentration per 0.1 l of particles greater than 1000nm. Returns a negative number if there is no data

  ```cpp
    int getCntBeyond2500nm();
  ```
  Get the last received number concentration per 0.1 l of particles greater than 2500nm. Returns a negative number if there is no data

  ```cpp
    int getCntBeyond5000nm();
  ```
  Get the last received number concentration per 0.1 l of particles greater than 5000nm. Returns a negative number if there is no data

  ```cpp
    int getCntBeyond10000nm();
  ```
  Get the last received number concentration per 0.1 l of particles greater than 10.000nm. Returns a negative number if there is no data


  ```cpp
    int getVersion();
  ```
  Get the last received sensor firmware version. Returns a negative number if there is no data. (Only newer devices return a version)

  ```cpp
    int getError();
  ```
  Get the last received error code. Returns a negative number if there is no data. (Only newer devices return a error code)

  ```cpp
    float getFormaldehydeConcentration();
  ```
  Get the last received formaldehyde concentration in mg/m³. Returns a negative number if there is no data. (At the moment just the PMS5003ST returns this data)

  ```cpp
    float getTemperature();
  ```
  Get the last received temperature in °C. Returns a negative number if there is no data. (At the moment just the PMS5003ST returns this data)

  ```cpp
    float getHumidity();
  ```
  Get the last received humidity in %. Returns a negative number if there is no data. (At the moment just the PMS5003ST returns this data)


## What's Next

- Implement more plantower sensors, especially PMSX003-N or PMSA003I (The PMSA003I use I2C, instad of uart)
- Write tests

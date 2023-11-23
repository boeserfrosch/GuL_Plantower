/**
 * GuL_Plantower
 * Copyright (c) 2023 Guido Lehne
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Developed for Arduino-ESP32
 * Created by Guido Lehne
 *
 */

#include <Arduino.h>
#include <vector>

#ifndef GUL_PLANTOWER_PLANTOWER_H
#define GUL_PLANTOWER_PLANTOWER_H

namespace GuL
{
  enum Plantower_Working_Mode
  {
    SLEEP,
    WAKEUP
  };

  enum Plantower_Reporting_Mode
  {
    ACTIVE,
    PASSIVE
  };

  enum Plantower_Parsing_Steps
  {
    WAIT_FOR_NEW_FRAME,
    CHECK_FRAME_HEADER,
    READ_FIRST_LEN_BYTE,
    READ_SECOND_LEN_BYTE,
    READ_PAYLOAD
  };

  class Plantower
  {
  public:
    Plantower(Stream &stream);
    std::string getSensorName() { return _name; }

    bool poll();
    virtual bool read();

    bool sleep();
    bool wakeup();
    bool setToActiveReporting();
    bool setToPassiveReporting();

    int getPM1_STD();
    int getPM2_5_STD();
    int getPM10_STD();
    int getPM1_ATM();
    int getPM2_5_ATM();
    int getPM10_ATM();

    int getCntBeyond300nm();
    int getCntBeyond500nm();
    int getCntBeyond1000nm();
    int getCntBeyond2500nm();
    int getCntBeyond5000nm();
    int getCntBeyond10000nm();

    int getVersion();
    int getError();

    float getFormaldehydeConcentration();
    float getTemperature();
    float getHumidity();

  private:
    Stream &_stream;
    Plantower_Parsing_Steps _parseStep = WAIT_FOR_NEW_FRAME;

    uint16_t _payloadIndex = 0;
    uint16_t _payloadStartIndex = 4;
    bool _gotValidFrame = false;

    bool expectedFrameLength(size_t len);
    void initPayloadBuffer();
    bool checkFrameChecksum();

  protected:
    enum PLANTOWER_DATA_IDX
    {
      PM1_STD_IDX = 0,
      PM2_5_STD_IDX,
      PM10_STD_IDX,
      PM1_ATM_IDX,
      PM2_5_ATM_IDX,
      PM10_ATM_IDX,
      CNT_BEYOND_300_NM_IDX,
      CNT_BEYOND_500_NM_IDX,
      CNT_BEYOND_1000_NM_IDX,
      CNT_BEYOND_2500_NM_IDX,
      CNT_BEYOND_5000_NM_IDX,
      CNT_BEYOND_10000_NM_IDX,
      VERSION_IDX,
      ERROR_IDX,
      FORMALDEHYDE_CONCENTRATION_IDX,
      TEMPERATURE_IDX,
      HUMIDITY_IDX,
      CNT_OF_CHANNELS
    };
    std::string _name = "PLANTOWER";
    std::vector<uint8_t> _payloadBuffer;
    size_t _bufferLength;
    uint16_t _frameLength = 0;
    int _activeFrameLength = -1;
    int32_t *_data = nullptr;
    Plantower_Working_Mode _workMode = Plantower_Working_Mode::WAKEUP;
    Plantower_Reporting_Mode _reportingMode = Plantower_Reporting_Mode::ACTIVE;

    uint16_t calcChecksum(std::vector<uint8_t> cmd, size_t cnt);
    virtual bool sendFrame(std::vector<uint8_t> cmd);
    virtual void unpackPayload();
    void unpackBasePMData();
    void unpackBasePNData();
  };
}; // namespace GuL

#endif // !GUL_PLANTOWER_PLANTOWER_H
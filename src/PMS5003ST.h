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

#include "Plantower.h"

#ifndef GUL_PLANTOWER_PMS5003ST_H
#define GUL_PLANTOWER_PMS5003ST_H

namespace GuL
{

  class PMS5003ST : public Plantower
  {
  private:
  protected:
    void unpackActiveProtocolPayload()
    {
      unpackBasePMData();
      unpackBasePNData();
      _data[FORMALDEHYDE_CONCENTRATION_IDX] = _payloadBuffer[28] << 8 | _payloadBuffer[29];
      _data[TEMPERATURE_IDX] = _payloadBuffer[30] << 8 | _payloadBuffer[31];
      _data[HUMIDITY_IDX] = _payloadBuffer[32] << 8 | _payloadBuffer[33];
      _data[VERSION_IDX] = _payloadBuffer[36];
      _data[ERROR_IDX] = _payloadBuffer[37];
    }

  public:
    PMS5003ST(Stream &stream) : Plantower(stream)
    {
      _name = "PMS5003ST";
      _activeFrameLength = 36;
    }
  };

}; // namespace GuL

#endif // !IDS_REAL_DEVICE_PMS5003ST_H
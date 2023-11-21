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

#ifndef GUL_PLANTOWER_PMSA003_H
#define GUL_PLANTOWER_PMSA003_H

namespace GuL
{

  class PMSA003 : public Plantower
  {
  private:
  protected:
  public:
    PMSA003(Stream *stream) : Plantower(stream)
    {
      _name = "PMSA003";
      _activeFrameLength = 28;
    }
  };

}; // namespace GuL

#endif // !IDS_REAL_DEVICE_PMSA003_H
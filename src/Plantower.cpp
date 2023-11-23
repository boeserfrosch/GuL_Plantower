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

namespace GuL
{

    Plantower::Plantower(Stream &stream) : _stream(stream)
    {
        _data = new int32_t[CNT_OF_CHANNELS];
        for (size_t i = 0; i < CNT_OF_CHANNELS; i++)
        {
            _data[i] = -1;
        }
    }

    bool Plantower::poll()
    {
        std::vector<uint8_t> command = {0x42, 0x4D, 0xE2, 0x00, 0x00, 0xFF, 0xFF};
        return this->sendFrame(command);
    }

    bool Plantower::sleep()
    {
        std::vector<uint8_t> command = {0x42, 0x4D, 0xE4, 0x00, 0x00, 0xFF, 0xFF};
        _workMode = Plantower_Working_Mode::SLEEP;
        return this->sendFrame(command);
    }

    bool Plantower::wakeup()
    {
        std::vector<uint8_t> command = {0x42, 0x4D, 0xE4, 0x00, 0x01, 0xFF, 0xFF};
        _workMode = Plantower_Working_Mode::WAKEUP;
        return this->sendFrame(command);
    }

    bool Plantower::setToActiveReporting()
    {
        std::vector<uint8_t> command = {0x42, 0x4D, 0xE1, 0x00, 0x01, 0xFF, 0xFF};
        _reportingMode = Plantower_Reporting_Mode::ACTIVE;
        return this->sendFrame(command);
    }

    bool Plantower::setToPassiveReporting()
    {
        std::vector<uint8_t> command = {0x42, 0x4D, 0xE1, 0x00, 0x00, 0xFF, 0xFF};
        _reportingMode = Plantower_Reporting_Mode::PASSIVE;
        return this->sendFrame(command);
    }

    bool Plantower::read()
    {

        if (_stream.available() == 0)
        {
            return false; // No data available
        }

        while (_stream.available())
        {
            int byte = _stream.read();
            switch (_parseStep)
            {
            case WAIT_FOR_NEW_FRAME:
                if (byte == 0x42)
                {
                    _payloadIndex = _payloadStartIndex;
                    _parseStep = CHECK_FRAME_HEADER;
                }
                break;
            case CHECK_FRAME_HEADER:
                if (byte != 0x4D)
                {
                    _parseStep = WAIT_FOR_NEW_FRAME;
                    break;
                }
                _parseStep = READ_FIRST_LEN_BYTE;
                break;
            case READ_FIRST_LEN_BYTE:
                _frameLength = byte << 8;
                _parseStep = READ_SECOND_LEN_BYTE;
                break;
            case READ_SECOND_LEN_BYTE:
                _frameLength |= byte;

                if (!this->expectedFrameLength(_frameLength))
                {
                    _parseStep = WAIT_FOR_NEW_FRAME;
                    break;
                }
                this->initPayloadBuffer();
                _parseStep = READ_PAYLOAD;
                break;
            case READ_PAYLOAD:
                _payloadBuffer[_payloadIndex] = byte;
                _payloadIndex++;
                if (_payloadIndex == _frameLength + _payloadStartIndex)
                {
                    if (this->checkFrameChecksum())
                    {
                        this->unpackPayload();
                        _gotValidFrame = true;
                    }
                    _parseStep = WAIT_FOR_NEW_FRAME;
                }
                break;
            }
        }

        if (_gotValidFrame)
        {
            _gotValidFrame = false;
            return true;
        }
        return false;
    }

    uint16_t Plantower::calcChecksum(std::vector<uint8_t> cmd, size_t cnt)
    {
        uint16_t sum = 0;
        for (size_t i = 0; i < cnt; i++)
        {
            sum += cmd[i];
        }
        return sum;
    }

    bool Plantower::sendFrame(std::vector<uint8_t> cmd)
    {

        uint16_t checksum = this->calcChecksum(cmd, cmd.size() - 2);
        cmd[cmd.size() - 2] = (checksum >> 8) & 0xFF;
        cmd[cmd.size() - 1] = checksum & 0xFF;

        size_t bytesSend = _stream.write(cmd.data(), cmd.size());
        return bytesSend == cmd.size();
    }

    bool Plantower::expectedFrameLength(size_t framelength)
    {
        if (_activeFrameLength < 0) // The expected frame length was not defined, so atleast check if any of common len
        {
            // PMS3003 send frames with length 2 * 9 + 2      \
            // PMS5003, PMS7003, PMSA003 sends frames with \
            // length 2*13+2
            // For newer hardware there can be another frame with a length of 4
            return framelength == 20 | framelength == 28 | framelength == 4;
        }
        return framelength == _activeFrameLength;
    }

    void Plantower::initPayloadBuffer()
    {
        if (_payloadBuffer.size() != _frameLength + 4)
        {
            _payloadBuffer.clear();
            _payloadBuffer.resize(_frameLength + 4); // The 4 is because of the header and the length
        }
        _payloadBuffer[0] = 0x42;
        _payloadBuffer[1] = 0x4d;
        _payloadBuffer[2] = _frameLength >> 8;
        _payloadBuffer[3] = _frameLength & 0xFF;
        _payloadStartIndex = 4;
        _payloadIndex = _payloadStartIndex;
    }

    bool Plantower::checkFrameChecksum()
    {
        uint16_t calcCS = this->calcChecksum(_payloadBuffer, _payloadBuffer.size() - 2);
        uint16_t recvCS = (_payloadBuffer[_payloadBuffer.size() - 2] << 8) | _payloadBuffer[_payloadBuffer.size() - 1];

        return recvCS == calcCS;
    }

    void Plantower::unpackPayload()
    {
        unpackBasePMData();
        if (_frameLength >= 28)
        {
            unpackBasePNData();
        }
    }

    void Plantower::unpackBasePMData()
    {
        _data[PM1_STD_IDX] = _payloadBuffer[4] << 8 | _payloadBuffer[5];
        _data[PM2_5_STD_IDX] = _payloadBuffer[6] << 8 | _payloadBuffer[7];
        _data[PM10_STD_IDX] = _payloadBuffer[8] << 8 | _payloadBuffer[9];
        _data[PM1_ATM_IDX] = _payloadBuffer[10] << 8 | _payloadBuffer[11];
        _data[PM2_5_ATM_IDX] = _payloadBuffer[12] << 8 | _payloadBuffer[13];
        _data[PM10_ATM_IDX] = _payloadBuffer[14] << 8 | _payloadBuffer[15];
    }

    void Plantower::unpackBasePNData()
    {
        _data[CNT_BEYOND_300_NM_IDX] = _payloadBuffer[16] << 8 | _payloadBuffer[17];
        _data[CNT_BEYOND_500_NM_IDX] = _payloadBuffer[18] << 8 | _payloadBuffer[19];
        _data[CNT_BEYOND_1000_NM_IDX] = _payloadBuffer[20] << 8 | _payloadBuffer[21];
        _data[CNT_BEYOND_2500_NM_IDX] = _payloadBuffer[22] << 8 | _payloadBuffer[23];
        _data[CNT_BEYOND_5000_NM_IDX] = _payloadBuffer[24] << 8 | _payloadBuffer[25];
        _data[CNT_BEYOND_10000_NM_IDX] = _payloadBuffer[26] << 8 | _payloadBuffer[27];
    }

    int Plantower::getPM1_STD() { return _data[PM1_STD_IDX]; }
    int Plantower::getPM2_5_STD() { return _data[PM2_5_STD_IDX]; }
    int Plantower::getPM10_STD() { return _data[PM10_STD_IDX]; }
    int Plantower::getPM1_ATM() { return _data[PM1_ATM_IDX]; }
    int Plantower::getPM2_5_ATM() { return _data[PM2_5_ATM_IDX]; }
    int Plantower::getPM10_ATM() { return _data[PM10_ATM_IDX]; }

    int Plantower::getCntBeyond300nm() { return _data[CNT_BEYOND_300_NM_IDX]; }
    int Plantower::getCntBeyond500nm() { return _data[CNT_BEYOND_500_NM_IDX]; }
    int Plantower::getCntBeyond1000nm() { return _data[CNT_BEYOND_1000_NM_IDX]; }
    int Plantower::getCntBeyond2500nm() { return _data[CNT_BEYOND_2500_NM_IDX]; }
    int Plantower::getCntBeyond5000nm() { return _data[CNT_BEYOND_500_NM_IDX]; }
    int Plantower::getCntBeyond10000nm() { return _data[CNT_BEYOND_10000_NM_IDX]; }

    int Plantower::getVersion() { return _data[VERSION_IDX]; }
    int Plantower::getError() { return _data[ERROR_IDX]; }

    float Plantower::getFormaldehydeConcentration() { return (float)_data[FORMALDEHYDE_CONCENTRATION_IDX] / 1000; };
    float Plantower::getTemperature() { return (float)_data[TEMPERATURE_IDX] / 10; };
    float Plantower::getHumidity() { return (float)_data[HUMIDITY_IDX] / 10; };

}; // namespace GuL

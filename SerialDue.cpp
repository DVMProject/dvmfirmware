/**
* Digital Voice Modem - DSP Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / DSP Firmware
*
*/
//
// Based on code from the MMDVM project. (https://github.com/g4klx/MMDVM)
// Licensed under the GPLv2 License (https://opensource.org/licenses/GPL-2.0)
//
/*
*   Copyright (C) 2016,2017 by Jonathan Naylor G4KLX
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
#include "Globals.h"
#include "SerialPort.h"

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
#if (defined(__SAM3X8E__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)) && defined(ARDUINO_SAM_DUE)
/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <param name="speed"></param>
void SerialPort::beginInt(uint8_t n, int speed)
{
    switch (n) {
    case 1U:
        Serial.begin(speed);
        break;
    case 2U:
        Serial2.begin(speed);
        break;
    case 3U:
        Serial3.begin(speed);
        break;
    default:
        break;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
int SerialPort::availableInt(uint8_t n)
{
    switch (n) {
    case 1U:
        return Serial.available();
    case 2U:
        return Serial2.available();
    case 3U:
        return Serial3.available();
    default:
        return false;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
int SerialPort::availableForWriteInt(uint8_t n)
{
    switch (n) {
    case 1U:
        return Serial.availableForWrite();
    case 2U:
        return Serial2.availableForWrite();
    case 3U:
        return Serial3.availableForWrite();
    default:
        return false;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
uint8_t SerialPort::readInt(uint8_t n)
{
    switch (n) {
    case 1U:
        return Serial.read();
    case 2U:
        return Serial2.read();
    case 3U:
        return Serial3.read();
    default:
        return 0U;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <param name="data"></param>
/// <param name="length"></param>
/// <param name="flush"></param>
void SerialPort::writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush)
{
    switch (n) {
    case 1U:
        Serial.write(data, length);
        if (flush)
            Serial.flush();
        break;
    case 2U:
        Serial2.write(data, length);
        if (flush)
            Serial2.flush();
        break;
    case 3U:
        Serial3.write(data, length);
        if (flush)
            Serial3.flush();
        break;
    default:
        break;
    }
}
#endif

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
*   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
*   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
*   Copyright (c) 2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2018,2022 Bryan Biedenkapp N2PLL
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

#if defined(NATIVE_SDR) || true

#include "sdr/port/UARTPort.h"
#include "sdr/port/PseudoPTYPort.h"

using namespace sdr::port;

// ---------------------------------------------------------------------------
//  Globals Variables
// ---------------------------------------------------------------------------

PseudoPTYPort* m_serialPort = nullptr;
static uint8_t m_readBuffer = 0x00U;

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/// <summary>
///
/// </summary>
void SerialPort::flashRead()
{
    DEBUG1("SerialPort: flashRead(): unsupported on Native SDR");
    sendNAK(RSN_NO_INTERNAL_FLASH);
    // unused on SDR based dedicated modems
}

/// <summary>
///
/// </summary>
/// <param name="data"></param>
/// <param name="length"></param>
uint8_t SerialPort::flashWrite(const uint8_t* data, uint8_t length)
{
    DEBUG1("SerialPort: flashWrite(): unsupported on Native SDR");
    // unused on Arduino Due based dedicated modems
    return RSN_NO_INTERNAL_FLASH;
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <param name="speed"></param>
void SerialPort::beginInt(uint8_t n, int speed)
{
    switch (n) {
    case 1U:
        m_readBuffer = 0x00U;
        m_serialPort = new PseudoPTYPort(m_ptyPort, SERIAL_115200, false);
        m_serialPort->open();
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
        return m_serialPort->read(&m_readBuffer, (uint8_t)(1 * sizeof(uint8_t)));
    default:
        return 0;
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
        return true;
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
        return m_readBuffer;  
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
        m_serialPort->write(data, length);
        break;
    default:
        break;
    }
}

#endif // defined(NATIVE_SDR)

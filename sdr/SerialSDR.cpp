// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2025 Bryan Biedenkapp N2PLL
 *
 */
#include "Globals.h"
#include "SerialPort.h"

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

/* Reads data from the modem flash parititon. */

void SerialPort::flashRead()
{
    DEBUG1("SerialPort: flashRead(): unsupported on Native SDR");
    sendNAK(RSN_NO_INTERNAL_FLASH);
    // unused on SDR based dedicated modems
}

/* Writes data to the modem flash partition. */

uint8_t SerialPort::flashWrite(const uint8_t* data, uint8_t length)
{
    DEBUG1("SerialPort: flashWrite(): unsupported on Native SDR");
    // unused on Arduino Due based dedicated modems
    return RSN_NO_INTERNAL_FLASH;
}

/* */

void SerialPort::beginInt(uint8_t n, int speed)
{
    ::LogMessage("Starting PTY serial...");

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

/* */

int SerialPort::availableInt(uint8_t n)
{
    switch (n) {
    case 1U:
        return m_serialPort->read(&m_readBuffer, (uint8_t)(1 * sizeof(uint8_t)));
    default:
        return 0;
    }
}

/* */

int SerialPort::availableForWriteInt(uint8_t n)
{
    switch (n) {
    case 1U:
        return true;
    default:
        return false;
    }
}

/* */

uint8_t SerialPort::readInt(uint8_t n)
{
    switch (n) {
    case 1U:
        return m_readBuffer;
    default:
        return 0U;
    }
}

/* */

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

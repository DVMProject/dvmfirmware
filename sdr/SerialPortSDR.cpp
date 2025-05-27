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
#include "sdr/SerialPortSDR.h"
#include "sdr/Log.h"
#include "sdr/Utils.h"

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the SerialPortSDR class. */

SerialPortSDR::SerialPortSDR() :
    SerialPort()
{
    // stub
}

/* */

void SerialPortSDR::writeDebug(const char* text)
{
    SerialPort::writeDebug(text);
    if (g_debug)
        ::LogDebug("DSP_FW_API %s", text);
}

/* */

void SerialPortSDR::writeDebug(const char* text, int16_t n1)
{
    SerialPort::writeDebug(text, n1);
    if (g_debug)
        ::LogDebug("DSP_FW_API %s %X", text, n1);
}

/* */

void SerialPortSDR::writeDebug(const char* text, int16_t n1, int16_t n2)
{
    SerialPort::writeDebug(text, n1, n2);
    if (g_debug)
        ::LogDebug("DSP_FW_API %s %X %X", text, n1, n2);
}

/* */

void SerialPortSDR::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3)
{
    SerialPort::writeDebug(text, n1, n2, n3);
    if (g_debug)
        ::LogDebug("DSP_FW_API %s %X %X %X", text, n1, n2, n3);
}

/* */

void SerialPortSDR::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4)
{
    SerialPort::writeDebug(text, n1, n2, n3, n4);
    if (g_debug)
        ::LogDebug("DSP_FW_API %s %X %X %X %X", text, n1, n2, n3, n4);
}

/* */

void SerialPortSDR::writeDump(const uint8_t* data, uint16_t length)
{
    SerialPort::writeDump(data, length);
    if (g_debug)
        Utils::dump(1U, "DSP_FW_API Dump", data, length);
}

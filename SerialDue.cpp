// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2018,2022 Bryan Biedenkapp, N2PLL
 *
 */
#include "Globals.h"
#include "SerialPort.h"

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

#if (defined(__SAM3X8E__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)) && defined(ARDUINO_SAM_DUE)
/* Reads data from the modem flash parititon. */

void SerialPort::flashRead()
{
    DEBUG1("SerialPort: flashRead(): unsupported on Arduino Due");
    sendNAK(RSN_NO_INTERNAL_FLASH);
    // unused on Arduino Due based dedicated modems
}

/* Writes data to the modem flash partition. */

uint8_t SerialPort::flashWrite(const uint8_t* data, uint8_t length)
{
    DEBUG1("SerialPort: flashWrite(): unsupported on Arduino Due");
    // unused on Arduino Due based dedicated modems
    return RSN_NO_INTERNAL_FLASH;
}

/* */

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

/* */

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

/* */

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

/* */

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

/* */

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

#endif // (defined(__SAM3X8E__) || defined(__MK20DX256__) || defined(__MK64FX512__) || defined(__MK66FX1M0__)) && defined(ARDUINO_SAM_DUE)

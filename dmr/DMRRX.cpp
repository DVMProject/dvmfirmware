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
*   Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
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
#include "dmr/DMRRX.h"

using namespace dmr;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the DMRRX class.
/// </summary>
DMRRX::DMRRX() :
    m_slot1RX(false),
    m_slot2RX(true)
{
    /* stub */
}

/// <summary>
/// Helper to reset data values to defaults.
/// </summary>
void DMRRX::reset()
{
    m_slot1RX.reset();
    m_slot2RX.reset();
}

/// <summary>
/// Sample DMR values from the air interface.
/// </summary>
/// <param name="samples"></param>
/// <param name="rssi"></param>
/// <param name="control"></param>
/// <param name="length"></param>
void DMRRX::samples(const q15_t* samples, const uint16_t* rssi, const uint8_t* control, uint8_t length)
{
    bool dcd1 = false;
    bool dcd2 = false;

    for (uint16_t i = 0U; i < length; i++) {
        switch (control[i]) {
        case MARK_SLOT1:
            m_slot1RX.start();
            break;
        case MARK_SLOT2:
            m_slot2RX.start();
            break;
        default:
            break;
        }

        dcd1 = m_slot1RX.processSample(samples[i], rssi[i]);
        dcd2 = m_slot2RX.processSample(samples[i], rssi[i]);
    }

    io.setDecode(dcd1 || dcd2);
}

/// <summary>
/// Sets the DMR color code.
/// </summary>
/// <param name="colorCode">Color code.</param>
void DMRRX::setColorCode(uint8_t colorCode)
{
    m_slot1RX.setColorCode(colorCode);
    m_slot2RX.setColorCode(colorCode);
}

/// <summary>
/// Sets the number of samples to delay before processing.
/// </summary>
/// <param name="delay">Number of samples to delay.</param>
void DMRRX::setRxDelay(uint8_t delay)
{
    m_slot1RX.setRxDelay(delay);
    m_slot2RX.setRxDelay(delay);
}

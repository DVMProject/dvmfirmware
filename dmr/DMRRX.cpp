// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
 *
 */
#include "Globals.h"
#include "dmr/DMRRX.h"

using namespace dmr;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the DMRRX class. */

DMRRX::DMRRX() :
    m_slot1RX(false),
    m_slot2RX(true)
{
    /* stub */
}

/* Helper to reset data values to defaults. */

void DMRRX::reset()
{
    m_slot1RX.reset();
    m_slot2RX.reset();
}

/* Sample DMR values from the air interface. */

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

/* Sets the DMR color code. */

void DMRRX::setColorCode(uint8_t colorCode)
{
    m_slot1RX.setColorCode(colorCode);
    m_slot2RX.setColorCode(colorCode);
}

/* Sets the number of samples to delay before processing. */

void DMRRX::setRxDelay(uint8_t delay)
{
    m_slot1RX.setRxDelay(delay);
    m_slot2RX.setRxDelay(delay);
}

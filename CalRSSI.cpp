// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2016 Jonathan Naylor, G4KLX
 *
 */
#include "Globals.h"
#include "CalRSSI.h"
#include "Utils.h"

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the CalRSSI class. */

CalRSSI::CalRSSI() :
    m_count(0U),
    m_accum(0U),
    m_min(0xFFFFU),
    m_max(0x0000U)
{
    /* stub */
}

/* Sample RSSI values from the air interface. */

void CalRSSI::samples(const uint16_t* rssi, uint8_t length)
{
    for (uint16_t i = 0U; i < length; i++) {
        uint16_t ss = rssi[i];

        m_accum += ss;

        if (ss > m_max)
            m_max = ss;
        if (ss < m_min)
            m_min = ss;

        m_count++;
        if (m_count >= 24000U) {
            uint16_t ave = m_accum / m_count;

            uint8_t buffer[6U];
            buffer[0U] = (m_max >> 8) & 0xFFU;
            buffer[1U] = (m_max >> 0) & 0xFFU;
            buffer[2U] = (m_min >> 8) & 0xFFU;
            buffer[3U] = (m_min >> 0) & 0xFFU;
            buffer[4U] = (ave >> 8) & 0xFFU;
            buffer[5U] = (ave >> 0) & 0xFFU;

            serial.writeRSSIData(buffer, 6U);

            m_count = 0U;
            m_accum = 0U;
            m_min = 0xFFFFU;
            m_max = 0x0000U;
        }
    }
}

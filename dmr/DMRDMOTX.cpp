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
*   Copyright (C) 2009-2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2016 by Colin Durbridge G4EML
*   Copyright (C) 2017 by Andy Uribe CA6JAU
*   Copyright (C) 2020 by Bryan Biedenkapp N2PLL
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
#include "dmr/DMRSlotType.h"

using namespace dmr;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = {
    0, 0, 0, 0, 850, 219, -720, -1548, -1795, -1172, 237, 1927, 3120, 3073, 1447, -1431, -4544, -6442,
    -5735, -1633, 5651, 14822, 23810, 30367, 32767, 30367, 23810, 14822, 5651, -1633, -5735, -6442,
    -4544, -1431, 1447, 3073, 3120, 1927, 237, -1172, -1795, -1548, -720, 219, 850
}; // numTaps = 45, L = 5
const uint16_t RRC_0_2_FILTER_PHASE_LEN = 9U; // phaseLength = numTaps/L

const q15_t DMR_LEVELA = 1362;
const q15_t DMR_LEVELB = 454;
const q15_t DMR_LEVELC = -454;
const q15_t DMR_LEVELD = -1362;

const uint8_t DMR_SYNC = 0x5FU;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the DMRDMOTX class.
/// </summary>
DMRDMOTX::DMRDMOTX() :
    m_fifo(),
    m_modFilter(),
    m_modState(),
    m_poBuffer(),
    m_poLen(0U),
    m_poPtr(0U),
    m_preambleCnt(DMRDMO_FIXED_DELAY)
{
    ::memset(m_modState, 0x00U, 16U * sizeof(q15_t));

    m_modFilter.L = DMR_RADIO_SYMBOL_LENGTH;
    m_modFilter.phaseLength = RRC_0_2_FILTER_PHASE_LEN;
    m_modFilter.pCoeffs = RRC_0_2_FILTER;
    m_modFilter.pState = m_modState;
}

/// <summary>
/// Process local buffer and transmit on the air interface.
/// </summary>
void DMRDMOTX::process()
{
    if (m_poLen == 0U && m_fifo.getData() > 0U) {
        if (!m_tx) {
            for (uint16_t i = 0U; i < m_preambleCnt; i++)
                m_poBuffer[i] = DMR_SYNC;

            m_poLen = m_preambleCnt;
        }
        else {
            for (unsigned int i = 0U; i < 72U; i++)
                m_poBuffer[i] = DMR_SYNC;

            for (unsigned int i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
                m_poBuffer[i + 39U] = m_fifo.get();

            m_poLen = 72U;
        }

        m_poPtr = 0U;
    }

    if (m_poLen > 0U) {
        uint16_t space = io.getSpace();

        while (space > (4U * DMR_RADIO_SYMBOL_LENGTH)) {
            uint8_t c = m_poBuffer[m_poPtr++];

            writeByte(c);

            space -= 4U * DMR_RADIO_SYMBOL_LENGTH;

            if (m_poPtr >= m_poLen) {
                m_poPtr = 0U;
                m_poLen = 0U;
                return;
            }
        }
    }
}

/// <summary>
/// Write data to the local buffer.
/// </summary>
/// <param name="data"></param>
/// <param name="length"></param>
/// <returns></returns>
uint8_t DMRDMOTX::writeData(const uint8_t* data, uint8_t length)
{
    if (length != (DMR_FRAME_LENGTH_BYTES + 1U))
        return RSN_ILLEGAL_LENGTH;

    uint16_t space = m_fifo.getSpace();
    if (space < DMR_FRAME_LENGTH_BYTES)
        return RSN_RINGBUFF_FULL;

    for (uint8_t i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
        m_fifo.put(data[i + 1U]);

    return RSN_OK;
}

/// <summary>
///
/// </summary>
/// <param name="preambleCnt"></param>
void DMRDMOTX::setPreambleCount(uint8_t preambleCnt)
{
    uint32_t preambles = (uint32_t)((float)preambleCnt / 0.2083F);
    m_preambleCnt = DMRDMO_FIXED_DELAY + preambles;

    // clamp preamble count to 250ms maximum
    if (m_preambleCnt > 1200U)
        m_preambleCnt = 1200U;
}

/// <summary>
/// Helper to get how much space the ring buffer has for samples.
/// </summary>
/// <returns></returns>
uint8_t DMRDMOTX::getSpace() const
{
    return m_fifo.getSpace() / (DMR_FRAME_LENGTH_BYTES + 2U);
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
/// <summary>
///
/// </summary>
/// <param name="c"></param>
void DMRDMOTX::writeByte(uint8_t c)
{
    q15_t inBuffer[4U];
    q15_t outBuffer[DMR_RADIO_SYMBOL_LENGTH * 4U];

    const uint8_t MASK = 0xC0U;

    for (uint8_t i = 0U; i < 4U; i++, c <<= 2) {
        switch (c & MASK) {
        case 0xC0U:
            inBuffer[i] = DMR_LEVELA;
            break;
        case 0x80U:
            inBuffer[i] = DMR_LEVELB;
            break;
        case 0x00U:
            inBuffer[i] = DMR_LEVELC;
            break;
        default:
            inBuffer[i] = DMR_LEVELD;
            break;
        }
    }

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, 4U);

    io.write(STATE_DMR, outBuffer, DMR_RADIO_SYMBOL_LENGTH * 4U);
}

void DMRDMOTX::writeSilence()
{
    q15_t inBuffer[4U] = { 0x00U, 0x00U, 0x00U, 0x00U };
    q15_t outBuffer[DMR_RADIO_SYMBOL_LENGTH * 4U];

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, 4U);

    io.write(STATE_DMR, outBuffer, DMR_RADIO_SYMBOL_LENGTH * 4U);
}

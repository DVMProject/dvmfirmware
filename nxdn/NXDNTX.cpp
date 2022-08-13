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
 *   Copyright (C) 2009-2018,2020 by Jonathan Naylor G4KLX
 *   Copyright (C) 2017 by Andy Uribe CA6JAU
 *   Copyright (C) 2022 by Bryan Biedenkapp N2PLL
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
#include "nxdn/NXDNTX.h"
#include "nxdn/NXDNDefines.h"

using namespace nxdn;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#if defined(NXDN_9600_BAUD)
// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = { 
    0, 0, 0, 0, 850, 219, -720, -1548, -1795, -1172, 237, 1927, 3120, 3073, 1447, -1431, -4544, -6442,
    -5735, -1633, 5651, 14822, 23810, 30367, 32767, 30367, 23810, 14822, 5651, -1633, -5735, -6442,
    -4544, -1431, 1447, 3073, 3120, 1927, 237, -1172, -1795, -1548, -720, 219, 850 
}; // numTaps = 45, L = 5
#else
// Generated using rcosdesign(0.2, 8, 10, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = { 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 850, 592, 219, -234, -720, -1179, -1548, -1769, -1795, -1597, -1172,
    -544, 237, 1092, 1927, 2637, 3120, 3286, 3073, 2454, 1447, 116, -1431, -3043, -4544, -5739, -6442,
    -6483, -5735, -4121, -1633, 1669, 5651, 10118, 14822, 19484, 23810, 27520, 30367, 32156, 32767,
    32156, 30367, 27520, 23810, 19484, 14822, 10118, 5651, 1669, -1633, -4121, -5735, -6483, -6442,
    -5739, -4544, -3043, -1431, 116, 1447, 2454, 3073, 3286, 3120, 2637, 1927, 1092, 237, -544, -1172,
    -1597, -1795, -1769, -1548, -1179, -720, -234, 219, 592, 850
}; // numTaps = 90, L = 10
#endif
const uint16_t RRC_0_2_FILTER_PHASE_LEN = 9U; // phaseLength = numTaps/L

static q15_t NXDN_SINC_FILTER[] = { 
    572, -1003, -253, 254, 740, 1290, 1902, 2527, 3090, 3517, 3747, 3747, 3517, 3090, 2527, 1902,
    1290, 740, 254, -253, -1003, 572
};
const uint16_t NXDN_SINC_FILTER_LEN = 22U;

#if defined(NXDN_9600_BAUD)
const q15_t NXDN_LEVELA =  1680;
const q15_t NXDN_LEVELB =  560;
const q15_t NXDN_LEVELC = -560;
const q15_t NXDN_LEVELD = -1680;
#else
const q15_t NXDN_LEVELA =  735;
const q15_t NXDN_LEVELB =  245;
const q15_t NXDN_LEVELC = -245;
const q15_t NXDN_LEVELD = -735;
#endif

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the NXDNTX class.
/// </summary>
NXDNTX::NXDNTX() :
    m_fifo(NXDN_TX_BUFFER_LEN),
    m_state(NXDNTXSTATE_NORMAL),
    m_modFilter(),
    m_sincFilter(),
    m_modState(),
    m_sincState(),
    m_poBuffer(),
    m_poLen(0U),
    m_poPtr(0U),
    m_preambleCnt(240U), // 200ms
    m_txHang(3000U),     // 5s
    m_tailCnt(0U),
    m_symLevel3Adj(0U),
    m_symLevel1Adj(0U)
{
    ::memset(m_modState, 0x00U, 16U * sizeof(q15_t));
    ::memset(m_sincState,  0x00U, 70U * sizeof(q15_t));

    m_modFilter.L = NXDN_RADIO_SYMBOL_LENGTH;
    m_modFilter.phaseLength = RRC_0_2_FILTER_PHASE_LEN;
    m_modFilter.pCoeffs = RRC_0_2_FILTER;
    m_modFilter.pState = m_modState;

    m_sincFilter.numTaps = NXDN_SINC_FILTER_LEN;
    m_sincFilter.pState = m_sincState;
    m_sincFilter.pCoeffs = NXDN_SINC_FILTER;
}

/// <summary>
/// Process local buffer and transmit on the air interface.
/// </summary>
void NXDNTX::process()
{
    if (m_fifo.getData() == 0U && m_poLen == 0U && m_tailCnt > 0U &&
        m_state != NXDNTXSTATE_CAL) {
        // transmit silence until the hang timer has expired
        uint16_t space = io.getSpace();

        while (space > (4U * NXDN_RADIO_SYMBOL_LENGTH)) {
            writeSilence();

            space -= 4U * NXDN_RADIO_SYMBOL_LENGTH;
            m_tailCnt--;

            if (m_tailCnt == 0U)
                return;
            if (m_fifo.getData() > 0U) {
                m_tailCnt = 0U;
                return;
            }
        }

        if (m_fifo.getData() == 0U && m_poLen == 0U)
            return;
    }

    if (m_poLen == 0U) {
        if (m_state == NXDNTXSTATE_CAL)
            m_tailCnt = 0U;

        if (m_fifo.getData() == 0U)
            return;

        createData();

        DEBUG2("NXDNTX: process(): poLen", m_poLen);
    }

    if (m_poLen > 0U) {
        uint16_t space = io.getSpace();

        while (space > (4U * NXDN_RADIO_SYMBOL_LENGTH)) {
            uint8_t c = m_poBuffer[m_poPtr++];

            writeByte(c);

            space -= 4U * NXDN_RADIO_SYMBOL_LENGTH;
            m_tailCnt = m_txHang;

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
uint8_t NXDNTX::writeData(const uint8_t* data, uint16_t length)
{
    if (length != (NXDN_FRAME_LENGTH_BYTES + 1U))
        return RSN_ILLEGAL_LENGTH;

    uint16_t space = m_fifo.getSpace();
    DEBUG3("NXDNTX: writeData(): dataLength/fifoLength", length, space);
    if (space < length) {
        m_fifo.reset();
        return RSN_RINGBUFF_FULL;
    }

    if (space < NXDN_FRAME_LENGTH_BYTES)
        return RSN_RINGBUFF_FULL;

    for (uint8_t i = 0U; i < NXDN_FRAME_LENGTH_BYTES; i++)
        m_fifo.put(data[i + 1U]);

    return RSN_OK;
}

/// <summary>
/// Clears the local buffer.
/// </summary>
void NXDNTX::clear()
{
    m_fifo.reset();
}

/// <summary>
/// Sets the FDMA preamble count.
/// </summary>
/// <param name="preambleCnt">Count of preambles.</param>
void NXDNTX::setPreambleCount(uint8_t preambleCnt)
{
    m_preambleCnt = 300U + uint16_t(preambleCnt) * 6U; // 500ms + tx delay

    if (m_preambleCnt > 1200U)
        m_preambleCnt = 1200U;
}

/// <summary>
/// Sets the Tx hang time.
/// </summary>
/// <param name="txHang">Transmit hang time in seconds.</param>
void NXDNTX::setTxHang(uint8_t txHang)
{
    m_txHang = txHang * NXDN_FIXED_TX_HANG;
}

/// <summary>
/// Sets the fine adjust 4FSK symbol levels.
/// </summary>
/// <param name="level3Adj">+3/-3 symbol adjust.</param>
/// <param name="level1Adj">+1/-1 symbol adjust.</param>
void NXDNTX::setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj)
{
    m_symLevel3Adj = level3Adj;
    m_symLevel1Adj = level1Adj;

    // clamp level adjustments no more then +/- 128
    if (m_symLevel3Adj > 128)
        m_symLevel3Adj = 0;
    if (m_symLevel3Adj < -128)
        m_symLevel3Adj = 0;

    // clamp level adjustments no more then +/- 128
    if (m_symLevel1Adj > 128)
        m_symLevel1Adj = 0;
    if (m_symLevel1Adj < -128)
        m_symLevel1Adj = 0;
}

/// <summary>
/// Helper to set the calibration state for Tx.
/// </summary>
/// <param name="start"></param>
void NXDNTX::setCal(bool start)
{
    m_state = start ? NXDNTXSTATE_CAL : NXDNTXSTATE_NORMAL;
}

/// <summary>
/// Helper to get how much space the ring buffer has for samples.
/// </summary>
/// <returns></returns>
uint8_t NXDNTX::getSpace() const
{
    return m_fifo.getSpace() / NXDN_FRAME_LENGTH_BYTES;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/// <summary>
///
/// </summary>
void NXDNTX::createData()
{
    if (!m_tx) {
        for (uint16_t i = 0U; i < m_preambleCnt; i++)
            m_poBuffer[m_poLen++] = NXDN_SYNC;

        m_poBuffer[m_poLen++] = NXDN_PREAMBLE[0U];
        m_poBuffer[m_poLen++] = NXDN_PREAMBLE[1U];
        m_poBuffer[m_poLen++] = NXDN_PREAMBLE[2U];
    }
    else {
        DEBUG2("NXDNTX: createData(): fifoSpace", m_fifo.getSpace());
        for (uint8_t i = 0U; i < NXDN_FRAME_LENGTH_BYTES; i++) {
            m_poBuffer[m_poLen++] = m_fifo.get();
        }
    }

    m_poPtr = 0U;
}

/// <summary>
///
/// </summary>
/// <param name="c"></param>
void NXDNTX::writeByte(uint8_t c)
{
    q15_t inBuffer[4U];
    q15_t intBuffer[NXDN_RADIO_SYMBOL_LENGTH * 4U];
    q15_t outBuffer[NXDN_RADIO_SYMBOL_LENGTH * 4U];

    const uint8_t MASK = 0xC0U;

    for (uint8_t i = 0U; i < 4U; i++, c <<= 2) {
        switch (c & MASK) {
        case 0xC0U:
            inBuffer[i] = (NXDN_LEVELA + m_symLevel3Adj); // +3
            break;
        case 0x80U:
            inBuffer[i] = (NXDN_LEVELB + m_symLevel1Adj); // +1
            break;
        case 0x00U:
            inBuffer[i] = (NXDN_LEVELC + -m_symLevel1Adj); // -1
            break;
        default: // 0x40U
            inBuffer[i] = (NXDN_LEVELD + -m_symLevel3Adj); // -3
            break;
        }
    }

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, intBuffer, 4U);

    ::arm_fir_fast_q15(&m_sincFilter, intBuffer, outBuffer, NXDN_RADIO_SYMBOL_LENGTH * 4U);

    io.write(STATE_NXDN, outBuffer, NXDN_RADIO_SYMBOL_LENGTH * 4U);
}

/// <summary>
///
/// </summary>
void NXDNTX::writeSilence()
{
    q15_t inBuffer[4U] = {0x00U, 0x00U, 0x00U, 0x00U};
    q15_t intBuffer[NXDN_RADIO_SYMBOL_LENGTH * 4U];
    q15_t outBuffer[NXDN_RADIO_SYMBOL_LENGTH * 4U];

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, intBuffer, 4U);

    ::arm_fir_fast_q15(&m_sincFilter, intBuffer, outBuffer, NXDN_RADIO_SYMBOL_LENGTH * 4U);

    io.write(STATE_NXDN, outBuffer, NXDN_RADIO_SYMBOL_LENGTH * 4U);
}

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
#include "p25/P25TX.h"
#include "p25/P25Defines.h"

using namespace p25;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

// Generated using rcosdesign(0.2, 8, 5, 'normal') in MATLAB
static q15_t RC_0_2_FILTER[] = {
    -897, -1636, -1840, -1278, 0, 1613, 2936, 3310, 2315, 0, -3011, -5627, -6580, -4839,
    0, 7482, 16311, 24651, 30607, 32767, 30607, 24651, 16311, 7482, 0, -4839, -6580, -5627,
    -3011, 0, 2315, 3310, 2936, 1613, 0, -1278, -1840, -1636, -897, 0
}; // numTaps = 40, L = 5
const uint16_t RC_0_2_FILTER_PHASE_LEN = 8U; // phaseLength = numTaps/L

// Generated in MATLAB using the following commands, and then normalised for unity gain
// shape2 = 'Inverse-sinc Lowpass';
// d2 = fdesign.interpolator(1, shape2);  
// h2 = design(d2, 'SystemObject', true);
static q15_t LOWPASS_FILTER[] = {
    124, -188, -682, 1262, 556, -621, -1912, -911, 2058, 3855, 1234, -4592, -7692, -2799,
    8556, 18133, 18133, 8556, -2799, -7692, -4592, 1234, 3855, 2058, -911, -1912, -621,
    556, 1262, -682, -188, 124
};
const uint16_t LOWPASS_FILTER_LEN = 32U;

const q15_t P25_LEVELA = 1260;
const q15_t P25_LEVELB = 420;
const q15_t P25_LEVELC = -420;
const q15_t P25_LEVELD = -1260;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the P25TX class.
/// </summary>
P25TX::P25TX() :
    m_fifo(P25_TX_BUFFER_LEN),
    m_state(P25TXSTATE_NORMAL),
    m_modFilter(),
    m_lpFilter(),
    m_modState(),
    m_lpState(),
    m_poBuffer(),
    m_poLen(0U),
    m_poPtr(0U),
    m_preambleCnt(P25_FIXED_DELAY),
    m_tailCnt(0U),
    m_symLevel3Adj(0U),
    m_symLevel1Adj(0U)
{
    ::memset(m_modState, 0x00U, 16U * sizeof(q15_t));
    ::memset(m_lpState, 0x00U, 60U * sizeof(q15_t));

    m_modFilter.L = P25_RADIO_SYMBOL_LENGTH;
    m_modFilter.phaseLength = RC_0_2_FILTER_PHASE_LEN;
    m_modFilter.pCoeffs = RC_0_2_FILTER;
    m_modFilter.pState = m_modState;

    m_lpFilter.numTaps = LOWPASS_FILTER_LEN;
    m_lpFilter.pState = m_lpState;
    m_lpFilter.pCoeffs = LOWPASS_FILTER;
}

/// <summary>
/// Process local buffer and transmit on the air interface.
/// </summary>
void P25TX::process()
{
    if (m_fifo.getData() == 0U && m_poLen == 0U && m_tailCnt > 0U) {
        // transmit silence until the hang timer has expired
        uint16_t space = io.getSpace();

        while (space > (4U * P25_RADIO_SYMBOL_LENGTH)) {
            writeByte(P25_START_SYNC);

            space -= 4U * P25_RADIO_SYMBOL_LENGTH;
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
        if (m_state == P25TXSTATE_CAL) {
            createCal();
        }
        else {
            if (m_fifo.getData() == 0U)
                return;

            createData();
        }

        DEBUG2("P25TX: process(): poLen", m_poLen);
    }

    if (m_poLen > 0U) {
        if (!m_tx) {
            io.setTransmit();
        }

        uint16_t space = io.getSpace();

        while (space > (4U * P25_RADIO_SYMBOL_LENGTH)) {
            uint8_t c = m_poBuffer[m_poPtr++];

            writeByte(c);

            space -= 4U * P25_RADIO_SYMBOL_LENGTH;
            m_tailCnt = P25_FIXED_TAIL;

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
uint8_t P25TX::writeData(const uint8_t* data, uint8_t length)
{
    if (length < (P25_TDU_FRAME_LENGTH_BYTES + 1U))
        return RSN_ILLEGAL_LENGTH;

    uint16_t space = m_fifo.getSpace();
    DEBUG3("P25TX: writeData(): dataLength/fifoLength", length, space);
    if (space < length) {
        m_fifo.reset();
        return RSN_RINGBUFF_FULL;
    }

    m_fifo.put(length - 1U);
    for (uint8_t i = 0U; i < (length - 1U); i++)
        m_fifo.put(data[i + 1U]);

    return RSN_OK;
}

/// <summary>
///
/// </summary>
void P25TX::clear()
{
    m_fifo.reset();
}

/// <summary>
///
/// </summary>
/// <param name="preambleCnt"></param>
void P25TX::setPreambleCount(uint8_t preambleCnt)
{
    uint32_t preambles = (uint32_t)((float)preambleCnt / 0.2083F);
    m_preambleCnt = P25_FIXED_DELAY + preambles;

    // clamp preamble count to 250ms maximum
    if (m_preambleCnt > 1200U)
        m_preambleCnt = 1200U;
}

/// <summary>
///
/// </summary>
/// <param name="level3Adj"></param>
/// <param name="level1Adj"></param>
void P25TX::setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj)
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
///
/// </summary>
/// <param name="start"></param>
void P25TX::setCal(bool start)
{
    m_state = start ? P25TXSTATE_CAL : P25TXSTATE_NORMAL;
}

/// <summary>
/// Helper to get how much space the ring buffer has for samples.
/// </summary>
/// <returns></returns>
uint8_t P25TX::getSpace() const
{
    return m_fifo.getSpace() / P25_LDU_FRAME_LENGTH_BYTES;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
/// <summary>
///
/// </summary>
void P25TX::createData()
{
    if (!m_tx) {
        for (uint16_t i = 0U; i < m_preambleCnt; i++)
            m_poBuffer[m_poLen++] = P25_START_SYNC;
    }
    else {
        uint8_t length = m_fifo.get();
        DEBUG3("P25TX: createData(): dataLength/fifoSpace", length, m_fifo.getSpace());
        for (uint8_t i = 0U; i < length; i++) {
            m_poBuffer[m_poLen++] = m_fifo.get();
        }
    }

    m_poPtr = 0U;
}

/// <summary>
///
/// </summary>
void P25TX::createCal()
{
    // 1.2 kHz sine wave generation
    if (m_modemState == STATE_P25_CAL ||
        m_modemState == STATE_P25_LEVELA || m_modemState == STATE_P25_LEVELB ||
        m_modemState == STATE_P25_LEVELC || m_modemState == STATE_P25_LEVELD) {
        for (unsigned int i = 0U; i < P25_LDU_FRAME_LENGTH_BYTES; i++) {
            m_poBuffer[i] = P25_START_SYNC;
        }

        m_poLen = P25_LDU_FRAME_LENGTH_BYTES;
    }

    m_poLen = P25_LDU_FRAME_LENGTH_BYTES;
    m_poPtr = 0U;
}

/// <summary>
///
/// </summary>
/// <param name="c"></param>
void P25TX::writeByte(uint8_t c)
{
    q15_t inBuffer[4U];
    q15_t intBuffer[P25_RADIO_SYMBOL_LENGTH * 4U];
    q15_t outBuffer[P25_RADIO_SYMBOL_LENGTH * 4U];

    const uint8_t MASK = 0xC0U;

    for (uint8_t i = 0U; i < 4U; i++, c <<= 2) {
        switch (c & MASK) {
        case 0xC0U:
            inBuffer[i] = (P25_LEVELA + m_symLevel3Adj); // +3
            break;
        case 0x80U:
            inBuffer[i] = (P25_LEVELB + m_symLevel1Adj); // +1
            break;
        case 0x00U:
            inBuffer[i] = (P25_LEVELC + -m_symLevel1Adj); // -1
            break;
        default: // 0x40U
            inBuffer[i] = (P25_LEVELD + -m_symLevel3Adj); // -3
            break;
        }

        // are we doing a diagnostic transmit of a specific level?
        if (m_modemState == STATE_P25_LEVELA || m_modemState == STATE_P25_LEVELB ||
            m_modemState == STATE_P25_LEVELC || m_modemState == STATE_P25_LEVELD) {
            switch (m_modemState) {
            case STATE_P25_LEVELA:
                inBuffer[i] = (P25_LEVELA + m_symLevel3Adj); // +3
                break;
            case STATE_P25_LEVELB:
                inBuffer[i] = (P25_LEVELB + m_symLevel1Adj); // +1
                break;
            case STATE_P25_LEVELC:
                inBuffer[i] = (P25_LEVELC + -m_symLevel1Adj); // -1
                break;
            case STATE_P25_LEVELD:
                inBuffer[i] = (P25_LEVELD + -m_symLevel3Adj); // -3
                break;
            default:
                break;
            }
        }
    }

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, intBuffer, 4U);

    ::arm_fir_fast_q15(&m_lpFilter, intBuffer, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);

    io.write(STATE_P25, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);
}

/// <summary>
///
/// </summary>
void P25TX::writeSilence()
{
    q15_t inBuffer[4U] = { 0x00U, 0x00U, 0x00U, 0x00U };
    q15_t intBuffer[P25_RADIO_SYMBOL_LENGTH * 4U];
    q15_t outBuffer[P25_RADIO_SYMBOL_LENGTH * 4U];
    
    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, intBuffer, 4U);
    
    ::arm_fir_fast_q15(&m_lpFilter, intBuffer, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);
    
    io.write(STATE_P25, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);
}

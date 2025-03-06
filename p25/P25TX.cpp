// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2017 Andy Uribe, CA6JAU
 *  Copyright (C) 2020-2022 Bryan Biedenkapp, N2PLL
 *
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

#if defined(P25_ALTERNATE_SYM_LEVELS)
const q15_t P25_LEVELA = 1500;
const q15_t P25_LEVELB = 330;
const q15_t P25_LEVELC = -330;
const q15_t P25_LEVELD = -1500;
#else
const q15_t P25_LEVELA = 1220;
const q15_t P25_LEVELB = 410;
const q15_t P25_LEVELC = -410;
const q15_t P25_LEVELD = -1220;
#endif

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the P25TX class. */

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
    m_txHang(P25_FIXED_TX_HANG),
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

/* Process local buffer and transmit on the air interface. */

void P25TX::process()
{
    if (m_fifo.getData() == 0U && m_poLen == 0U && m_tailCnt > 0U &&
        m_state != P25TXSTATE_CAL) {
        // transmit silence until the hang timer has expired
        uint16_t space = io.getSpace();

        while (space > (4U * P25_RADIO_SYMBOL_LENGTH)) {
            writeSilence();

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
            m_tailCnt = 0U;
            createCal();
        }
        else {
            if (m_fifo.getData() == 0U)
                return;

            createData();
        }
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
            m_tailCnt = m_txHang;

            if (m_poPtr >= m_poLen) {
                m_poPtr = 0U;
                m_poLen = 0U;
                return;
            }
        }
    }
}

/* Write data to the local buffer. */

uint8_t P25TX::writeData(const uint8_t* data, uint16_t length)
{
    if (length < (P25_TDU_FRAME_LENGTH_BYTES + 1U))
        return RSN_ILLEGAL_LENGTH;

    uint16_t space = m_fifo.getSpace();
    DEBUG3("P25TX::writeData() dataLength/fifoLength", length, space);
    if (space < length) {
        m_fifo.reset();
        return RSN_RINGBUFF_FULL;
    }

    m_fifo.put(length - 1U);
    for (uint16_t i = 0U; i < (length - 1U); i++)
        m_fifo.put(data[i + 1U]);

    return RSN_OK;
}

/* Clears the local buffer. */

void P25TX::clear()
{
    m_fifo.reset();
}

/* Sets the FDMA preamble count. */

void P25TX::setPreambleCount(uint8_t preambleCnt)
{
    m_preambleCnt = P25_FIXED_DELAY + preambleCnt;

    // clamp preamble count to 250ms maximum
    if (m_preambleCnt > 1200U)
        m_preambleCnt = 1200U;
}

/* Sets the Tx hang time. */

void P25TX::setTxHang(uint8_t txHang)
{
    if (txHang > 0U)
        m_txHang = txHang * 1200U;
    else
        m_txHang = P25_FIXED_TX_HANG;

    // clamp tx hang count to 13s maximum
    if (txHang > 13U)
        m_txHang = 13U * 1200U;
}

/* Sets the fine adjust 4FSK symbol levels. */

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

/* Helper to set the calibration state for Tx. */

void P25TX::setCal(bool start)
{
    m_state = start ? P25TXSTATE_CAL : P25TXSTATE_NORMAL;
}

/* Helper to resize the FIFO buffer. */

void P25TX::resizeBuffer(uint16_t size)
{
    m_fifo.reset();
    m_fifo.reinitialize(size);
}

/* Helper to get how much space the ring buffer has for samples. */

uint8_t P25TX::getSpace() const
{
    return m_fifo.getSpace() / P25_LDU_FRAME_LENGTH_BYTES;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Helper to generate data. */

void P25TX::createData()
{
    if (!m_tx) {
        for (uint16_t i = 0U; i < m_preambleCnt; i++)
            m_poBuffer[m_poLen++] = P25_START_SYNC;
    }
    else {
        uint16_t length = m_fifo.get();
        DEBUG3("P25TX::createData() dataLength/fifoSpace", length, m_fifo.getSpace());
        for (uint16_t i = 0U; i < length; i++) {
            m_poBuffer[m_poLen++] = m_fifo.get();
        }
    }

    m_poPtr = 0U;
}

/* Helper to generate calibration data. */

void P25TX::createCal()
{
    // 1.2 kHz sine wave generation
    if (m_modemState == STATE_P25_CAL) {
        for (uint8_t i = 0U; i < P25_LDU_FRAME_LENGTH_BYTES; i++) {
            m_poBuffer[i] = P25_START_SYNC;
        }

        m_poLen = P25_LDU_FRAME_LENGTH_BYTES;
    }

    m_poLen = P25_LDU_FRAME_LENGTH_BYTES;
    m_poPtr = 0U;
}

/* Helper to write a raw byte to the DAC. */

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
    }

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, intBuffer, 4U);

    ::arm_fir_fast_q15(&m_lpFilter, intBuffer, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);

    io.write(STATE_P25, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);
}

/* */

void P25TX::writeSilence()
{
    q15_t inBuffer[4U] = { 0x00U, 0x00U, 0x00U, 0x00U };
    q15_t intBuffer[P25_RADIO_SYMBOL_LENGTH * 4U];
    q15_t outBuffer[P25_RADIO_SYMBOL_LENGTH * 4U];
    
    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, intBuffer, 4U);
    
    ::arm_fir_fast_q15(&m_lpFilter, intBuffer, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);
    
    io.write(STATE_P25, outBuffer, P25_RADIO_SYMBOL_LENGTH * 4U);
}

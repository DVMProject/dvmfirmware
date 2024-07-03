// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2009-2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2017 Andy Uribe, CA6JAU
 *  Copyright (C) 2020 Bryan Biedenkapp, N2PLL
 *
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

// PR FILL pattern
const uint8_t PR_FILL[] =
        { 0x63U, 0xEAU, 0x00U, 0x76U, 0x6CU, 0x76U, 0xC4U, 0x52U, 0xC8U, 0x78U,
          0x09U, 0x2DU, 0xB8U, 0x79U, 0x27U, 0x57U, 0x9BU, 0x31U, 0xBCU, 0x3EU,
          0xEAU, 0x45U, 0xC3U, 0x30U, 0x49U, 0x17U, 0x93U, 0xAEU, 0x8BU, 0x6DU,
          0xA4U, 0xA5U, 0xADU, 0xA2U, 0xF1U, 0x35U, 0xB5U, 0x3CU, 0x1EU };

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the DMRDMOTX class. */

DMRDMOTX::DMRDMOTX() :
    m_fifo(DMR_TX_BUFFER_LEN),
    m_modFilter(),
    m_modState(),
    m_poBuffer(),
    m_poLen(0U),
    m_poPtr(0U),
    m_preambleCnt(DMRDMO_FIXED_DELAY),
    m_symLevel3Adj(0U),
    m_symLevel1Adj(0U)
{
    ::memset(m_modState, 0x00U, 16U * sizeof(q15_t));

    m_modFilter.L = DMR_RADIO_SYMBOL_LENGTH;
    m_modFilter.phaseLength = RRC_0_2_FILTER_PHASE_LEN;
    m_modFilter.pCoeffs = RRC_0_2_FILTER;
    m_modFilter.pState = m_modState;
}

/* Process local buffer and transmit on the air interface. */

void DMRDMOTX::process()
{
    if (m_poLen == 0U && m_fifo.getData() > 0U) {
        if (!m_tx) {
            for (uint16_t i = 0U; i < m_preambleCnt; i++)
                m_poBuffer[i] = DMR_START_SYNC;

            m_poLen = m_preambleCnt;
        }
        else {
            for (unsigned int i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
                m_poBuffer[i] = m_fifo.get();

            for (unsigned int i = 0U; i < 39U; i++)
                m_poBuffer[i + DMR_FRAME_LENGTH_BYTES] = PR_FILL[i];

            m_poLen = 72U;
        }

        m_poPtr = 0U;
    }

    if (m_poLen > 0U) {
        if (!m_tx) {
            io.setTransmit();
        }

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

/* Write data to the local buffer. */

uint8_t DMRDMOTX::writeData(const uint8_t* data, uint8_t length)
{
    if (length != (DMR_FRAME_LENGTH_BYTES + 1U))
        return RSN_ILLEGAL_LENGTH;

    uint16_t space = m_fifo.getSpace();
    DEBUG3("DMRDMOTX::writeData() dataLength/fifoLength", length, space);
    if (space < DMR_FRAME_LENGTH_BYTES)
        return RSN_RINGBUFF_FULL;

    for (uint8_t i = 0U; i < DMR_FRAME_LENGTH_BYTES; i++)
        m_fifo.put(data[i + 1U]);

    return RSN_OK;
}

/* Sets the FDMA preamble count. */

void DMRDMOTX::setPreambleCount(uint8_t preambleCnt)
{
    uint32_t preambles = (uint32_t)((float)preambleCnt / 0.2083F);
    m_preambleCnt = DMRDMO_FIXED_DELAY + preambles;

    // clamp preamble count to 250ms maximum
    if (m_preambleCnt > 1200U)
        m_preambleCnt = 1200U;
}

/* Sets the fine adjust 4FSK symbol levels. */

void DMRDMOTX::setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj)
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

/* Helper to resize the FIFO buffer. */

void DMRDMOTX::resizeBuffer(uint16_t size)
{
    m_fifo.reset();
    m_fifo.reinitialize(size);
}

/* Helper to get how much space the ring buffer has for samples. */

uint8_t DMRDMOTX::getSpace() const
{
    return m_fifo.getSpace() / (DMR_FRAME_LENGTH_BYTES + 2U);
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Helper to write a raw byte to the DAC. */

void DMRDMOTX::writeByte(uint8_t c)
{
    q15_t inBuffer[4U];
    q15_t outBuffer[DMR_RADIO_SYMBOL_LENGTH * 4U];

    const uint8_t MASK = 0xC0U;

    for (uint8_t i = 0U; i < 4U; i++, c <<= 2) {
        switch (c & MASK) {
        case 0xC0U:
            inBuffer[i] = (DMR_LEVELA + m_symLevel3Adj); // +3
            break;
        case 0x80U:
            inBuffer[i] = (DMR_LEVELB + m_symLevel1Adj); // +1
            break;
        case 0x00U:
            inBuffer[i] = (DMR_LEVELC + -m_symLevel1Adj); // -1
            break;
        default: // 0x40U
            inBuffer[i] = (DMR_LEVELD + -m_symLevel3Adj); // -3
            break;
        }
    }

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, 4U);

    io.write(STATE_DMR, outBuffer, DMR_RADIO_SYMBOL_LENGTH * 4U);
}

/* */

void DMRDMOTX::writeSilence()
{
    q15_t inBuffer[4U] = { 0x00U, 0x00U, 0x00U, 0x00U };
    q15_t outBuffer[DMR_RADIO_SYMBOL_LENGTH * 4U];

    ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, 4U);

    io.write(STATE_DMR, outBuffer, DMR_RADIO_SYMBOL_LENGTH * 4U);
}

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
*   Copyright (C) 2017-2019 Bryan Biedenkapp N2PLL
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
#include "p25/P25RX.h"
#include "Utils.h"

using namespace p25;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const q15_t SCALING_FACTOR = 18750;      // Q15(0.57)

const uint8_t CORRELATION_COUNTDOWN = 8U;

const uint8_t MAX_SYNC_BIT_START_ERRS = 2U;
const uint8_t MAX_SYNC_BIT_RUN_ERRS = 4U;
const uint8_t MAX_SYNC_SYMBOLS_ERRS = 2U;

const uint8_t NOAVEPTR = 99U;

const uint16_t NOENDPTR = 9999U;

const unsigned int MAX_SYNC_FRAMES = 5U;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the P25RX class.
/// </summary>
P25RX::P25RX() :
    m_state(P25RXS_NONE),
    m_bitBuffer(),
    m_buffer(),
    m_bitPtr(0U),
    m_dataPtr(0U),
    m_minSyncPtr(NOENDPTR),
    m_maxSyncPtr(NOENDPTR),
    m_burstStartPtr(NOENDPTR),
    m_burstSyncPtr(NOENDPTR),
    m_lduStartPtr(NOENDPTR),
    m_lduEndPtr(NOENDPTR),
    m_lduSyncPtr(NOENDPTR),
    m_averagePtr(NOAVEPTR),
    m_maxCorr(0),
    m_lostCount(0U),
    m_countdown(0U),
    m_centre(),
    m_centreVal(0),
    m_threshold(),
    m_thresholdVal(0),
    m_rssiAccum(0U),
    m_rssiCount(0U),
    m_nac(0xF7EU),
    m_duid(0xFFU)
{
    /* stub */
}

/// <summary>
/// Helper to reset data values to defaults.
/// </summary>
void P25RX::reset()
{
    m_state = P25RXS_NONE;

    //m_dataPtr = 0U;
    //m_bitPtr = 0U;

    m_minSyncPtr = NOENDPTR;
    m_maxSyncPtr = NOENDPTR;

    m_burstStartPtr = NOENDPTR;
    m_burstSyncPtr = NOENDPTR;

    m_lduStartPtr = NOENDPTR;
    m_lduEndPtr = NOENDPTR;
    m_lduSyncPtr = NOENDPTR;

    m_averagePtr = NOAVEPTR;

    m_centreVal = 0;
    m_thresholdVal = 0;

    m_lostCount = 0U;
    m_countdown = 0U;

    m_rssiAccum = 0U;
    m_rssiCount = 0U;

    m_maxCorr = 0;

    m_duid = 0xFFU;
}

/// <summary>
/// Sample P25 values from the air interface.
/// </summary>
/// <param name="samples"></param>
/// <param name="rssi"></param>
/// <param name="length"></param>
void P25RX::samples(const q15_t* samples, uint16_t* rssi, uint8_t length)
{
    for (uint8_t i = 0U; i < length; i++) {
        q15_t sample = samples[i];

        m_rssiAccum += rssi[i];
        m_rssiCount++;

        m_buffer[m_dataPtr] = sample;

        m_bitBuffer[m_bitPtr] <<= 1;
        if (sample < 0)
            m_bitBuffer[m_bitPtr] |= 0x01U;

        if (m_state == P25RXS_BURST) {
            processBurst(sample);
        }
        else if (m_state == P25RXS_LDU) {
            processLdu(sample);
        }
        else if (m_state == P25RXS_PDU) {
            processPdu(sample);
        }
        else {
            processNone(sample);
        }

        m_dataPtr++;
        if (m_dataPtr >= P25_LDU_FRAME_LENGTH_SAMPLES) {
            m_duid = 0xFFU;
            m_dataPtr = 0U;
        }

        m_bitPtr++;
        if (m_bitPtr >= P25_RADIO_SYMBOL_LENGTH)
            m_bitPtr = 0U;
    }
}

/// <summary>
///
/// </summary>
/// <param name="nac"></param>
void P25RX::setNAC(uint16_t nac)
{
    m_nac = nac;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Helper to perform initial sample processing.
/// </summary>
/// <param name="sample"></param>
void P25RX::processNone(q15_t sample)
{
    bool ret = correlateSync();
    if (ret) {
        // on the first sync, start the countdown to the state change
        if (m_countdown == 0U) {
            m_rssiAccum = 0U;
            m_rssiCount = 0U;

            io.setDecode(true);
            io.setADCDetection(true);

            m_averagePtr = NOAVEPTR;

            m_countdown = CORRELATION_COUNTDOWN;
        }
    }

    if (m_countdown > 0U)
        m_countdown--;

    if (m_countdown == 1U) {
        // calculate range sync positions - these are used for the sync positions
        // of the following LDU after a HDU
        m_minSyncPtr = m_burstSyncPtr + P25_HDU_FRAME_LENGTH_SAMPLES - 1U;
        if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
            m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

        m_maxSyncPtr = m_burstSyncPtr + P25_HDU_FRAME_LENGTH_SAMPLES + 1U;
        if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
            m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

        DEBUG4("P25RX: processNone(): dataPtr/startPtr/lduEndPtr", m_dataPtr, m_burstStartPtr, m_lduEndPtr);
        DEBUG3("P25RX: processNone(): maxSyncPtr/minSyncPtr", m_maxSyncPtr, m_minSyncPtr);

        m_state = P25RXS_BURST;
        m_countdown = 0U;
    }
}

/// <summary>
/// Helper to process "burst" P25 samples.
/// </summary>
/// <param name="sample"></param>
void P25RX::processBurst(q15_t sample)
{
    if (m_minSyncPtr < m_maxSyncPtr) {
        if (m_dataPtr >= m_minSyncPtr && m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }
    else {
        if (m_dataPtr >= m_minSyncPtr || m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }

    if (m_dataPtr == m_maxSyncPtr) {
        m_lostCount--;

        // we've not seen a data sync for too long, signal sync lost and change to P25RXS_NONE
        if (m_lostCount == 0U) {
            DEBUG1("P25RX: processBurst(): sync timed out in burst, lost lock");

            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeP25Lost();
            reset();
        }
        else {
            DEBUG4("P25RX: processBurst(): dataPtr/startPtr/lduEndPtr", m_dataPtr, m_burstStartPtr, m_lduEndPtr);
            DEBUG3("P25RX: processBurst(): maxSyncPtr/minSyncPtr", m_maxSyncPtr, m_minSyncPtr);

            if (!decodeNid(m_burstStartPtr)) {
                io.setDecode(false);
                io.setADCDetection(false);

                serial.writeP25Lost();
                reset();
            }
            else {
                switch (m_duid) {
                    case P25_DUID_HDU:
                        {
                            calculateLevels(m_burstStartPtr, P25_HDU_FRAME_LENGTH_SYMBOLS);

                            DEBUG4("P25RX: processBurst(): sync found in HDU pos/centre/threshold", m_burstSyncPtr, m_centreVal, m_thresholdVal);

                            uint8_t frame[P25_HDU_FRAME_LENGTH_BYTES + 1U];
                            samplesToBits(m_burstStartPtr, P25_HDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                            frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
                            serial.writeP25Data(frame, P25_HDU_FRAME_LENGTH_BYTES + 1U);
                            reset();
                        }
                        return;
                    case P25_DUID_TDU:
                        {
                            calculateLevels(m_burstStartPtr, P25_TDU_FRAME_LENGTH_SYMBOLS);

                            DEBUG4("P25RX: processBurst(): sync found in TDU pos/centre/threshold", m_burstSyncPtr, m_centreVal, m_thresholdVal);

                            uint8_t frame[P25_TDU_FRAME_LENGTH_BYTES + 1U];
                            samplesToBits(m_burstStartPtr, P25_TDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                            frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
                            serial.writeP25Data(frame, P25_TDU_FRAME_LENGTH_BYTES + 1U);
                            reset();
                        }
                        return;
                    case P25_DUID_LDU1:
                        m_state = P25RXS_LDU;
                        break;
                    case P25_DUID_TSDU:
                        {
                            // calculateLevels(m_burstStartPtr, P25_TSDU_FRAME_LENGTH_SYMBOLS);

                            DEBUG4("P25RX: processBurst(): sync found in TSDU pos/centre/threshold", m_burstSyncPtr, m_centreVal, m_thresholdVal);

                            uint8_t frame[P25_TSDU_FRAME_LENGTH_BYTES + 1U];
                            samplesToBits(m_burstStartPtr, P25_TSDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                            frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
                            serial.writeP25Data(frame, P25_TSDU_FRAME_LENGTH_BYTES + 1U);
                            reset();
                        }
                        return;
                    case P25_DUID_LDU2:
                        m_state = P25RXS_LDU;
                        break;
                    case P25_DUID_PDU:
                        m_state = P25RXS_PDU;
                        break;
                    case P25_DUID_TDULC:
                        {
                            calculateLevels(m_burstStartPtr, P25_TDULC_FRAME_LENGTH_SYMBOLS);

                            DEBUG4("P25RX: processBurst(): sync found in TDULC pos/centre/threshold", m_burstSyncPtr, m_centreVal, m_thresholdVal);

                            uint8_t frame[P25_TDULC_FRAME_LENGTH_BYTES + 1U];
                            samplesToBits(m_burstStartPtr, P25_TDULC_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                            frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
                            serial.writeP25Data(frame, P25_TDULC_FRAME_LENGTH_BYTES + 1U);
                            reset();
                        }
                        return;
                    default:
                        {
                            DEBUG3("P25RX: processBurst(): illegal DUID in NID", m_nac, m_duid);
                            reset();
                        }
                        return;
                }
            }

            // calculate range sync positions
            m_minSyncPtr = m_lduSyncPtr + P25_LDU_FRAME_LENGTH_SAMPLES - 1U;
            if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_maxSyncPtr = m_lduSyncPtr + 1U;
            if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_lostCount = MAX_SYNC_FRAMES;

            m_rssiAccum = 0U;
            m_rssiCount = 0U;

            m_maxCorr = 0;
        }
    }
}

/// <summary>
/// Helper to process LDU P25 samples.
/// </summary>
/// <param name="sample"></param>
void P25RX::processLdu(q15_t sample)
{
    if (m_minSyncPtr < m_maxSyncPtr) {
        if (m_dataPtr >= m_minSyncPtr && m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }
    else {
        if (m_dataPtr >= m_minSyncPtr || m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }

    if (m_dataPtr == m_lduEndPtr) {
        if (m_lostCount == MAX_SYNC_FRAMES) {
            m_minSyncPtr = m_lduSyncPtr + P25_LDU_FRAME_LENGTH_SAMPLES - 1U;
            if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_maxSyncPtr = m_lduSyncPtr + 1U;
            if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;
        }

        m_lostCount--;

        // we've not seen a data sync for too long, signal sync lost and change to P25RXS_NONE
        if (m_lostCount == 0U) {
            DEBUG1("P25RX: processLdu(): sync timed out in LDU, lost lock");

            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeP25Lost();
            reset();
        }
        else {
            if (!decodeNid(m_lduStartPtr)) {
                io.setDecode(false);
                io.setADCDetection(false);

                serial.writeP25Lost();
                reset();
            }
            else {
                if (m_duid == P25_DUID_TDU) {
                    calculateLevels(m_lduStartPtr, P25_TDU_FRAME_LENGTH_SYMBOLS);

                    DEBUG4("P25RX: processLdu(): sync found in TDU pos/centre/threshold", m_lduSyncPtr, m_centreVal, m_thresholdVal);

                    uint8_t frame[P25_TDU_FRAME_LENGTH_BYTES + 1U];
                    samplesToBits(m_lduStartPtr, P25_TDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                    frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
                    serial.writeP25Data(frame, P25_TDU_FRAME_LENGTH_BYTES + 1U);

                    io.setDecode(false);
                    io.setADCDetection(false);

                    reset();
                    return;
                }

                calculateLevels(m_lduStartPtr, P25_LDU_FRAME_LENGTH_SYMBOLS);

                DEBUG4("P25RX: processLdu(): sync found in LDU pos/centre/threshold", m_lduSyncPtr, m_centreVal, m_thresholdVal);

                uint8_t frame[P25_LDU_FRAME_LENGTH_BYTES + 3U];
                samplesToBits(m_lduStartPtr, P25_LDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
#if defined(SEND_RSSI_DATA)
                if (m_rssiCount > 0U) {
                    uint16_t rssi = m_rssiAccum / m_rssiCount;
                    frame[217U] = (rssi >> 8) & 0xFFU;
                    frame[218U] = (rssi >> 0) & 0xFFU;

                    serial.writeP25Data(false, frame, P25_LDU_FRAME_LENGTH_BYTES + 3U);
                }
                else {
                    serial.writeP25Data(false, frame, P25_LDU_FRAME_LENGTH_BYTES + 1U);
                }
#else
                serial.writeP25Data(frame, P25_LDU_FRAME_LENGTH_BYTES + 1U);
#endif
                m_rssiAccum = 0U;
                m_rssiCount = 0U;

                m_maxCorr = 0;
            }
        }
    }
}

/// <summary>
/// Helper to process PDU P25 samples.
/// </summary>
/// <param name="sample"></param>
void P25RX::processPdu(q15_t sample)
{
    if (m_minSyncPtr < m_maxSyncPtr) {
        if (m_dataPtr >= m_minSyncPtr && m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }
    else {
        if (m_dataPtr >= m_minSyncPtr || m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }

    if (m_dataPtr == m_lduEndPtr) {
        // only update the centre and threshold if they are from a good sync
        if (m_lostCount == MAX_SYNC_FRAMES) {
            m_minSyncPtr = m_lduSyncPtr + P25_LDU_FRAME_LENGTH_SAMPLES - 1U;
            if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_maxSyncPtr = m_lduSyncPtr + 1U;
            if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;
        }

        m_lostCount--;

        // we've not seen a data sync for too long, signal sync lost and change to P25RXS_NONE
        if (m_lostCount == 0U) {
            DEBUG1("P25RX: processPdu(): sync timed out in PDU, lost lock");

            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeP25Lost();
            reset();
        }
        else {
            if (!decodeNid(m_lduStartPtr)) {
                io.setDecode(false);
                io.setADCDetection(false);

                serial.writeP25Lost();
                reset();
            }
            else {
                // calculateLevels(m_lduStartPtr, P25_LDU_FRAME_LENGTH_SYMBOLS);

                DEBUG4("P25RX: processPdu(): sync found in PDU pos/centre/threshold", m_lduSyncPtr, m_centreVal, m_thresholdVal);

                uint8_t frame[P25_LDU_FRAME_LENGTH_BYTES + 3U];
                samplesToBits(m_lduStartPtr, P25_LDU_FRAME_LENGTH_SYMBOLS, frame, 8U, m_centreVal, m_thresholdVal);

                frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U;
                serial.writeP25Data(frame, P25_LDU_FRAME_LENGTH_BYTES + 1U);

                m_rssiAccum = 0U;
                m_rssiCount = 0U;

                m_maxCorr = 0;
            }
        }
    }
}

/// <summary>
/// Frame synchronization correlator.
/// </summary>
/// <returns></returns>
bool P25RX::correlateSync()
{
    uint8_t errs = countBits32((m_bitBuffer[m_bitPtr] & P25_SYNC_SYMBOLS_MASK) ^ P25_SYNC_SYMBOLS);

    if (errs <= MAX_SYNC_SYMBOLS_ERRS) {
        uint16_t ptr = m_dataPtr + P25_LDU_FRAME_LENGTH_SAMPLES - P25_SYNC_LENGTH_SAMPLES + P25_RADIO_SYMBOL_LENGTH;
        if (ptr >= P25_LDU_FRAME_LENGTH_SAMPLES)
            ptr -= P25_LDU_FRAME_LENGTH_SAMPLES;

        q31_t corr = 0;
        q15_t min = 16000;
        q15_t max = -16000;

        for (uint8_t i = 0U; i < P25_SYNC_LENGTH_SYMBOLS; i++) {
            q15_t val = m_buffer[ptr];

            if (val > max)
                max = val;
            if (val < min)
                min = val;

            switch (P25_SYNC_SYMBOLS_VALUES[i]) {
            case +3:
                corr -= (val + val + val);
                break;
            case +1:
                corr -= val;
                break;
            case -1:
                corr += val;
                break;
            default:  // -3
                corr += (val + val + val);
                break;
            }

            ptr += P25_RADIO_SYMBOL_LENGTH;
            if (ptr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                ptr -= P25_LDU_FRAME_LENGTH_SAMPLES;
        }

        if (corr > m_maxCorr) {
            if (m_averagePtr == NOAVEPTR) {
                m_centreVal = (max + min) >> 1;

                q31_t v1 = (max - m_centreVal) * SCALING_FACTOR;
                m_thresholdVal = q15_t(v1 >> 15);
            }

            uint16_t startPtr = m_dataPtr + P25_LDU_FRAME_LENGTH_SAMPLES - P25_SYNC_LENGTH_SAMPLES + P25_RADIO_SYMBOL_LENGTH;
            if (startPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                startPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            uint8_t sync[P25_SYNC_BYTES_LENGTH];
            samplesToBits(startPtr, P25_SYNC_LENGTH_SYMBOLS, sync, 0U, m_centreVal, m_thresholdVal);

            uint8_t maxErrs;
            if (m_state == P25RXS_NONE)
                maxErrs = MAX_SYNC_BIT_START_ERRS;
            else
                maxErrs = MAX_SYNC_BIT_RUN_ERRS;

            uint8_t errs = 0U;
            for (uint8_t i = 0U; i < P25_SYNC_BYTES_LENGTH; i++)
                errs += countBits8(sync[i] ^ P25_SYNC_BYTES[i]);

            if (errs <= maxErrs) {
                // DEBUG2("P25RX: correlateSync(): correlateSync errs", errs);
                m_maxCorr = corr;
                m_lostCount = MAX_SYNC_FRAMES;

                m_lduSyncPtr = m_dataPtr;

                // these are the positions of the start and end of an LDU
                m_lduStartPtr = startPtr;
                m_lduEndPtr = m_dataPtr + P25_LDU_FRAME_LENGTH_SAMPLES - P25_SYNC_LENGTH_SAMPLES - 1U;
                if (m_lduEndPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                    m_lduEndPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

                if (m_state == P25RXS_NONE) {
                    m_burstSyncPtr = m_dataPtr;

                    // this is the position of the start of a xDU
                    m_burstStartPtr = startPtr;

                    // calculate range sync positions - these are used for the sync positions
                    // of the following LDU after a HDU
                    m_minSyncPtr = m_dataPtr + P25_HDU_FRAME_LENGTH_SAMPLES - 1U;
                    if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                        m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

                    m_maxSyncPtr = m_dataPtr + P25_HDU_FRAME_LENGTH_SAMPLES + 1U;
                    if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                        m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

                    DEBUG4("P25RX: correlateSync(): dataPtr/startPtr/lduEndPtr", m_dataPtr, startPtr, m_lduEndPtr);
                    DEBUG3("P25RX: correlateSync(): maxSyncPtr/minSyncPtr", m_maxSyncPtr, m_minSyncPtr);
                }

                return true;
            }
        }
    }

    return false;
}

/// <summary>
///
/// </summary>
/// <param name="start"></param>
bool P25RX::decodeNid(uint16_t start)
{
    uint16_t nidStartPtr = start + P25_SYNC_LENGTH_SAMPLES;
    if (nidStartPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
        nidStartPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

    uint8_t nid[P25_NID_LENGTH_BYTES];
    samplesToBits(nidStartPtr, P25_NID_LENGTH_SYMBOLS, nid, 0U, m_centreVal, m_thresholdVal);

    if (m_nac == 0xF7EU) {
        m_duid = nid[1U] & 0x0FU;
        DEBUG2("P25RX: decodeNid(): DUID for xDU", m_duid);
        return true;
    }

    uint16_t nac = (nid[0U] << 4) | ((nid[1U] & 0xF0U) >> 4);
    if (nac == m_nac) {
        m_duid = nid[1U] & 0x0FU;
        DEBUG2("P25RX: decodeNid(): DUID for xDU", m_duid);
        return true;
    }
    else {
        DEBUG3("P25RX: decodeNid(): invalid NAC found; nac != m_nac", nac, m_nac);
    }

    return false;
}

/// <summary>
///
/// </summary>
/// <param name="start"></param>
/// <param name="count"></param>
void P25RX::calculateLevels(uint16_t start, uint16_t count)
{
    q15_t maxPos = -16000;
    q15_t minPos = 16000;
    q15_t maxNeg = 16000;
    q15_t minNeg = -16000;

    for (uint16_t i = 0U; i < count; i++) {
        q15_t sample = m_buffer[start];

        if (sample > 0) {
            if (sample > maxPos)
                maxPos = sample;
            if (sample < minPos)
                minPos = sample;
        }
        else {
            if (sample < maxNeg)
                maxNeg = sample;
            if (sample > minNeg)
                minNeg = sample;
        }

        start += P25_RADIO_SYMBOL_LENGTH;
        if (start >= P25_LDU_FRAME_LENGTH_SAMPLES)
            start -= P25_LDU_FRAME_LENGTH_SAMPLES;
    }

    q15_t posThresh = (maxPos + minPos) >> 1;
    q15_t negThresh = (maxNeg + minNeg) >> 1;

    q15_t centre = (posThresh + negThresh) >> 1;

    q15_t threshold = posThresh - centre;

    DEBUG5("P25RX: calculateLevels(): pos/neg/centre/threshold", posThresh, negThresh, centre, threshold);

    if (m_averagePtr == NOAVEPTR) {
        for (uint8_t i = 0U; i < 16U; i++) {
            m_centre[i] = centre;
            m_threshold[i] = threshold;
        }

        m_averagePtr = 0U;
    }
    else {
        m_centre[m_averagePtr] = centre;
        m_threshold[m_averagePtr] = threshold;

        m_averagePtr++;
        if (m_averagePtr >= 16U)
            m_averagePtr = 0U;
    }

    m_centreVal = 0;
    m_thresholdVal = 0;

    for (uint8_t i = 0U; i < 16U; i++) {
        m_centreVal += m_centre[i];
        m_thresholdVal += m_threshold[i];
    }

    m_centreVal >>= 4;
    m_thresholdVal >>= 4;
}

/// <summary>
///
/// </summary>
/// <param name="start"></param>
/// <param name="count"></param>
/// <param name="buffer"></param>
/// <param name="offset"></param>
/// <param name="centre"></param>
/// <param name="threshold"></param>
void P25RX::samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold)
{
    for (uint16_t i = 0U; i < count; i++) {
        q15_t sample = m_buffer[start] - centre;

        if (sample < -threshold) {
            _WRITE_BIT(buffer, offset, false);
            offset++;
            _WRITE_BIT(buffer, offset, true);
            offset++;
        }
        else if (sample < 0) {
            _WRITE_BIT(buffer, offset, false);
            offset++;
            _WRITE_BIT(buffer, offset, false);
            offset++;
        }
        else if (sample < threshold) {
            _WRITE_BIT(buffer, offset, true);
            offset++;
            _WRITE_BIT(buffer, offset, false);
            offset++;
        }
        else {
            _WRITE_BIT(buffer, offset, true);
            offset++;
            _WRITE_BIT(buffer, offset, true);
            offset++;
        }

        start += P25_RADIO_SYMBOL_LENGTH;
        if (start >= P25_LDU_FRAME_LENGTH_SAMPLES)
            start -= P25_LDU_FRAME_LENGTH_SAMPLES;
    }
}

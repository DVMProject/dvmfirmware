// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2009-2018,2020 Jonathan Naylor, G4KLX
 *
 */
#include "Globals.h"
#include "nxdn/NXDNRX.h"
#include "Utils.h"

using namespace nxdn;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const q15_t SCALING_FACTOR = 18750;      // Q15(0.55)

const uint8_t MAX_FSW_BIT_START_ERRS = 1U;
const uint8_t MAX_FSW_BIT_RUN_ERRS   = 3U;

const uint8_t MAX_FSW_SYMBOLS_ERRS = 2U;

const unsigned int MAX_FSW_FRAMES = 5U + 1U;

const uint8_t CORRELATION_COUNTDOWN = 5U;

const uint8_t NOAVEPTR = 99U;
const uint16_t NOENDPTR = 9999U;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the NXDNRX class. */

NXDNRX::NXDNRX() :
    m_bitBuffer(),
    m_buffer(),
    m_bitPtr(0U),
    m_dataPtr(0U),
    m_startPtr(NOENDPTR),
    m_endPtr(NOENDPTR),
    m_fswPtr(NOENDPTR),
    m_minFSWPtr(NOENDPTR),
    m_maxFSWPtr(NOENDPTR),
    m_maxCorr(0),
    m_centre(),
    m_centreVal(0),
    m_threshold(),
    m_thresholdVal(0),
    m_averagePtr(NOAVEPTR),
    m_lostCount(0U),
    m_countdown(0U),
    m_corrCountdown(CORRELATION_COUNTDOWN),
    m_state(NXDNRXS_NONE),
    m_rssiAccum(0U),
    m_rssiCount(0U)
{
    /* stub */
}

/* Helper to reset data values to defaults. */

void NXDNRX::reset()
{
    m_bitPtr = 0U;
    m_dataPtr = 0U;

    m_startPtr = NOENDPTR;
    m_endPtr = NOENDPTR;

    m_fswPtr = NOENDPTR;
    m_minFSWPtr = NOENDPTR;
    m_maxFSWPtr = NOENDPTR;

    m_maxCorr = 0;
    m_centreVal = 0;
    m_thresholdVal = 0;
    m_averagePtr = NOAVEPTR;
    
    m_lostCount = 0U;
    m_countdown = 0U;

    m_state = NXDNRXS_NONE;

    m_rssiAccum = 0U;
    m_rssiCount = 0U;
}

/* Sample P25 values from the air interface. */

void NXDNRX::samples(const q15_t* samples, uint16_t* rssi, uint8_t length)
{
    for (uint8_t i = 0U; i < length; i++) {
        q15_t sample = samples[i];

        m_rssiAccum += rssi[i];
        m_rssiCount++;

        m_bitBuffer[m_bitPtr] <<= 1;
        if (sample < 0)
            m_bitBuffer[m_bitPtr] |= 0x01U;

        m_buffer[m_dataPtr] = sample;

        switch (m_state) {
        case NXDNRXS_DATA:
            processData(sample);
            break;
        default:
            processSample(sample);
            break;
        }

        m_dataPtr++;
        if (m_dataPtr >= NXDN_FRAME_LENGTH_SAMPLES)
            m_dataPtr = 0U;

        m_bitPtr++;
        if (m_bitPtr >= NXDN_RADIO_SYMBOL_LENGTH)
            m_bitPtr = 0U;
    }
}

/* Sets the NXDN sync correlation countdown. */

void NXDNRX::setCorrCount(uint8_t count)
{
    m_corrCountdown = count;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Helper to process NXDN samples. */

void NXDNRX::processSample(q15_t sample)
{
    bool ret = correlateSync();
    if (ret) {
        // On the first sync, start the countdown to the state change
        if (m_countdown == 0U) {
            m_rssiAccum = 0U;
            m_rssiCount = 0U;

            io.setDecode(true);
            io.setADCDetection(true);

            m_averagePtr = NOAVEPTR;
            m_countdown = m_corrCountdown;
            DEBUG2("NXDNRX::processSample() correlation countdown", m_countdown);
        }
    }

    if (m_countdown > 0U)
        m_countdown--;

    if (m_countdown == 1U) {
        m_minFSWPtr = m_fswPtr + NXDN_FRAME_LENGTH_SAMPLES - 1U;
        if (m_minFSWPtr >= NXDN_FRAME_LENGTH_SAMPLES)
            m_minFSWPtr -= NXDN_FRAME_LENGTH_SAMPLES;

        m_maxFSWPtr = m_fswPtr + 1U;
        if (m_maxFSWPtr >= NXDN_FRAME_LENGTH_SAMPLES)
            m_maxFSWPtr -= NXDN_FRAME_LENGTH_SAMPLES;

        m_state = NXDNRXS_DATA;
        m_countdown  = 0U;
    }
}

/* Helper to process NXDN data samples. */

void NXDNRX::processData(q15_t sample)
{
    if (m_minFSWPtr < m_maxFSWPtr) {
        if (m_dataPtr >= m_minFSWPtr && m_dataPtr <= m_maxFSWPtr)
            correlateSync();
    } else {
        if (m_dataPtr >= m_minFSWPtr || m_dataPtr <= m_maxFSWPtr)
            correlateSync();
    }

    if (m_dataPtr == m_endPtr) {
        // Only update the centre and threshold if they are from a good sync
        if (m_lostCount == MAX_FSW_FRAMES) {
            m_minFSWPtr = m_fswPtr + NXDN_FRAME_LENGTH_SAMPLES - 1U;
        if (m_minFSWPtr >= NXDN_FRAME_LENGTH_SAMPLES)
            m_minFSWPtr -= NXDN_FRAME_LENGTH_SAMPLES;

        m_maxFSWPtr = m_fswPtr + 1U;
        if (m_maxFSWPtr >= NXDN_FRAME_LENGTH_SAMPLES)
            m_maxFSWPtr -= NXDN_FRAME_LENGTH_SAMPLES;
        }

        calculateLevels(m_startPtr, NXDN_FRAME_LENGTH_SYMBOLS);

        DEBUG4("NXDNRX::processData() sync found pos/centre/threshold", m_fswPtr, m_centreVal, m_thresholdVal);

        uint8_t frame[NXDN_FRAME_LENGTH_BYTES + 3U];
        samplesToBits(m_startPtr, NXDN_FRAME_LENGTH_SYMBOLS, frame, 8U, m_centreVal, m_thresholdVal);

        // We've not seen a data sync for too long, signal RXLOST and change to RX_NONE
        m_lostCount--;
        if (m_lostCount == 0U) {
            DEBUG1("NXDNRX::processData() sync timed out, lost lock");

            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeNXDNLost();

            m_state = NXDNRXS_NONE;
            m_endPtr = NOENDPTR;
            m_averagePtr = NOAVEPTR;
            m_countdown  = 0U;
            m_maxCorr = 0;
        } else {
            frame[0U] = m_lostCount == (MAX_FSW_FRAMES - 1U) ? 0x01U : 0x00U;

#if defined(SEND_RSSI_DATA)
            if (m_rssiCount > 0U) {
                uint16_t rssi = m_rssiAccum / m_rssiCount;

                frame[49U] = (rssi >> 8) & 0xFFU;
                frame[50U] = (rssi >> 0) & 0xFFU;

                serial.writeNXDNData(frame, NXDN_FRAME_LENGTH_BYTES + 3U);
            } else {
                serial.writeNXDNData(frame, NXDN_FRAME_LENGTH_BYTES + 1U);
            }
#else
            serial.writeNXDNData(frame, NXDN_FRAME_LENGTH_BYTES + 1U);
#endif

            m_rssiAccum = 0U;
            m_rssiCount = 0U;

            m_maxCorr = 0;
        }
    }
}

/* Frame synchronization correlator. */

bool NXDNRX::correlateSync()
{
    if (countBits32((m_bitBuffer[m_bitPtr] & NXDN_FSW_SYMBOLS_MASK) ^ NXDN_FSW_SYMBOLS) <= MAX_FSW_SYMBOLS_ERRS) {
        uint16_t ptr = m_dataPtr + NXDN_FRAME_LENGTH_SAMPLES - NXDN_FSW_LENGTH_SAMPLES + NXDN_RADIO_SYMBOL_LENGTH;
        if (ptr >= NXDN_FRAME_LENGTH_SAMPLES)
            ptr -= NXDN_FRAME_LENGTH_SAMPLES;

        q31_t corr = 0;
        q15_t min =  16000;
        q15_t max = -16000;

        for (uint8_t i = 0U; i < NXDN_FSW_LENGTH_SYMBOLS; i++) {
            q15_t val = m_buffer[ptr];

            if (val > max)
                max = val;
            if (val < min)
                min = val;

            switch (NXDN_FSW_SYMBOLS_VALUES[i]) {
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

            ptr += NXDN_RADIO_SYMBOL_LENGTH;
            if (ptr >= NXDN_FRAME_LENGTH_SAMPLES)
                ptr -= NXDN_FRAME_LENGTH_SAMPLES;
        }

        if (corr > m_maxCorr) {
            if (m_averagePtr == NOAVEPTR) {
                m_centreVal = (max + min) >> 1;

                q31_t v1 = (max - m_centreVal) * SCALING_FACTOR;
                m_thresholdVal = q15_t(v1 >> 15);
            }

            uint16_t startPtr = m_dataPtr + NXDN_FRAME_LENGTH_SAMPLES - NXDN_FSW_LENGTH_SAMPLES + NXDN_RADIO_SYMBOL_LENGTH;
            if (startPtr >= NXDN_FRAME_LENGTH_SAMPLES)
                startPtr -= NXDN_FRAME_LENGTH_SAMPLES;

            uint8_t sync[NXDN_FSW_BYTES_LENGTH];
            samplesToBits(startPtr, NXDN_FSW_LENGTH_SYMBOLS, sync, 0U, m_centreVal, m_thresholdVal);

            uint8_t maxErrs;
            if (m_state == NXDNRXS_NONE)
                maxErrs = MAX_FSW_BIT_START_ERRS;
            else
                maxErrs = MAX_FSW_BIT_RUN_ERRS;

            uint8_t errs = 0U;
            for (uint8_t i = 0U; i < NXDN_FSW_BYTES_LENGTH; i++)
                errs += countBits8((sync[i] & NXDN_FSW_BYTES_MASK[i]) ^ NXDN_FSW_BYTES[i]);

            if (errs <= maxErrs) {
                DEBUG2("NXDNRX::correlateSync() sync errs", errs);

                DEBUG4("NXDNRX::correlateSync() sync [b0 - b2]", sync[0], sync[1], sync[2]);

                m_maxCorr = corr;
                m_lostCount = MAX_FSW_FRAMES;
                m_fswPtr = m_dataPtr;

                m_startPtr = startPtr;

                m_endPtr = m_dataPtr + NXDN_FRAME_LENGTH_SAMPLES - NXDN_FSW_LENGTH_SAMPLES - 1U;
                if (m_endPtr >= NXDN_FRAME_LENGTH_SAMPLES)
                    m_endPtr -= NXDN_FRAME_LENGTH_SAMPLES;

                DEBUG4("NXDNRX::correlateSync() dataPtr/startPtr/endPtr", m_dataPtr, startPtr, m_endPtr);

                return true;
            }
        }
    }

    return false;
}

/* */

void NXDNRX::calculateLevels(uint16_t start, uint16_t count)
{
    q15_t maxPos = -16000;
    q15_t minPos =  16000;
    q15_t maxNeg =  16000;
    q15_t minNeg = -16000;

    for (uint16_t i = 0U; i < count; i++) {
        q15_t sample = m_buffer[start];

        if (sample > 0) {
            if (sample > maxPos)
                maxPos = sample;
            if (sample < minPos)
                minPos = sample;
        } else {
            if (sample < maxNeg)
                maxNeg = sample;
            if (sample > minNeg)
                minNeg = sample;
        }

        start += NXDN_RADIO_SYMBOL_LENGTH;
        if (start >= NXDN_FRAME_LENGTH_SAMPLES)
            start -= NXDN_FRAME_LENGTH_SAMPLES;
    }

    q15_t posThresh = (maxPos + minPos) >> 1;
    q15_t negThresh = (maxNeg + minNeg) >> 1;

    q15_t centre = (posThresh + negThresh) >> 1;

    q15_t threshold = posThresh - centre;

    DEBUG5("NXDNRX::calculateLevels() pos/neg/centre/threshold", posThresh, negThresh, centre, threshold);

    if (m_averagePtr == NOAVEPTR) {
        for (uint8_t i = 0U; i < 16U; i++) {
            m_centre[i] = centre;
            m_threshold[i] = threshold;
        }

        m_averagePtr = 0U;
    } else {
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

/* */

void NXDNRX::samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold)
{
    for (uint16_t i = 0U; i < count; i++) {
        q15_t sample = m_buffer[start] - centre;

        if (sample < -threshold) {
            _WRITE_BIT(buffer, offset, false);
            offset++;
            _WRITE_BIT(buffer, offset, true);
            offset++;
        } else if (sample < 0) {
            _WRITE_BIT(buffer, offset, false);
            offset++;
            _WRITE_BIT(buffer, offset, false);
            offset++;
        } else if (sample < threshold) {
            _WRITE_BIT(buffer, offset, true);
            offset++;
            _WRITE_BIT(buffer, offset, false);
            offset++;
        } else {
            _WRITE_BIT(buffer, offset, true);
            offset++;
            _WRITE_BIT(buffer, offset, true);
            offset++;
        }

        start += NXDN_RADIO_SYMBOL_LENGTH;
        if (start >= NXDN_FRAME_LENGTH_SAMPLES)
            start -= NXDN_FRAME_LENGTH_SAMPLES;
    }
}

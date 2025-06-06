// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2009-2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2017-2025 Bryan Biedenkapp, N2PLL
 *  Copyright (C) 2024 Patrick McDonnell, W3AXL
 *
 */
#include "Globals.h"
#include "p25/P25RX.h"
#include "Utils.h"

using namespace p25;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const q15_t SCALING_FACTOR = 18750;      // Q15(0.57)

const uint8_t MAX_SYNC_SYMBOLS_ERRS = 2U;
const uint8_t MAX_SYNC_BYTES_START_ERRS = 2U;
const uint8_t MAX_SYNC_BYTES_ERRS = 4U;

const uint16_t MAX_SYNC_FRAMES = 7U;

const uint8_t CORRELATION_COUNTDOWN = 6U;

const uint16_t NOENDPTR = 9999U;
const uint8_t NOAVEPTR = 99U;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the P25RX class. */

P25RX::P25RX() :
    m_bitBuffer(),
    m_buffer(),
    m_bitPtr(0U),
    m_dataPtr(0U),
    m_minSyncPtr(0U),
    m_maxSyncPtr(NOENDPTR),
    m_startPtr(0U),
    m_endPtr(NOENDPTR),
    m_pduEndPtr(NOENDPTR),
    m_syncPtr(0U),
    m_maxCorr(0),
    m_centre(),
    m_centreVal(0),
    m_threshold(),
    m_thresholdVal(0),
    m_averagePtr(NOAVEPTR),
    m_lostCount(0U),
    m_countdown(0U),
    m_nac(0xF7EU),
    m_corrCountdown(CORRELATION_COUNTDOWN),
    m_state(P25RXS_NONE),
    m_lduSyncPos(false),
    m_duid(0xFFU),
    m_rssiAccum(0U),
    m_rssiCount(0U)
{
    m_bitBuffer[0U] = m_bitBuffer[1U] = m_bitBuffer[2U] = m_bitBuffer[3U] = m_bitBuffer[4U] = 0U;
}

/* Helper to reset data values to defaults. */

void P25RX::reset()
{
    m_minSyncPtr = 0U;
    m_maxSyncPtr = NOENDPTR;

    m_startPtr = 0U;
    m_endPtr = NOENDPTR;
    m_pduEndPtr = NOENDPTR;
    m_syncPtr = 0U;

    m_bitPtr = 0U;
    m_bitBuffer[0U] = m_bitBuffer[1U] = m_bitBuffer[2U] = m_bitBuffer[3U] = m_bitBuffer[4U] = 0U;

    m_maxCorr = 0;
    m_centreVal = 0;
    m_thresholdVal = 0;
    m_averagePtr = NOAVEPTR;

    m_lostCount = 0U;
    m_countdown = 0U;

    DEBUG1("P25RX::samples() m_state = P25RXS_NONE");
    m_state = P25RXS_NONE;
    m_lduSyncPos = false;

    m_duid = 0xFFU;

    m_rssiAccum = 0U;
    m_rssiCount = 0U;
}

/* Sample P25 values from the air interface. */

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

        if (m_state == P25RXS_SYNC) {
            processSample(sample);
        }
        else if (m_state == P25RXS_VOICE) {
            processVoice(sample);
        }
        else if (m_state == P25RXS_DATA) {
            processData(sample);
        }
        else {
            bool ret = correlateSync();
            if (ret) {
                // on the first sync, start the countdown to the state change
                if (m_countdown == 0U) {
                    m_rssiAccum = 0U;
                    m_rssiCount = 0U;

                    io.setDecode(true);
                    io.setADCDetection(true);

                    m_averagePtr = NOAVEPTR;

                    m_countdown = m_corrCountdown;
                    DEBUG2("P25RX::samples() correlation countdown", m_countdown);
                }
            }

            if (m_countdown > 0U)
                m_countdown--;

            if (m_countdown == 1U) {
                // are we using LDU sync positions?
                if (m_lduSyncPos) {
                    m_minSyncPtr = m_syncPtr + P25_LDU_FRAME_LENGTH_SAMPLES - 1U;
                    if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                        m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

                    m_maxSyncPtr = m_syncPtr + 1U;
                    if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                        m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

                    m_lostCount = MAX_SYNC_FRAMES;
                } else {
                    m_minSyncPtr = m_syncPtr + P25_HDU_FRAME_LENGTH_SAMPLES - 1U;
                    if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                        m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

                    m_maxSyncPtr = m_syncPtr + P25_HDU_FRAME_LENGTH_SAMPLES + 1U;
                    if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                        m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;
                }

                DEBUG4("P25RX::samples() dataPtr/startPtr/endPtr", m_dataPtr, m_startPtr, m_endPtr);
                DEBUG4("P25RX::samples() lostCount/maxSyncPtr/minSyncPtr", m_lostCount, m_maxSyncPtr, m_minSyncPtr);

                m_countdown = 0U;

                DEBUG1("P25RX::samples() m_state = P25RXS_SYNC");
                m_state = P25RXS_SYNC;
            }
        }

        m_dataPtr++;
        if (m_state != P25RXS_DATA) {
            if (m_dataPtr >= P25_LDU_FRAME_LENGTH_SAMPLES) {
                m_duid = 0xFFU;
                m_dataPtr = 0U;
            }
        } else {
            if (m_dataPtr >= P25_PDU_FRAME_LENGTH_SAMPLES) {
                m_duid = 0xFFU;
                m_dataPtr = 0U;
            }
        }

        m_bitPtr++;
        if (m_bitPtr >= P25_RADIO_SYMBOL_LENGTH)
            m_bitPtr = 0U;
    }
}

/* Sets the P25 NAC. */

void P25RX::setNAC(uint16_t nac)
{
    m_nac = nac;
}

/* Sets the P25 sync correlation countdown. */

void P25RX::setCorrCount(uint8_t count)
{
    m_corrCountdown = count;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Helper to process P25 samples. */

void P25RX::processSample(q15_t sample)
{
    // initial sample processing does not have an end pointer -- we simply wait till we've read
    // the samples up to the maximum sync pointer
    if (m_dataPtr == m_maxSyncPtr) {
        DEBUG4("P25RX::processSample() dataPtr/startPtr/endPtr", m_dataPtr, m_startPtr, m_maxSyncPtr);
        DEBUG4("P25RX::processSample() lostCount/maxSyncPtr/minSyncPtr", m_lostCount, m_maxSyncPtr, m_minSyncPtr);

        // calculateLevels(m_startPtr, P25_NID_LENGTH_SYMBOLS);

        if (!decodeNid(m_startPtr)) {
            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeP25Lost();
            reset();
        }
        else {
            switch (m_duid) {
            case P25_DUID_HDU:
                {
                    calculateLevels(m_startPtr, P25_HDU_FRAME_LENGTH_SYMBOLS);

                    DEBUG4("P25RX::processSample() sync found in HDU pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

                    uint8_t frame[P25_HDU_FRAME_LENGTH_BYTES + 1U];
                    samplesToBits(m_startPtr, P25_HDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                    frame[0U] = 0x01U; // has sync
                    serial.writeP25Data(frame, P25_HDU_FRAME_LENGTH_BYTES + 1U);
                    reset();

                    // setup next cycle to use LDU sync positions
                    m_lduSyncPos = true;
                }
                return;
            case P25_DUID_TDU:
                {
                    calculateLevels(m_startPtr, P25_TDU_FRAME_LENGTH_SYMBOLS);

                    DEBUG4("P25RX::processSample() sync found in TDU pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

                    uint8_t frame[P25_TDU_FRAME_LENGTH_BYTES + 1U];
                    samplesToBits(m_startPtr, P25_TDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                    frame[0U] = 0x01U; // has sync
                    serial.writeP25Data(frame, P25_TDU_FRAME_LENGTH_BYTES + 1U);
                    reset();
                }
                return;
            case P25_DUID_LDU1:
            case P25_DUID_VSELP1:
                {
                    // are we using LDU sync positions?
                    if (m_lduSyncPos) {
                        writeLDUFrame();
                    }

                    DEBUG1("P25RX::samples() m_state = P25RXS_VOICE (LDU1)");
                    m_state = P25RXS_VOICE;
                }
                break;
            case P25_DUID_TSDU:
                {
                    // calculateLevels(m_startPtr, P25_TSDU_FRAME_LENGTH_SYMBOLS);

                    DEBUG4("P25RX::processSample() sync found in TSDU pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

                    uint8_t frame[P25_TSDU_FRAME_LENGTH_BYTES + 1U];
                    samplesToBits(m_startPtr, P25_TSDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                    frame[0U] = 0x01U; // has sync
                    serial.writeP25Data(frame, P25_TSDU_FRAME_LENGTH_BYTES + 1U);
                    reset();
                }
                return;
            case P25_DUID_LDU2:
            case P25_DUID_VSELP2:
                DEBUG1("P25RX::samples() m_state = P25RXS_VOICE (LDU2)");
                m_state = P25RXS_VOICE;
                break;
            case P25_DUID_PDU:
                DEBUG1("P25RX::samples() m_state = P25RXS_DATA");
                m_state = P25RXS_DATA;
                m_maxCorr = 0;
                m_lostCount = MAX_SYNC_FRAMES;
                break;
            case P25_DUID_TDULC:
                {
                    calculateLevels(m_startPtr, P25_TDULC_FRAME_LENGTH_SYMBOLS);

                    DEBUG4("P25RX::processSample() sync found in TDULC pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

                    uint8_t frame[P25_TDULC_FRAME_LENGTH_BYTES + 1U];
                    samplesToBits(m_startPtr, P25_TDULC_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                    frame[0U] = 0x01U; // has sync
                    serial.writeP25Data(frame, P25_TDULC_FRAME_LENGTH_BYTES + 1U);
                    reset();
                }
                return;
            default:
                {
                    DEBUG3("P25RX::processSample() illegal DUID in NID", m_nac, m_duid);
                    reset();
                }
                return;
            }
        }

        if (m_state == P25RXS_DATA) {
            processData(sample);
            return;
        }

        // late entry?
        if (!m_lduSyncPos) {
            m_minSyncPtr = m_syncPtr + P25_LDU_FRAME_LENGTH_SAMPLES - 1U;
            if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_maxSyncPtr = m_syncPtr + 1U;
            if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_maxCorr = 0;

            m_lostCount = MAX_SYNC_FRAMES;
        }

        m_rssiAccum = 0U;
        m_rssiCount = 0U;

        if (m_state == P25RXS_VOICE) {
            processVoice(sample);
        }
    }
}

/* Helper to process LDU P25 samples. */

void P25RX::processVoice(q15_t sample)
{
    if (m_minSyncPtr < m_maxSyncPtr) {
        if (m_dataPtr >= m_minSyncPtr && m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }
    else {
        if (m_dataPtr >= m_minSyncPtr || m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }

    // process voice frame
    if (m_dataPtr == m_endPtr) {
        if (m_lostCount == MAX_SYNC_FRAMES) {
            m_minSyncPtr = m_syncPtr + P25_LDU_FRAME_LENGTH_SAMPLES - 1U;
            if (m_minSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_minSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

            m_maxSyncPtr = m_syncPtr + 1U;
            if (m_maxSyncPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                m_maxSyncPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;
        }

        m_lostCount--;

        DEBUG4("P25RX::processVoice() dataPtr/startPtr/endPtr", m_dataPtr, m_startPtr, m_endPtr);
        DEBUG4("P25RX::processVoice() lostCount/maxSyncPtr/minSyncPtr", m_lostCount, m_maxSyncPtr, m_minSyncPtr);

        // we've not seen a data sync for too long, signal sync lost and change to P25RXS_NONE
        if (m_lostCount == 0U) {
            DEBUG1("P25RX::processVoice() sync timeout in LDU, lost lock");

            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeP25Lost();
            reset();
        }
        else {
            if (!decodeNid(m_startPtr)) {
                io.setDecode(false);
                io.setADCDetection(false);

                serial.writeP25Lost();
                reset();
            }
            else {
                if (m_duid == P25_DUID_TDU) {
                    calculateLevels(m_startPtr, P25_TDU_FRAME_LENGTH_SYMBOLS);

                    DEBUG4("P25RX::processVoice() sync found in TDU pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

                    uint8_t frame[P25_TDU_FRAME_LENGTH_BYTES + 1U];
                    samplesToBits(m_startPtr, P25_TDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

                    frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U; // set sync flag
                    serial.writeP25Data(frame, P25_TDU_FRAME_LENGTH_BYTES + 1U);

                    io.setDecode(false);
                    io.setADCDetection(false);

                    reset();
                    return;
                }

                writeLDUFrame();

                m_rssiAccum = 0U;
                m_rssiCount = 0U;

                m_maxCorr = 0;
            }
        }
    }
}

/* Helper to write a LDU data frame. */

void P25RX::writeLDUFrame()
{
    calculateLevels(m_startPtr, P25_LDU_FRAME_LENGTH_SYMBOLS);

    DEBUG4("P25RX::writeLDUFrame() sync found in LDUx pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

    uint8_t frame[P25_LDU_FRAME_LENGTH_BYTES + 3U];
    samplesToBits(m_startPtr, P25_LDU_FRAME_LENGTH_SYMBOLS, frame, P25_NID_LENGTH_SYMBOLS, m_centreVal, m_thresholdVal);

    frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U; // set sync flag
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
}

/* Helper to process PDU P25 samples. */

void P25RX::processData(q15_t sample)
{
    if (m_minSyncPtr < m_maxSyncPtr) {
        if (m_dataPtr >= m_minSyncPtr && m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }
    else {
        if (m_dataPtr >= m_minSyncPtr || m_dataPtr <= m_maxSyncPtr)
            correlateSync();
    }

    // process data frame
    if (m_dataPtr == m_pduEndPtr) {
        // only update the centre and threshold if they are from a good sync
        if (m_lostCount == MAX_SYNC_FRAMES) {
            m_minSyncPtr = m_syncPtr + P25_PDU_FRAME_LENGTH_SAMPLES - 1U;
            if (m_minSyncPtr >= P25_PDU_FRAME_LENGTH_SAMPLES)
                m_minSyncPtr -= P25_PDU_FRAME_LENGTH_SAMPLES;

            m_maxSyncPtr = m_syncPtr + 1U;
            if (m_maxSyncPtr >= P25_PDU_FRAME_LENGTH_SAMPLES)
                m_maxSyncPtr -= P25_PDU_FRAME_LENGTH_SAMPLES;
        }

        m_lostCount--;

        DEBUG4("P25RX::processData() dataPtr/startPtr/pduEndPtr", m_dataPtr, m_startPtr, m_pduEndPtr);
        DEBUG4("P25RX::processData() lostCount/maxSyncPtr/minSyncPtr", m_lostCount, m_maxSyncPtr, m_minSyncPtr);

        // we've not seen a data sync for too long, signal sync lost and change to P25RXS_NONE
        if (m_lostCount == 0U) {
            DEBUG1("P25RX::processData() sync timeout in PDU, lost lock");

            io.setDecode(false);
            io.setADCDetection(false);

            serial.writeP25Lost();
            reset();
        }
        else {
            if (!decodeNid(m_startPtr)) {
                io.setDecode(false);
                io.setADCDetection(false);

                serial.writeP25Lost();
                reset();
            }
            else {
                // calculateLevels(m_startPtr, P25_PDU_FRAME_LENGTH_SYMBOLS);

                DEBUG4("P25RX::processData() sync found in PDU pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

                uint8_t frame[P25_PDU_FRAME_LENGTH_BYTES + 1U];
                samplesToBits(m_startPtr, P25_PDU_FRAME_LENGTH_SYMBOLS, frame, 8U, m_centreVal, m_thresholdVal);

                frame[0U] = m_lostCount == (MAX_SYNC_FRAMES - 1U) ? 0x01U : 0x00U; // set sync flag
                serial.writeP25Data(frame, P25_PDU_FRAME_LENGTH_BYTES + 1U);

                m_rssiAccum = 0U;
                m_rssiCount = 0U;

                m_maxCorr = 0;
            }
        }
    }
}

/* Frame synchronization correlator. */

bool P25RX::correlateSync()
{
    uint8_t symErrs = countBits32((m_bitBuffer[m_bitPtr] & P25_SYNC_SYMBOLS_MASK) ^ P25_SYNC_SYMBOLS);

    if (symErrs <= MAX_SYNC_SYMBOLS_ERRS) {
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
                maxErrs = MAX_SYNC_BYTES_START_ERRS;
            else
                maxErrs = MAX_SYNC_BYTES_ERRS;

            uint8_t errs = 0U;
            for (uint8_t i = 0U; i < P25_SYNC_BYTES_LENGTH; i++)
                errs += countBits8(sync[i] ^ P25_SYNC_BYTES[i]);

            if (errs <= maxErrs) {
                DEBUG2("P25RX::correlateSync() symbol errs", symErrs);
                DEBUG2("P25RX::correlateSync() bitPtr", m_bitPtr);
                DEBUG5("P25RX::correlateSync() bitBuffer", (m_bitBuffer[m_bitPtr] >> 24) & 0xFFU, (m_bitBuffer[m_bitPtr] >> 16) & 0xFFU, (m_bitBuffer[m_bitPtr] >> 8) & 0xFFU, m_bitBuffer[m_bitPtr] & 0xFFU);

                DEBUG2("P25RX::correlateSync() sync errs", errs);

                DEBUG4("P25RX::correlateSync() sync [b0 - b2]", sync[0], sync[1], sync[2]);
                DEBUG4("P25RX::correlateSync() sync [b3 - b5]", sync[3], sync[4], sync[5]);

                m_maxCorr = corr;
                m_lostCount = MAX_SYNC_FRAMES;

                m_syncPtr = m_dataPtr;
                m_startPtr = startPtr;

                m_endPtr = m_dataPtr + P25_LDU_FRAME_LENGTH_SAMPLES - P25_SYNC_LENGTH_SAMPLES - 1U;
                if (m_endPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
                    m_endPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;
                m_pduEndPtr = m_dataPtr + P25_PDU_FRAME_LENGTH_SAMPLES - P25_SYNC_LENGTH_SAMPLES - 1U;
                if (m_pduEndPtr >= P25_PDU_FRAME_LENGTH_SAMPLES)
                    m_pduEndPtr -= P25_PDU_FRAME_LENGTH_SAMPLES;

                DEBUG5("P25RX::correlateSync() dataPtr/startPtr/endPtr/pduEndPtr", m_dataPtr, startPtr, m_endPtr, m_pduEndPtr);

                return true;
            }
        }
    }

    return false;
}

/* Helper to decode the P25 NID. */

bool P25RX::decodeNid(uint16_t start)
{
    uint16_t nidStartPtr = start + P25_SYNC_LENGTH_SAMPLES;
    if (nidStartPtr >= P25_LDU_FRAME_LENGTH_SAMPLES)
        nidStartPtr -= P25_LDU_FRAME_LENGTH_SAMPLES;

    uint8_t nid[P25_NID_LENGTH_BYTES];
    samplesToBits(nidStartPtr, P25_NID_LENGTH_SYMBOLS, nid, 0U, m_centreVal, m_thresholdVal);

    DEBUG3("P25RX::decodeNid() sync [b0 - b1]", nid[0], nid[1]);

    if (m_nac == 0xF7EU) {
        m_duid = nid[1U] & 0x0FU;
        DEBUG2("P25RX::decodeNid() DUID for xDU", m_duid);
        return true;
    }

    uint16_t nac = (nid[0U] << 4) | ((nid[1U] & 0xF0U) >> 4);
    if (nac == m_nac) {
        m_duid = nid[1U] & 0x0FU;
        DEBUG2("P25RX::decodeNid() DUID for xDU", m_duid);
        return true;
    }
    else {
        DEBUG3("P25RX::decodeNid() invalid NAC found; nac != m_nac", nac, m_nac);
    }

    return false;
}

/* */

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

    DEBUG5("P25RX::calculateLevels() pos/neg/centre/threshold", posThresh, negThresh, centre, threshold);

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

/* */

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

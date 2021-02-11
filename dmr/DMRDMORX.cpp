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
#include "dmr/DMRDMORX.h"
#include "dmr/DMRSlotType.h"
#include "Utils.h"

using namespace dmr;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const q15_t SCALING_FACTOR = 19505;      // Q15(0.60)

const uint8_t MAX_SYNC_SYMBOLS_ERRS = 2U;
const uint8_t MAX_SYNC_BYTES_ERRS = 3U;

const uint8_t MAX_SYNC_LOST_FRAMES = 13U;

const uint16_t NOENDPTR = 9999U;

const uint8_t CONTROL_NONE = 0x00U;
const uint8_t CONTROL_VOICE = 0x20U;
const uint8_t CONTROL_DATA = 0x40U;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the DMRDMORX class.
/// </summary>
DMRDMORX::DMRDMORX() :
    m_bitBuffer(),
    m_buffer(),
    m_bitPtr(0U),
    m_dataPtr(0U),
    m_syncPtr(0U),
    m_startPtr(0U),
    m_endPtr(NOENDPTR),
    m_maxCorr(0),
    m_centre(),
    m_threshold(),
    m_averagePtr(0U),
    m_control(CONTROL_NONE),
    m_syncCount(0U),
    m_colorCode(0U),
    m_state(DMORXS_NONE),
    m_n(0U),
    m_type(0U),
    m_rssi()
{
    /* stub */
}

/// <summary>
/// Helper to reset data values to defaults.
/// </summary>
void DMRDMORX::reset()
{
    m_syncPtr = 0U;
    m_maxCorr = 0;
    m_control = CONTROL_NONE;
    m_syncCount = 0U;
    m_state = DMORXS_NONE;
    m_startPtr = 0U;
    m_endPtr = NOENDPTR;
}

/// <summary>
/// Sample DMR values from the air interface.
/// </summary>
/// <param name="samples"></param>
/// <param name="rssi"></param>
/// <param name="length"></param>
void DMRDMORX::samples(const q15_t* samples, const uint16_t* rssi, uint8_t length)
{
    bool dcd = false;

    for (uint8_t i = 0U; i < length; i++)
        dcd = processSample(samples[i], rssi[i]);

    io.setDecode(dcd);
}

/// <summary>
///
/// </summary>
/// <param name="colorCode"></param>
void DMRDMORX::setColorCode(uint8_t colorCode)
{
    m_colorCode = colorCode;
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Helper to perform sample processing.
/// </summary>
/// <param name="sample"></param>
/// <param name="rssi"></param>
/// <returns></returns>
bool DMRDMORX::processSample(q15_t sample, uint16_t rssi)
{
    m_buffer[m_dataPtr] = sample;
    m_rssi[m_dataPtr] = rssi;

    m_bitBuffer[m_bitPtr] <<= 1;
    if (sample < 0)
        m_bitBuffer[m_bitPtr] |= 0x01U;

    if (m_state == DMORXS_NONE) {
        correlateSync(true);
    }
    else {

        uint16_t min = m_syncPtr + DMO_BUFFER_LENGTH_SAMPLES - 1U;
        uint16_t max = m_syncPtr + 1U;

        if (min >= DMO_BUFFER_LENGTH_SAMPLES)
            min -= DMO_BUFFER_LENGTH_SAMPLES;
        if (max >= DMO_BUFFER_LENGTH_SAMPLES)
            max -= DMO_BUFFER_LENGTH_SAMPLES;

        if (min < max) {
            if (m_dataPtr >= min && m_dataPtr <= max)
                correlateSync(false);
        }
        else {
            if (m_dataPtr >= min || m_dataPtr <= max)
                correlateSync(false);
        }
    }

    if (m_dataPtr == m_endPtr) {
        // Find the average centre and threshold values
        q15_t centre = (m_centre[0U] + m_centre[1U] + m_centre[2U] + m_centre[3U]) >> 2;
        q15_t threshold = (m_threshold[0U] + m_threshold[1U] + m_threshold[2U] + m_threshold[3U]) >> 2;

        uint8_t frame[DMR_FRAME_LENGTH_BYTES + 3U];
        frame[0U] = m_control;

        uint16_t ptr = m_endPtr + DMO_BUFFER_LENGTH_SAMPLES - DMR_FRAME_LENGTH_SAMPLES + DMR_RADIO_SYMBOL_LENGTH + 1U;
        if (ptr >= DMO_BUFFER_LENGTH_SAMPLES)
            ptr -= DMO_BUFFER_LENGTH_SAMPLES;

        samplesToBits(ptr, DMR_FRAME_LENGTH_SYMBOLS, frame, 8U, centre, threshold);

        if (m_control == CONTROL_DATA) {
            // Data sync
            uint8_t colorCode;
            uint8_t dataType;
            DMRSlotType slotType;
            slotType.decode(frame + 1U, colorCode, dataType);

            if (colorCode == m_colorCode) {
                m_syncCount = 0U;
                m_n = 0U;

                frame[0U] |= dataType;

                switch (dataType) {
                case DT_DATA_HEADER:
                    DEBUG4("DMRDMORX: processSample(): data header found pos/centre/threshold", m_syncPtr, centre, threshold);
                    writeRSSIData(frame);
                    m_state = DMORXS_DATA;
                    m_type = 0x00U;
                    break;
                case DT_RATE_12_DATA:
                case DT_RATE_34_DATA:
                case DT_RATE_1_DATA:
                    if (m_state == DMORXS_DATA) {
                        DEBUG4("DMRDMORX: processSample(): data payload found pos/centre/threshold", m_syncPtr, centre, threshold);
                        writeRSSIData(frame);
                        m_type = dataType;
                    }
                    break;
                case DT_VOICE_LC_HEADER:
                    DEBUG4("DMRDMORX: processSample(): voice header found pos/centre/threshold", m_syncPtr, centre, threshold);
                    writeRSSIData(frame);
                    m_state = DMORXS_VOICE;
                    break;
                case DT_VOICE_PI_HEADER:
                    if (m_state == DMORXS_VOICE) {
                        DEBUG4("DMRDMORX: processSample(): voice pi header found pos/centre/threshold", m_syncPtr, centre, threshold);
                        writeRSSIData(frame);
                    }
                    m_state = DMORXS_VOICE;
                    break;
                case DT_TERMINATOR_WITH_LC:
                    if (m_state == DMORXS_VOICE) {
                        DEBUG4("DMRDMORX: processSample(): voice terminator found pos/centre/threshold", m_syncPtr, centre, threshold);
                        writeRSSIData(frame);
                        reset();
                    }
                    break;
                default:    // DT_CSBK
                    DEBUG4("DMRDMORX: processSample(): csbk found pos/centre/threshold", m_syncPtr, centre, threshold);
                    writeRSSIData(frame);
                    reset();
                    break;
                }
            }
        }
        else if (m_control == CONTROL_VOICE) {
            // Voice sync
            DEBUG4("DMRDMORX: processSample(): voice sync found pos/centre/threshold", m_syncPtr, centre, threshold);
            writeRSSIData(frame);
            m_state = DMORXS_VOICE;
            m_syncCount = 0U;
            m_n = 0U;
        }
        else {
            if (m_state != DMORXS_NONE) {
                m_syncCount++;
                if (m_syncCount >= MAX_SYNC_LOST_FRAMES) {
                    serial.writeDMRLost(true);
                    reset();
                }
            }

            if (m_state == DMORXS_VOICE) {
                if (m_n >= 5U) {
                    frame[0U] = CONTROL_VOICE;
                    m_n = 0U;
                }
                else {
                    frame[0U] = ++m_n;
                }

                serial.writeDMRData(true, frame, DMR_FRAME_LENGTH_BYTES + 1U);
            }
            else if (m_state == DMORXS_DATA) {
                if (m_type != 0x00U) {
                    frame[0U] = CONTROL_DATA | m_type;
                    writeRSSIData(frame);
                }
            }
        }

        // End of this slot, reset some items for the next slot.
        m_maxCorr = 0;
        m_control = CONTROL_NONE;
    }

    m_dataPtr++;
    if (m_dataPtr >= DMO_BUFFER_LENGTH_SAMPLES)
        m_dataPtr = 0U;

    m_bitPtr++;
    if (m_bitPtr >= DMR_RADIO_SYMBOL_LENGTH)
        m_bitPtr = 0U;

    return m_state != DMORXS_NONE;
}

/// <summary>
/// Frame synchronization correlator.
/// </summary>
/// <param name="first"></param>
void DMRDMORX::correlateSync(bool first)
{
    uint8_t errs = countBits32((m_bitBuffer[m_bitPtr] & DMR_SYNC_SYMBOLS_MASK) ^ DMR_MS_DATA_SYNC_SYMBOLS);

    // The voice sync is the complement of the data sync
    bool data = (errs <= MAX_SYNC_SYMBOLS_ERRS);
    bool voice = (errs >= (DMR_SYNC_LENGTH_SYMBOLS - MAX_SYNC_SYMBOLS_ERRS));

    if (data || voice) {
        uint16_t ptr = m_dataPtr + DMO_BUFFER_LENGTH_SAMPLES - DMR_SYNC_LENGTH_SAMPLES + DMR_RADIO_SYMBOL_LENGTH;
        if (ptr >= DMO_BUFFER_LENGTH_SAMPLES)
            ptr -= DMO_BUFFER_LENGTH_SAMPLES;

        q31_t corr = 0;
        q15_t min = 16000;
        q15_t max = -16000;

        for (uint8_t i = 0U; i < DMR_SYNC_LENGTH_SYMBOLS; i++) {
            q15_t val = m_buffer[ptr];

            if (val > max)
                max = val;
            if (val < min)
                min = val;

            int8_t corrVal;
            if (data)
                corrVal = DMR_MS_DATA_SYNC_SYMBOLS_VALUES[i];
            else
                corrVal = DMR_MS_VOICE_SYNC_SYMBOLS_VALUES[i];

            switch (corrVal) {
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

            ptr += DMR_RADIO_SYMBOL_LENGTH;
            if (ptr >= DMO_BUFFER_LENGTH_SAMPLES)
                ptr -= DMO_BUFFER_LENGTH_SAMPLES;
        }

        if (corr > m_maxCorr) {
            q15_t centre = (max + min) >> 1;

            q31_t v1 = (max - centre) * SCALING_FACTOR;
            q15_t threshold = q15_t(v1 >> 15);

            uint8_t sync[DMR_SYNC_BYTES_LENGTH];
            uint16_t ptr = m_dataPtr + DMO_BUFFER_LENGTH_SAMPLES - DMR_SYNC_LENGTH_SAMPLES + DMR_RADIO_SYMBOL_LENGTH;
            if (ptr >= DMO_BUFFER_LENGTH_SAMPLES)
                ptr -= DMO_BUFFER_LENGTH_SAMPLES;

            samplesToBits(ptr, DMR_SYNC_LENGTH_SYMBOLS, sync, 4U, centre, threshold);

            if (data) {
                uint8_t errs = 0U;
                for (uint8_t i = 0U; i < DMR_SYNC_BYTES_LENGTH; i++)
                    errs += countBits8((sync[i] & DMR_SYNC_BYTES_MASK[i]) ^ DMR_MS_DATA_SYNC_BYTES[i]);

                if (errs <= MAX_SYNC_BYTES_ERRS) {
                    if (first) {
                        m_threshold[0U] = m_threshold[1U] = m_threshold[2U] = m_threshold[3U] = threshold;
                        m_centre[0U] = m_centre[1U] = m_centre[2U] = m_centre[3U] = centre;
                        m_averagePtr = 0U;
                    }
                    else {
                        m_threshold[m_averagePtr] = threshold;
                        m_centre[m_averagePtr] = centre;

                        m_averagePtr++;
                        if (m_averagePtr >= 4U)
                            m_averagePtr = 0U;
                    }

                    m_maxCorr = corr;
                    m_control = CONTROL_DATA;
                    m_syncPtr = m_dataPtr;

                    m_startPtr = m_dataPtr + DMO_BUFFER_LENGTH_SAMPLES - DMR_SLOT_TYPE_LENGTH_SAMPLES / 2U - DMR_INFO_LENGTH_SAMPLES / 2U - DMR_SYNC_LENGTH_SAMPLES;
                    if (m_startPtr >= DMO_BUFFER_LENGTH_SAMPLES)
                        m_startPtr -= DMO_BUFFER_LENGTH_SAMPLES;

                    m_endPtr = m_dataPtr + DMR_SLOT_TYPE_LENGTH_SAMPLES / 2U + DMR_INFO_LENGTH_SAMPLES / 2U - 1U;
                    if (m_endPtr >= DMO_BUFFER_LENGTH_SAMPLES)
                        m_endPtr -= DMO_BUFFER_LENGTH_SAMPLES;

                    DEBUG5("DMRDMORX: correlateSync(): dataPtr/syncPtr/startPtr/lduEndPtr", m_dataPtr, m_syncPtr, m_startPtr, m_endPtr);
                }
            }
            else {  // if (voice1 || voice2)
                uint8_t errs = 0U;
                for (uint8_t i = 0U; i < DMR_SYNC_BYTES_LENGTH; i++)
                    errs += countBits8((sync[i] & DMR_SYNC_BYTES_MASK[i]) ^ DMR_MS_VOICE_SYNC_BYTES[i]);

                if (errs <= MAX_SYNC_BYTES_ERRS) {
                    if (first) {
                        m_threshold[0U] = m_threshold[1U] = m_threshold[2U] = m_threshold[3U] = threshold;
                        m_centre[0U] = m_centre[1U] = m_centre[2U] = m_centre[3U] = centre;
                        m_averagePtr = 0U;
                    }
                    else {
                        m_threshold[m_averagePtr] = threshold;
                        m_centre[m_averagePtr] = centre;

                        m_averagePtr++;
                        if (m_averagePtr >= 4U)
                            m_averagePtr = 0U;
                    }

                    m_maxCorr = corr;
                    m_control = CONTROL_VOICE;
                    m_syncPtr = m_dataPtr;

                    m_startPtr = m_dataPtr + DMO_BUFFER_LENGTH_SAMPLES - DMR_SLOT_TYPE_LENGTH_SAMPLES / 2U - DMR_INFO_LENGTH_SAMPLES / 2U - DMR_SYNC_LENGTH_SAMPLES;
                    if (m_startPtr >= DMO_BUFFER_LENGTH_SAMPLES)
                        m_startPtr -= DMO_BUFFER_LENGTH_SAMPLES;

                    m_endPtr = m_dataPtr + DMR_SLOT_TYPE_LENGTH_SAMPLES / 2U + DMR_INFO_LENGTH_SAMPLES / 2U - 1U;
                    if (m_endPtr >= DMO_BUFFER_LENGTH_SAMPLES)
                        m_endPtr -= DMO_BUFFER_LENGTH_SAMPLES;

                    DEBUG5("DMRDMORX: correlateSync(): dataPtr/syncPtr/startPtr/lduEndPtr", m_dataPtr, m_syncPtr, m_startPtr, m_endPtr);
                }
            }
        }
    }
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
void DMRDMORX::samplesToBits(uint16_t start, uint8_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold)
{
    for (uint8_t i = 0U; i < count; i++) {
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

        start += DMR_RADIO_SYMBOL_LENGTH;
        if (start >= DMO_BUFFER_LENGTH_SAMPLES)
            start -= DMO_BUFFER_LENGTH_SAMPLES;
    }
}

/// <summary>
///
/// </summary>
/// <param name="frame"></param>
void DMRDMORX::writeRSSIData(uint8_t* frame)
{
#if defined(SEND_RSSI_DATA)
    // Calculate RSSI average over a burst period. We don't take into account 2.5 ms at the beginning and 2.5 ms at the end
    uint16_t start = m_startPtr + DMR_SYNC_LENGTH_SAMPLES / 2U;

    uint32_t accum = 0U;
    for (uint16_t i = 0U; i < (DMR_FRAME_LENGTH_SAMPLES - DMR_SYNC_LENGTH_SAMPLES); i++) {
        accum += m_rssi[start];

        start++;
        if (start >= DMO_BUFFER_LENGTH_SAMPLES)
            start -= DMO_BUFFER_LENGTH_SAMPLES;
    }

    uint16_t avg = accum / (DMR_FRAME_LENGTH_SAMPLES - DMR_SYNC_LENGTH_SAMPLES);
    frame[34U] = (avg >> 8) & 0xFFU;
    frame[35U] = (avg >> 0) & 0xFFU;

    serial.writeDMRData(true, frame, DMR_FRAME_LENGTH_BYTES + 3U);
#else
    serial.writeDMRData(true, frame, DMR_FRAME_LENGTH_BYTES + 1U);
#endif
}

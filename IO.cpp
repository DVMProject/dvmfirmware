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
*   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2015 by Jim Mclaughlin KI6ZUM
*   Copyright (C) 2016 by Colin Durbridge G4EML
*   Copyright (C) 2017-2018 Bryan Biedenkapp N2PLL
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
#include "IO.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = {
    401, 104, -340, -731, -847, -553, 112, 909, 1472, 1450, 683, -675, -2144, -3040, -2706, -770, 2667, 6995,
    11237, 14331, 15464, 14331, 11237, 6995, 2667, -770, -2706, -3040, -2144, -675, 683, 1450, 1472, 909, 112,
    -553, -847, -731, -340, 104, 401, 0 };
const uint16_t RRC_0_2_FILTER_LEN = 42U;

// One symbol boxcar filter
#if defined(P25_RX_NORMAL_BOXCAR)
static q15_t BOXCAR_5_FILTER[] = { 12000, 12000, 12000, 12000, 12000, 0 };
#endif
#if defined(P25_RX_NARROW_BOXCAR)
static q15_t BOXCAR_5_FILTER[] = { 9600, 9600, 9600, 9600, 9600, 0 };
#endif
const uint16_t BOXCAR_5_FILTER_LEN = 6U;

// Generated using [b, a] = butter(1, 0.001) in MATLAB
static q31_t DC_FILTER[] = { 3367972, 0, 3367972, 0, 2140747704, 0 }; // {b0, 0, b1, b2, -a1, -a2}
const uint32_t DC_FILTER_STAGES = 1U; // One Biquad stage

const uint16_t DC_OFFSET = 2048U;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------
/// <summary>
/// Initializes a new instance of the IO class.
/// </summary>
IO::IO() :
    m_started(false),
    m_rxBuffer(RX_RINGBUFFER_SIZE),
    m_txBuffer(TX_RINGBUFFER_SIZE),
    m_rssiBuffer(RX_RINGBUFFER_SIZE),
    m_rrc_0_2_Filter(),
    m_boxcar_5_Filter(),
    m_dcFilter(),
    m_rrc_0_2_State(),
    m_boxcar_5_State(),
    m_dcState(),
    m_pttInvert(false),
    m_rxLevel(128 * 128),
    m_rxInvert(false),
    m_cwIdTXLevel(128 * 128),
    m_dmrTXLevel(128 * 128),
    m_p25TXLevel(128 * 128),
    m_rxDCOffset(DC_OFFSET),
    m_txDCOffset(DC_OFFSET),
    m_ledCount(0U),
    m_ledValue(true),
    m_detect(false),
    m_adcOverflow(0U),
    m_dacOverflow(0U),
    m_watchdog(0U),
    m_lockout(false)
{
    ::memset(m_rrc_0_2_State, 0x00U, 70U * sizeof(q15_t));
    ::memset(m_boxcar_5_State, 0x00U, 30U * sizeof(q15_t));

    ::memset(m_dcState, 0x00U, 4U * sizeof(q31_t));

    m_rrc_0_2_Filter.numTaps = RRC_0_2_FILTER_LEN;
    m_rrc_0_2_Filter.pState = m_rrc_0_2_State;
    m_rrc_0_2_Filter.pCoeffs = RRC_0_2_FILTER;

    m_boxcar_5_Filter.numTaps = BOXCAR_5_FILTER_LEN;
    m_boxcar_5_Filter.pState = m_boxcar_5_State;
    m_boxcar_5_Filter.pCoeffs = BOXCAR_5_FILTER;

    m_dcFilter.numStages = DC_FILTER_STAGES;
    m_dcFilter.pState = m_dcState;
    m_dcFilter.pCoeffs = DC_FILTER;
    m_dcFilter.postShift = 0;

    initInt();
    selfTest();
}

/// <summary>
/// Starts air interface sampler.
/// </summary>
void IO::start()
{
    if (m_started)
        return;

    startInt();

    m_started = true;

    setMode();
}

/// <summary>
/// Process samples from air interface.
/// </summary>
void IO::process()
{
    m_ledCount++;
    if (m_started) {
        // Two seconds timeout
        if (m_watchdog >= 48000U) {
            if (m_modemState == STATE_DMR) {
                if (m_tx)
                    dmrTX.setStart(false);
                m_modemState = STATE_IDLE;
                setMode();
            }

            m_watchdog = 0U;
        }
        if (m_ledCount >= 48000U) {
            m_ledCount = 0U;
            m_ledValue = !m_ledValue;
            setLEDInt(m_ledValue);
        }
    }
    else {
        if (m_ledCount >= 480000U) {
            m_ledCount = 0U;
            m_ledValue = !m_ledValue;
            setLEDInt(m_ledValue);
        }
        return;
    }

    // use the COS line to lockout the modem
    if (m_cosLockoutEnable) {
        m_lockout = getCOSInt();
    }

    // Switch off the transmitter if needed
    if (m_txBuffer.getData() == 0U && m_tx) {
        m_tx = false;
        setPTTInt(m_pttInvert ? true : false);
    }

    if (m_rxBuffer.getData() >= RX_BLOCK_SIZE) {
        q15_t samples[RX_BLOCK_SIZE];
        uint8_t control[RX_BLOCK_SIZE];
        uint16_t rssi[RX_BLOCK_SIZE];

        for (uint16_t i = 0U; i < RX_BLOCK_SIZE; i++) {
            uint16_t sample;
            m_rxBuffer.get(sample, control[i]);
            m_rssiBuffer.get(rssi[i]);

            // Detect ADC overflow
            if (m_detect && (sample == 0U || sample == 4095U))
                m_adcOverflow++;

            q15_t res1 = q15_t(sample) - m_rxDCOffset;
            q31_t res2 = res1 * m_rxLevel;
            samples[i] = q15_t(__SSAT((res2 >> 15), 16));
        }

        if (m_lockout)
            return;

        q15_t dcSamples[RX_BLOCK_SIZE];
        if (m_dcBlockerEnable) {
            q31_t q31Samples[RX_BLOCK_SIZE];

            ::arm_q15_to_q31(samples, q31Samples, RX_BLOCK_SIZE);

            q31_t dcValues[RX_BLOCK_SIZE];
            ::arm_biquad_cascade_df1_q31(&m_dcFilter, q31Samples, dcValues, RX_BLOCK_SIZE);

            q31_t dcLevel = 0;
            for (uint8_t i = 0U; i < RX_BLOCK_SIZE; i++)
                dcLevel += dcValues[i];
            dcLevel /= RX_BLOCK_SIZE;

            q15_t offset = q15_t(__SSAT((dcLevel >> 16), 16));;

            for (uint8_t i = 0U; i < RX_BLOCK_SIZE; i++)
                dcSamples[i] = samples[i] - offset;
        }

        /** Idle Modem State */
        if (m_modemState == STATE_IDLE) {
            /** Project 25 */
            if (m_p25Enable) {
                q15_t c4fmSamples[RX_BLOCK_SIZE];
                if (m_dcBlockerEnable) {
                    ::arm_fir_fast_q15(&m_boxcar_5_Filter, dcSamples, c4fmSamples, RX_BLOCK_SIZE);
                }
                else {
                    ::arm_fir_fast_q15(&m_boxcar_5_Filter, samples, c4fmSamples, RX_BLOCK_SIZE);
                }

                p25RX.samples(c4fmSamples, rssi, RX_BLOCK_SIZE);
            }

            /** Digital Mobile Radio */
            if (m_dmrEnable) {
                q15_t c4fmSamples[RX_BLOCK_SIZE];
                ::arm_fir_fast_q15(&m_rrc_0_2_Filter, samples, c4fmSamples, RX_BLOCK_SIZE);

                if (m_dmrEnable) {
                    if (m_duplex)
                        dmrIdleRX.samples(c4fmSamples, RX_BLOCK_SIZE);
                    else
                        dmrDMORX.samples(c4fmSamples, rssi, RX_BLOCK_SIZE);
                }
            }
        }
        else if (m_modemState == STATE_DMR) {        // DMR State
            /** Digital Mobile Radio */
            if (m_dmrEnable) {
                q15_t c4fmSamples[RX_BLOCK_SIZE];
                ::arm_fir_fast_q15(&m_rrc_0_2_Filter, samples, c4fmSamples, RX_BLOCK_SIZE);

                if (m_duplex) {
                    // If the transmitter isn't on, use the DMR idle RX to detect the wakeup CSBKs
                    if (m_tx)
                        dmrRX.samples(c4fmSamples, rssi, control, RX_BLOCK_SIZE);
                    else
                        dmrIdleRX.samples(c4fmSamples, RX_BLOCK_SIZE);
                }
                else {
                    dmrDMORX.samples(c4fmSamples, rssi, RX_BLOCK_SIZE);
                }
            }
        }
        else if (m_modemState == STATE_P25) {        // P25 State
            /** Project 25 */
            if (m_p25Enable) {
                q15_t c4fmSamples[RX_BLOCK_SIZE];
                if (m_dcBlockerEnable) {
                    ::arm_fir_fast_q15(&m_boxcar_5_Filter, dcSamples, c4fmSamples, RX_BLOCK_SIZE);
                }
                else {
                    ::arm_fir_fast_q15(&m_boxcar_5_Filter, samples, c4fmSamples, RX_BLOCK_SIZE);
                }

                p25RX.samples(c4fmSamples, rssi, RX_BLOCK_SIZE);
            }
        }
        else if (m_modemState == STATE_RSSI_CAL) {
            calRSSI.samples(rssi, RX_BLOCK_SIZE);
        }
    }
}

/// <summary>
/// Write samples to air interface.
/// </summary>
/// <param name="mode"></param>
/// <param name="samples"></param>
/// <param name="length"></param>
/// <param name="control"></param>
void IO::write(DVM_STATE mode, q15_t* samples, uint16_t length, const uint8_t* control)
{
    if (!m_started)
        return;

    if (m_lockout)
        return;

    // Switch the transmitter on if needed
    if (!m_tx) {
        m_tx = true;
        setPTTInt(m_pttInvert ? false : true);
    }

    q15_t txLevel = 0;
    switch (mode) {
    case STATE_DMR:
        txLevel = m_dmrTXLevel;
        break;
    case STATE_P25:
        txLevel = m_p25TXLevel;
        break;
    default:
        txLevel = m_cwIdTXLevel;
        break;
    }

    for (uint16_t i = 0U; i < length; i++) {
        q31_t res1 = samples[i] * txLevel;
        q15_t res2 = q15_t(__SSAT((res1 >> 15), 16));
        uint16_t res3 = uint16_t(res2 + m_txDCOffset);

        // Detect DAC overflow
        if (res3 > 4095U)
            m_dacOverflow++;

        if (control == NULL)
            m_txBuffer.put(res3, MARK_NONE);
        else
            m_txBuffer.put(res3, control[i]);
    }
}

/// <summary>
/// Helper to get how much space the transmit ring buffer has for samples.
/// </summary>
/// <returns></returns>
uint16_t IO::getSpace() const
{
    return m_txBuffer.getSpace();
}

/// <summary>
///
/// </summary>
/// <param name="dcd"></param>
void IO::setDecode(bool dcd)
{
    if (dcd != m_dcd)
        setCOSInt(dcd ? true : false);

    m_dcd = dcd;
}

/// <summary>
///
/// </summary>
/// <param name="detect"></param>
void IO::setADCDetection(bool detect)
{
    m_detect = detect;
}

/// <summary>
/// Helper to set the modem air interface state.
/// </summary>
void IO::setMode()
{
    setDMRInt(m_modemState == STATE_DMR);
    setP25Int(m_modemState == STATE_P25);
}

/// <summary>
/// Helper to assert or deassert radio PTT.
/// </summary>
void IO::setTransmit()
{
    // Switch the transmitter on if needed
    if (!m_tx) {
        m_tx = true;
        setPTTInt(m_pttInvert ? false : true);
    }
    else {
        m_tx = false;
        setPTTInt(m_pttInvert ? true : false);
    }
}

/// <summary>
/// Sets various air interface parameters.
/// </summary>
/// <param name="rxInvert">Flag indicating the Rx polarity should be inverted.</param>
/// <param name="txInvert">Flag indicating the Tx polarity should be inverted.</param>
/// <param name="pttInvert">Flag indicating the PTT polarity should be inverted.</param>
/// <param name="rxLevel"></param>
/// <param name="cwIdTXLevel"></param>
/// <param name="dmrTXLevel"></param>
/// <param name="p25TXLevel"></param>
/// <param name="txDCOffset"></param>
/// <param name="rxDCOffset"></param>
void IO::setParameters(bool rxInvert, bool txInvert, bool pttInvert, uint8_t rxLevel, uint8_t cwIdTXLevel, uint8_t dmrTXLevel,
                       uint8_t p25TXLevel, uint16_t txDCOffset, uint16_t rxDCOffset)
{
    m_pttInvert = pttInvert;

    m_rxLevel = q15_t(rxLevel * 128);
    m_cwIdTXLevel = q15_t(cwIdTXLevel * 128);
    m_dmrTXLevel = q15_t(dmrTXLevel * 128);
    m_p25TXLevel = q15_t(p25TXLevel * 128);

    m_rxDCOffset = DC_OFFSET + rxDCOffset;
    m_txDCOffset = DC_OFFSET + txDCOffset;

    if (rxInvert) {
        m_rxInvert = rxInvert;
        m_rxLevel = -m_rxLevel;
    }

    if (txInvert) {
        m_dmrTXLevel = -m_dmrTXLevel;
        m_p25TXLevel = -m_p25TXLevel;
    }
}

/// <summary>
/// Sets the software Rx sample level.
/// </summary>
/// <param name="rxLevel"></param>
void IO::setRXLevel(uint8_t rxLevel)
{
    m_rxLevel = q15_t(rxLevel * 128);

    if (m_rxInvert)
        m_rxLevel = -m_rxLevel;
}

/// <summary>
/// Helper to get the state of the ADC and DAC overflow flags.
/// </summary>
/// <param name="adcOverflow"></param>
/// <param name="dacOverflow"></param>
void IO::getOverflow(bool& adcOverflow, bool& dacOverflow)
{
    adcOverflow = m_adcOverflow > 0U;
    dacOverflow = m_dacOverflow > 0U;

    m_adcOverflow = 0U;
    m_dacOverflow = 0U;
}

/// <summary>
/// Flag indicating the TX ring buffer has overflowed.
/// </summary>
/// <returns></returns>
bool IO::hasTXOverflow()
{
    return m_txBuffer.hasOverflowed();
}

/// <summary>
/// Flag indicating the RX ring buffer has overflowed.
/// </summary>
/// <returns></returns>
bool IO::hasRXOverflow()
{
    return m_rxBuffer.hasOverflowed();
}

/// <summary>
/// Flag indicating the air interface is locked out from transmitting.
/// </summary>
/// <returns></returns>
bool IO::hasLockout() const
{
    return m_lockout;
}

/// <summary>
///
/// </summary>
void IO::resetWatchdog()
{
    m_watchdog = 0U;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint32_t IO::getWatchdog()
{
    return m_watchdog;
}

/// <summary>
///
/// </summary>
void IO::selfTest()
{
    bool ledValue = false;

    for (uint8_t i = 0; i < 6; i++) {
        ledValue = !ledValue;

        // We exclude PTT to avoid trigger the transmitter
        setLEDInt(ledValue);
        setCOSInt(ledValue);

        setDMRInt(ledValue);
        setP25Int(ledValue);

        delayInt(250);
    }

    // blinkin lights
    setLEDInt(false);
    setCOSInt(false);
    setDMRInt(false);
    setP25Int(false);
    delayInt(250);

    setLEDInt(true);
    setCOSInt(false);
    setDMRInt(false);
    setP25Int(false);
    delayInt(250);

    setLEDInt(false);
    setCOSInt(true);
    setDMRInt(false);
    setP25Int(false);
    delayInt(250);

    setLEDInt(false);
    setCOSInt(false);
    setDMRInt(true);
    setP25Int(false);
    delayInt(250);

    setLEDInt(false);
    setCOSInt(false);
    setDMRInt(false);
    setP25Int(true);
    delayInt(250);

    setLEDInt(false);
    setCOSInt(false);
    setDMRInt(true);
    setP25Int(false);
    delayInt(250);

    setLEDInt(false);
    setCOSInt(true);
    setDMRInt(false);
    setP25Int(false);
    delayInt(250);

    setLEDInt(true);
    setCOSInt(false);
    setDMRInt(false);
    setP25Int(false);
    delayInt(250);

    setLEDInt(false);
    setCOSInt(false);
    setDMRInt(false);
    setP25Int(false);
    delayInt(250);
}

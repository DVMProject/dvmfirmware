// SPDX-License-Identifier: GPL-2.0-only
/**
* Digital Voice Modem - Modem Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / Modem Firmware
* @derivedfrom MMDVM (https://github.com/g4klx/MMDVM)
* @license GPLv2 License (https://opensource.org/licenses/GPL-2.0)
*
*   Copyright (C) 2018 Andy Uribe, CA6JAU
*   Copyright (C) 2020 Jonathan Naylor, G4KLX
*
*/
#include "Globals.h"
#include "nxdn/CalNXDN.h"

using namespace nxdn;

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

// NXDN 1031 Hz Test Pattern, RAN: 1, Unit ID: 1, Dst Group ID: 1, Outbound Direction
const uint8_t NXDN_CAL1K[4][49] = {
    { 0x00U, 
        0xCDU, 0xF5U, 0x9DU, 0x5DU, 0x7CU, 0xFAU, 0x0AU, 0x6EU, 0x8AU, 0x23U, 0x56U, 0xE8U,
        0x4CU, 0xAAU, 0xDEU, 0x8BU, 0x26U, 0xE4U, 0xF2U, 0x82U, 0x88U,
        0xC6U, 0x8AU, 0x74U, 0x29U, 0xA4U, 0xECU, 0xD0U, 0x08U, 0x22U,
        0xCEU, 0xA2U, 0xFCU, 0x01U, 0x8CU, 0xECU, 0xDAU, 0x0AU, 0xA0U,
        0xEEU, 0x8AU, 0x7EU, 0x2BU, 0x26U, 0xCCU, 0xF8U, 0x8AU, 0x08U },

    { 0x00U,
        0xCDU, 0xF5U, 0x9DU, 0x5DU, 0x7CU, 0x6DU, 0xBBU, 0x0EU, 0xB3U, 0xA4U, 0x26U, 0xA8U,
        0x4CU, 0xAAU, 0xDEU, 0x8BU, 0x26U, 0xE4U, 0xF2U, 0x82U, 0x88U,
        0xC6U, 0x8AU, 0x74U, 0x29U, 0xA4U, 0xECU, 0xD0U, 0x08U, 0x22U,
        0xCEU, 0xA2U, 0xFCU, 0x01U, 0x8CU, 0xECU, 0xDAU, 0x0AU, 0xA0U,
        0xEEU, 0x8AU, 0x7EU, 0x2BU, 0x26U, 0xCCU, 0xF8U, 0x8AU, 0x08U },

    { 0x00U,
        0xCDU, 0xF5U, 0x9DU, 0x5DU, 0x76U, 0x3AU, 0x1BU, 0x4AU, 0x81U, 0xA8U, 0xE2U, 0x80U,
        0x4CU, 0xAAU, 0xDEU, 0x8BU, 0x26U, 0xE4U, 0xF2U, 0x82U, 0x88U,
        0xC6U, 0x8AU, 0x74U, 0x29U, 0xA4U, 0xECU, 0xD0U, 0x08U, 0x22U,
        0xCEU, 0xA2U, 0xFCU, 0x01U, 0x8CU, 0xECU, 0xDAU, 0x0AU, 0xA0U,
        0xEEU, 0x8AU, 0x7EU, 0x2BU, 0x26U, 0xCCU, 0xF8U, 0x8AU, 0x08U },

    { 0x00U,
        0xCDU, 0xF5U, 0x9DU, 0x5DU, 0x74U, 0x28U, 0x83U, 0x02U, 0xB0U, 0x2DU, 0x07U, 0xE2U,
        0x4CU, 0xAAU, 0xDEU, 0x8BU, 0x26U, 0xE4U, 0xF2U, 0x82U, 0x88U,
        0xC6U, 0x8AU, 0x74U, 0x29U, 0xA4U, 0xECU, 0xD0U, 0x08U, 0x22U,
        0xCEU, 0xA2U, 0xFCU, 0x01U, 0x8CU, 0xECU, 0xDAU, 0x0AU, 0xA0U,
        0xEEU, 0x8AU, 0x7EU, 0x2BU, 0x26U, 0xCCU, 0xF8U, 0x8AU, 0x08U }
};

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the CalNXDN class.
/// </summary>
CalNXDN::CalNXDN() :
    m_transmit(false),
    m_state(NXDNCAL1K_IDLE),
    m_audioSeq(0U)
{
    /* stub */
}

/// <summary>
/// Process local state and transmit on the air interface.
/// </summary>
void CalNXDN::process()
{
    if (m_transmit) {
        nxdnTX.setCal(true);
        nxdnTX.process();
    }
    else {
        nxdnTX.setCal(false);
    }

    uint16_t space = nxdnTX.getSpace();
    if (space < 1U)
        return;

    if (m_audioSeq > 3U) {
        m_audioSeq = 0U;
    }

    switch (m_state) {
    case NXDNCAL1K_TX:
        nxdnTX.writeData(NXDN_CAL1K[m_audioSeq], NXDN_FRAME_LENGTH_BYTES + 1U);
        m_audioSeq++;
        if (!m_transmit) {
            m_state = NXDNCAL1K_IDLE;
            m_audioSeq = 0U;
        }
        break;
    default:
        m_state = NXDNCAL1K_IDLE;
        m_audioSeq = 0U;
        break;
    }
}

/// <summary>
/// Write P25 calibration data to the local buffer.
/// </summary>
/// <param name="data"></param>
/// <param name="length"></param>
/// <returns></returns>
uint8_t CalNXDN::write(const uint8_t* data, uint16_t length)
{
    if (length != 1U)
        return RSN_ILLEGAL_LENGTH;

    m_transmit = data[0U] == 1U;

    if(m_transmit && m_state == NXDNCAL1K_IDLE)
        m_state = NXDNCAL1K_TX;

    return RSN_OK;
}

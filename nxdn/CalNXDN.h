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
#if !defined(__CAL_NXDN_H__)
#define __CAL_NXDN_H__

#include "Defines.h"
#include "nxdn/NXDNDefines.h"

namespace nxdn
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    enum NXDNCAL1K {
        NXDNCAL1K_IDLE,
        NXDNCAL1K_TX
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements logic for NXDN calibration mode.
    // ---------------------------------------------------------------------------

    class DSP_FW_API CalNXDN {
    public:
        /// <summary>Initializes a new instance of the CalNXDN class.</summary>
        CalNXDN();

        /// <summary>Process local state and transmit on the air interface.</summary>
        void process();

        /// <summary>Write NXDN calibration state.</summary>
        uint8_t write(const uint8_t* data, uint16_t length);

    private:
        bool m_transmit;
        NXDNCAL1K m_state;
        
        uint8_t m_audioSeq;
    };
} // namespace nxdn

#endif // __CAL_NXDN_H__

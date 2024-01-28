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
*
*/
#if !defined(__CAL_P25_H__)
#define __CAL_P25_H__

#include "Defines.h"
#include "p25/P25Defines.h"

namespace p25
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    enum P25CAL1K {
        P25CAL1K_IDLE,
        P25CAL1K_LDU1,
        P25CAL1K_LDU2
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements logic for P25 calibration mode.
    // ---------------------------------------------------------------------------

    class DSP_FW_API CalP25 {
    public:
        /// <summary>Initializes a new instance of the CalP25 class.</summary>
        CalP25();

        /// <summary>Process local state and transmit on the air interface.</summary>
        void process();

        /// <summary>Write P25 calibration state.</summary>
        uint8_t write(const uint8_t* data, uint8_t length);

    private:
        bool m_transmit;
        P25CAL1K m_state;
    };
} // namespace p25

#endif // __CAL_P25_H__

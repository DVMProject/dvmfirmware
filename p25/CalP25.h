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
*   Copyright (C) 2018 by Andy Uribe CA6JAU
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

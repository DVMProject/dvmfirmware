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
 *   Copyright (C) 2020 by Jonathan Naylor G4KLX
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

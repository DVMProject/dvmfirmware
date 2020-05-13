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
*   Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
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
#if !defined(__DMR_RX_H__)
#define __DMR_RX_H__

#include "Defines.h"
#include "dmr/DMRSlotRX.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for duplex DMR mode operation.
    // ---------------------------------------------------------------------------

    class DSP_FW_API DMRRX {
    public:
        /// <summary>Initializes a new instance of the DMRRX class.</summary>
        DMRRX();

        /// <summary>Helper to reset data values to defaults.</summary>
        void reset();

        /// <summary>Sample DMR values from the air interface.</summary>
        void samples(const q15_t* samples, const uint16_t* rssi, const uint8_t* control, uint8_t length);

        /// <summary></summary>
        void setColorCode(uint8_t colorCode);
        /// <summary></summary>
        void setRxDelay(uint8_t delay);

    private:
        DMRSlotRX m_slot1RX;
        DMRSlotRX m_slot2RX;
    };
} // namespace dmr

#endif // __DMR_RX_H__

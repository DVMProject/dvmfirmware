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
*   Copyright (C) 2016 by Jonathan Naylor G4KLX
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
#if !defined(__CAL_RSSI_H__)
#define __CAL_RSSI_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements logic for RSSI calibration mode.
// ---------------------------------------------------------------------------

class DSP_FW_API CalRSSI {
public:
    /// <summary>Initializes a new instance of the CalRSSI class.</summary>
    CalRSSI();

    /// <summary>Sample RSSI values from the air interface.</summary>
    void samples(const uint16_t* rssi, uint8_t length);

private:
    uint32_t m_count;
    uint32_t m_accum;
    uint16_t m_min;
    uint16_t m_max;
};

#endif // __CAL_RSSI_H__

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
*   Copyright (C) 2009-2015 by Jonathan Naylor G4KLX
*   Copyright (C) 2016 by Colin Durbridge G4EML
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
#if !defined(__CWID_TX_H__)
#define __CWID_TX_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements logic to transmit a CW ID.
// ---------------------------------------------------------------------------

class DSP_FW_API CWIdTX {
public:
    /// <summary>Initializes a new instance of the CWIdTX class.</summary>
    CWIdTX();

    /// <summary>Process local buffer and transmit on the air interface.</summary>
    void process();

    /// <summary>Write CW ID data to the local buffer.</summary>
    uint8_t write(const uint8_t* data, uint8_t length);

    /// <summary>Helper to reset data values to defaults.</summary>
    void reset();

private:
    uint8_t m_poBuffer[1000U];
    uint16_t m_poLen;
    uint16_t m_poPtr;

    uint8_t m_n;
};

#endif // __CWID_TX_H__

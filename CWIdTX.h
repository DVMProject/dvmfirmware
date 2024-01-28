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
*   Copyright (C) 2009-2015 Jonathan Naylor, G4KLX
*   Copyright (C) 2016 Colin Durbridge, G4EML
*
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

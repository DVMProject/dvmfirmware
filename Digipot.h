/**
* Digital Voice Modem - DSP Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / DSP Firmware
*
*/
// Licensed under the GPLv2 License (https://opensource.org/licenses/GPL-2.0)
//
/*
*   Copyright (C) 2023 by Natalie Moore, Patrick McDonnell
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

#if !defined(__DIGIPOT_H__)
#define __DIGIPOT_H__

#if defined(DIGIPOT_ENABLED)

#include "Defines.h"
#include <stm32f4xx_i2c.h>

#define TxPotAddr   0xA2    // 7-Bit address 1010001 (0x51) becomes 0xA2 after << 1
#define RxPotAddr   0xA4    // 7-Bit address 1010010 (0x52) becomes 0xA4 after << 1
//#define RssiPotAddr 0x2E  // RSSI pot is part of TX pot IC

#define TPL0102_REG_WRA     0x00
#define TPL0102_REG_WRB     0x01
#define TPL0102_REG_ACR     0x10

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements logic to set digipot values for audio
// ---------------------------------------------------------------------------

class DSP_FW_API Digipot {
public:
    /// <summary>Initializes a new instance of the Digipot class.</summary>
    Digipot();

    /// <summary>Set Rx fine value on digipot</summary>
    void setRxFine(uint8_t val);

    /// <summary>Set Rx coarse value on digipot</summary>
    void setRxCoarse(uint8_t val);

    /// <summary>Set Tx fine value on digipot</summary>
    void setTxFine(uint8_t val);

    /// <summary>Set Tx coarse value on digipot</summary>
    void setTxCoarse(uint8_t val);

    /// <summary>Set RSSI fine value on digipot</summary>
    void setRssiFine(uint8_t val);

    /// <summary>Set RSSI coarse value on digipot</summary>
    void setRssiCoarse(uint8_t val);

    /// <summary>Helper to initialize all digipots at "middle" value</summary>
    void initialize();

private:
    uint8_t m_RxFine;
    uint8_t m_RxCoarse;
    uint8_t m_TxCoarse;
    uint8_t m_RssiCoarse;
    // helper to set registers
    void setRegister(uint8_t i2c_addr, uint8_t reg_addr, uint8_t reg_value);
};

#endif // DIGIPOT_ENABLED

#endif // __DIGIPOT_H__

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
*   Copyright (C) 2022 by Natalie Moore
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
#include "Globals.h"
#include "Digipot.h"
#include "IO.h"

#if defined(DIGIPOT_ENABLED)
/// <summary>
/// Initializes a new instance of the CWIdTX class.
/// </summary>
Digipot::Digipot() :
    m_RxFine(127U),
    m_RxCoarse(127U),
    m_TxCoarse(127U),
    m_RssiCoarse(127U)
{
    /* stub */
}

void Digipot::initialize() {
    // reset all values to middle
    m_RxFine     = 127U;
    m_RxCoarse   = 127U;
    m_TxCoarse   = 127U;
    m_RssiCoarse = 127U;

    // Set the pot's ACR registers to non-volatile, non-shutdown mode
    setRegister(TxPotAddr, TPL0102_REG_ACR, 0x40);
    setRegister(RxPotAddr, TPL0102_REG_ACR, 0x40);
    
    // push the values to the pots
    setRxFine(m_RxFine);
    setRxCoarse(m_RxCoarse);
    setTxCoarse(m_TxCoarse);
    setRssiCoarse(m_RssiCoarse);
}

void Digipot::setRegister(uint8_t i2c_addr, uint8_t reg_addr, uint8_t reg_value) {
    uint8_t cmd[2] = {reg_addr, reg_value};
    io.I2C_Write(i2c_addr, cmd, 2U);
}

void Digipot::setRxFine(uint8_t val) {
    setRegister(RxPotAddr, TPL0102_REG_WRB, val);
}

void Digipot::setRxCoarse(uint8_t val) {
    setRegister(RxPotAddr, TPL0102_REG_WRA, val);
}

void Digipot::setTxFine(uint8_t val) {
    //there is no TX fine pot so ignore this
    return;
}

void Digipot::setTxCoarse(uint8_t val) {
    setRegister(TxPotAddr, TPL0102_REG_WRA, val);
}

void Digipot::setRssiFine(uint8_t val) {
    //there is no RSSI fine pot so ignore this
    return;
}

void Digipot::setRssiCoarse(uint8_t val) {
    setRegister(TxPotAddr, TPL0102_REG_WRB, val);
}

#endif // DIGIPOT_ENABLED

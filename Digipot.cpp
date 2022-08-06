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
#if DIGIPOT_ENABLED

#include "Globals.h"
#include "Digipot.h"
#include "IO.h"


/// <summary>
/// Initializes a new instance of the CWIdTX class.
/// </summary>
Digipot::Digipot() :
    m_RxFine(127U),
    m_RxCoarse(127U),
    m_TxFine(127U),
    m_TxCoarse(127U),
    m_RssiFine(127U),
    m_RssiCoarse(127U)
{
    /* stub */
}

void Digipot::initialize() {
    // reset all values to middle
    m_RxFine     = 127U;
    m_RxCoarse   = 127U;
    m_TxFine     = 127U;
    m_TxCoarse   = 127U;
    m_RssiFine   = 127U;
    m_RssiCoarse = 127U;
    
    // push the values to the pots
    setPotVal(RxPotAddr, 0, m_RxFine);
    setPotVal(RxPotAddr, 1, m_RxCoarse);
    setPotVal(TxPotAddr, 0, m_TxFine);
    setPotVal(TxPotAddr, 1, m_TxCoarse);
    setPotVal(RssiPotAddr, 0, m_RssiFine);
    setPotVal(RssiPotAddr, 1, m_RssiCoarse);
}

void Digipot::setPotVal(uint8_t addr, uint8_t reg, uint8_t value) {
    uint8_t cmd[2U];
    io.I2C_Write(addr, cmd, 2U);
}

void Digipot::setRxFine(uint8_t val) {
    setPotVal(RxPotAddr, 1, val);
}

void Digipot::setRxCoarse(uint8_t val) {
    setPotVal(RxPotAddr, 0, val);
}

void Digipot::setTxFine(uint8_t val) {
    setPotVal(TxPotAddr, 1, val);
}

void Digipot::setTxCoarse(uint8_t val) {
    setPotVal(TxPotAddr, 0, val);
}

void Digipot::setRssiFine(uint8_t val) {
    setPotVal(RssiPotAddr, 1, val);
}

void Digipot::setRssiCoarse(uint8_t val) {
    setPotVal(RssiPotAddr, 0, val);
}

#endif
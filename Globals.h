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
*   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2017-2021 Bryan Biedenkapp N2PLL
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
#if !defined(__GLOBALS_H__)
#define __GLOBALS_H__

#if defined(STM32F4XX)
#include "stm32f4xx.h"
#include <cstddef>
#else
#include <Arduino.h>
#endif

#include "Defines.h"
#include "SerialPort.h"
#include "dmr/DMRIdleRX.h"
#include "dmr/DMRDMORX.h"
#include "dmr/DMRDMOTX.h"
#include "dmr/DMRRX.h"
#include "dmr/DMRTX.h"
#include "dmr/CalDMR.h"
#include "p25/P25RX.h"
#include "p25/P25TX.h"
#include "p25/CalP25.h"
#include "CalRSSI.h"
#include "CWIdTX.h"
#include "IO.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const uint8_t   MARK_SLOT1 = 0x08U;
const uint8_t   MARK_SLOT2 = 0x04U;
const uint8_t   MARK_NONE = 0x00U;

const uint16_t  RX_BLOCK_SIZE = 2U;

const uint16_t  TX_RINGBUFFER_SIZE = 500U;
const uint16_t  RX_RINGBUFFER_SIZE = 600U;

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

#define  DEBUG1(a)          serial.writeDebug((a))
#define  DEBUG2(a,b)        serial.writeDebug((a),(b))
#define  DEBUG3(a,b,c)      serial.writeDebug((a),(b),(c))
#define  DEBUG4(a,b,c,d)    serial.writeDebug((a),(b),(c),(d))
#define  DEBUG5(a,b,c,d,e)  serial.writeDebug((a),(b),(c),(d),(e))
#define  DEBUG_DUMP(a,b)    serial.writeDump((a),(b))

// ---------------------------------------------------------------------------
//  Global Externs
// ---------------------------------------------------------------------------

extern DVM_STATE m_modemState;

extern bool m_dmrEnable;
extern bool m_p25Enable;

extern bool m_dcBlockerEnable;
extern bool m_cosLockoutEnable;

extern bool m_duplex;

extern bool m_tx;
extern bool m_dcd;

extern SerialPort serial;
extern IO io;

/** DMR BS */
extern dmr::DMRIdleRX dmrIdleRX;
extern dmr::DMRRX dmrRX;
extern dmr::DMRTX dmrTX;

/** DMR MS-DMO */
extern dmr::DMRDMORX dmrDMORX;
extern dmr::DMRDMOTX dmrDMOTX;

/** P25 BS */
extern p25::P25RX p25RX;
extern p25::P25TX p25TX;

/** Calibration */
extern dmr::CalDMR calDMR;
extern p25::CalP25 calP25;
extern CalRSSI calRSSI;

/** CW */
extern CWIdTX cwIdTX;

#endif // __GLOBALS_H__

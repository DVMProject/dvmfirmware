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
*   Copyright (C) 2018 Bryan Biedenkapp N2PLL
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
#if !defined(__DEFINES_H__)
#define __DEFINES_H__

#include <stdint.h>

#if !defined(NATIVE_SDR)
#if defined(__SAM3X8E__) && !defined(STM32F4XX)
#define  ARM_MATH_CM3
#elif defined(STM32F4XX)
#define  ARM_MATH_CM4
#else
#error "Unknown processor type"
#endif
#else
#include <cstring>
#endif

#include <arm_math.h>

// ---------------------------------------------------------------------------
//  Types
// ---------------------------------------------------------------------------

#ifndef _INT8_T_DECLARED
#ifndef __INT8_TYPE__
typedef signed char         int8_t;
#endif // __INT8_TYPE__
#endif // _INT8_T_DECLARED
#ifndef _INT16_T_DECLARED
#ifndef __INT16_TYPE__
typedef short               int16_t;
#endif // __INT16_TYPE__
#endif // _INT16_T_DECLARED
#ifndef _INT32_T_DECLARED
#ifndef __INT32_TYPE__
typedef int                 int32_t;
#endif // __INT32_TYPE__
#endif // _INT32_T_DECLARED
#ifndef _INT64_T_DECLARED
#ifndef __INT64_TYPE__
typedef long long           int64_t;
#endif // __INT64_TYPE__
#endif // _INT64_T_DECLARED
#ifndef _UINT8_T_DECLARED
#ifndef __UINT8_TYPE__
typedef unsigned char       uint8_t;
#endif // __UINT8_TYPE__
#endif // _UINT8_T_DECLARED
#ifndef _UINT16_T_DECLARED
#ifndef __UINT16_TYPE__
typedef unsigned short      uint16_t;
#endif // __UINT16_TYPE__
#endif // _UINT16_T_DECLARED
#ifndef _UINT32_T_DECLARED
#ifndef __UINT32_TYPE__
typedef unsigned int        uint32_t;
#endif // __UINT32_TYPE__
#endif // _UINT32_T_DECLARED
#ifndef _UINT64_T_DECLARED
#ifndef __UINT64_TYPE__
typedef unsigned long long  uint64_t;
#endif // __UINT64_TYPE__
#endif // _UINT64_T_DECLARED

#ifndef __LONG64_TYPE__
typedef long long           long64_t;
#endif // __LONG64_TYPE__
#ifndef __ULONG64_TYPE__
typedef unsigned long long  ulong64_t;
#endif // __ULONG64_TYPE__

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#define __PROG_NAME__ "Digital Voice Modem DSP"
#define __NET_NAME__ "DVM_DMR_P25"
#define __EXE_NAME__ "dvm-firmware"
#define __BUILD__ __DATE__ " " __TIME__

#define DSP_FW_API 

// Normal Boxcar Filter for P25
//#define P25_RX_NORMAL_BOXCAR

// Narrow Boxcar Filter for P25
#define P25_RX_NARROW_BOXCAR

// Boxcar Filter for NXDN
//#define NXDN_BOXCAR_FILTER

// Alternate P25 Symbol Levels
//#define P25_ALTERNATE_SYM_LEVELS

// Allow for the use of high quality external clock oscillators
// The number is the frequency of the oscillator in Hertz.
//
// The frequency of the TCXO must be an integer multiple of 48000.
// Frequencies such as 12.0 Mhz (48000 * 250) and 14.4 Mhz (48000 * 300) are suitable.
// Frequencies such as 10.0 Mhz (48000 * 208.333) or 20 Mhz (48000 * 416.666) are not suitable.
//
// For 12 MHz
#ifndef EXTERNAL_OSC
#define EXTERNAL_OSC 12000000
#endif
// For 12.288 MHz
// #define EXTERNAL_OSC 12288000
// For 14.4 MHz
// #define EXTERNAL_OSC 14400000
// For 19.2 MHz
// #define EXTERNAL_OSC 19200000

// Sanity check to make sure EXTERNAL_OSC is a valid integer multiple of 48kHz
#if (EXTERNAL_OSC % 48000 != 0)
#error "Invalid EXTERNAL_OSC specified! Must be an integer multiple of 48000"
#endif

// Pass RSSI information to the host
// #define SEND_RSSI_DATA

#define DESCR_DMR        "DMR, "
#define DESCR_P25        "P25, "
#define DESCR_NXDN       "NXDN, "

#if defined(EXTERNAL_OSC)
#define DESCR_OSC        "TCXO, "
#else
#define DESCR_OSC        ""
#endif

#if defined(SEND_RSSI_DATA)
#define DESCR_RSSI        "RSSI, "
#else
#define DESCR_RSSI        ""
#endif

#define DESCRIPTION        __PROG_NAME__ " (" DESCR_DMR DESCR_P25 DESCR_NXDN DESCR_OSC DESCR_RSSI "CW Id)"

const uint8_t BIT_MASK_TABLE[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define CPU_TYPE_ARDUINO_DUE 0x00U
#define CPU_TYPE_NXP 0x01U
#define CPU_TYPE_STM32 0x02U

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

#define _WRITE_BIT(p, i, b) p[(i) >> 3] = (b) ? (p[(i) >> 3] | BIT_MASK_TABLE[(i) & 7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i) & 7])
#define _READ_BIT(p, i)     (p[(i) >> 3] & BIT_MASK_TABLE[(i) & 7])

#endif // __DEFINES_H__

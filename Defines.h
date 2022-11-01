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

#if !defined(NATIVE_SDR)
#include <arm_math.h>
#else
#include "sdr/arm_math.h"
#endif

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

// Allow the DMR protocol
#define ENABLE_DMR

// Allow the P25 protocol
#define ENABLE_P25

// Allow the NXDN protocol
#define ENABLE_NXDN

// Normal Boxcar Filter for P25
//#define P25_RX_NORMAL_BOXCAR

// Narrow Boxcar Filter for P25
#define P25_RX_NARROW_BOXCAR

// Boxcar Filter for NXDN
//#define NXDN_BOXCAR_FILTER

// Allow for the use of high quality external clock oscillators
// The number is the frequency of the oscillator in Hertz.
//
// The frequency of the TCXO must be an integer multiple of 48000.
// Frequencies such as 12.0 Mhz (48000 * 250) and 14.4 Mhz (48000 * 300) are suitable.
// Frequencies such as 10.0 Mhz (48000 * 208.333) or 20 Mhz (48000 * 416.666) are not suitable.
//
// For 12 MHz
#define EXTERNAL_OSC 12000000
// For 12.288 MHz
// #define EXTERNAL_OSC 12288000
// For 14.4 MHz
// #define EXTERNAL_OSC 14400000
// For 19.2 MHz
// #define EXTERNAL_OSC 19200000

// Pass RSSI information to the host
// #define SEND_RSSI_DATA

#if defined(ENABLE_DMR)
#define DESCR_DMR        "DMR, "
#else
#define DESCR_DMR        ""
#endif
#if defined(ENABLE_P25)
#define DESCR_P25        "P25, "
#else
#define DESCR_P25        ""
#endif
#if defined(ENABLE_NXDN)
#define DESCR_NXDN       "NXDN, "
#else
#define DESCR_NXDN       ""
#endif

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
#define CPU_TYPE_NATIVE_SDR 0xF0U

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

#define _WRITE_BIT(p, i, b) p[(i) >> 3] = (b) ? (p[(i) >> 3] | BIT_MASK_TABLE[(i) & 7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i) & 7])
#define _READ_BIT(p, i)     (p[(i) >> 3] & BIT_MASK_TABLE[(i) & 7])

#endif // __DEFINES_H__

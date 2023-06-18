/**
* Digital Voice Modem - DSP Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / DSP Firmware
*
*/
//
// Based on code from Mark Carter (https://mcturra2000.wordpress.com/2022/03/10/sending-data-over-i2c-using-bit-banging/)
// And also https://github.com/PascalPolygon/stm32_bitbang_i2c/
// Licensed under the GPLv2 License (https://opensource.org/licenses/GPL-2.0)
//
/*
*   Copyright (C) 2023 by Patrick McDonnell, W3AXL
*
*   This library is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Library General Public
*   License as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
*
*   This library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Library General Public License for more details.
*
*   You should have received a copy of the GNU Library General Public
*   License along with this library; if not, write to the
*   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
*   Boston, MA  02110-1301, USA.
*
*/

#ifndef __DWT_DELAY_H__
#define __DWT_DELAY_H__

#if (!defined(__SAM3X8E__) && !defined(__MK20DX256__) && !defined(__MK64FX512__) && !defined(__MK66FX1M0__)) && !defined(ARDUINO_SAM_DUE)

#include "stm32f4xx_rcc.h"

uint32_t DWT_Delay_Init(void);
void DWT_Delay_us(volatile uint32_t microseconds);

#endif

#endif
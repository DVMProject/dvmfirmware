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
*   Copyright (C) 2015,2016 by Jonathan Naylor G4KLX
*   Serial FIFO Control Copyright (C) 2015 by James McLaughlin KI6ZUM
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
#if !defined(__SERIAL_RB_H__)
#define __SERIAL_RB_H__

#if defined(STM32F4XX)
#include "stm32f4xx.h"
#include <cstddef>
#else
#include <Arduino.h>
#endif

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const uint16_t SERIAL_RINGBUFFER_SIZE = 396U;

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements a circular buffer for serial data.
// ---------------------------------------------------------------------------

class DSP_FW_API SerialBuffer {
public:
    /// <summary>Initializes a new instance of the SerialBuffer class.</summary>
    SerialBuffer(uint16_t length = SERIAL_RINGBUFFER_SIZE);
    /// <summary>Finalizes a instance of the SerialBuffer class.</summary>
    ~SerialBuffer();

    /// <summary>Helper to get how much space the ring buffer has for samples.</summary>
    uint16_t getSpace() const;

    /// <summary></summary>
    uint16_t getData() const;

    /// <summary>Helper to reset data values to defaults.</summary>
    void reset();
    /// <summary>Helper to reset and reinitialize data values to defaults.</summary>
    void reinitialize(uint16_t length);

    /// <summary></summary>
    bool put(uint8_t c);

    /// <summary></summary>
    uint8_t peek() const;

    /// <summary></summary>
    uint8_t get();

private:
    uint16_t m_length;
    volatile uint8_t* m_buffer;

    volatile uint16_t m_head;
    volatile uint16_t m_tail;

    volatile bool m_full;
};

#endif // __SERIAL_RB_H__

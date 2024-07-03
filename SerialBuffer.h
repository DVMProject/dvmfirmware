// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * @package DVM / Modem Firmware
 * @derivedfrom MMDVM (https://github.com/g4klx/MMDVM)
 * @license GPLv2 License (https://opensource.org/licenses/GPL-2.0)
 *
 *  Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
 *  Serial FIFO Control Copyright (C) 2015 by James McLaughlin, KI6ZUM
 *
 */
/**
 * @file SerialBuffer.h
 * @ingroup modem_fw
 * @file SerialBuffer.cpp
 * @ingroup modem_fw
 */
#if !defined(__SERIAL_RB_H__)
#define __SERIAL_RB_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const uint16_t SERIAL_RINGBUFFER_SIZE = 396U;

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Implements a circular ring buffer for serial data.
 * @ingroup modem_fw
 */
class DSP_FW_API SerialBuffer {
public:
    /**
     * @brief Initializes a new instance of the SerialBuffer class.
     * @param length Length of buffer.
     */
    SerialBuffer(uint16_t length = SERIAL_RINGBUFFER_SIZE);
    /**
     * @brief Finalizes a instance of the SerialBuffer class.
     */
    ~SerialBuffer();

    /**
     * @brief Helper to get how much space the ring buffer has for data.
     * @returns uint16_t Amount of space remaining for data.
     */
    uint16_t getSpace() const;

    /**
     * @brief 
     * @returns uint16_t 
     */
    uint16_t getData() const;

    /**
     * @brief Helper to reset data values to defaults.
     */
    void reset();
    /**
     * @brief Helper to reset and reinitialize data values to defaults.
     * @param length Length of buffer.
     */
    void reinitialize(uint16_t length);

    /**
     * @brief 
     * @param c 
     * @returns bool 
     */
    bool put(uint8_t c);

    /**
     * @brief 
     * @returns uint8_t 
     */
    uint8_t peek() const;

    /**
     * @brief 
     * @returns uint8_t 
     */
    uint8_t get();

private:
    uint16_t m_length;
    volatile uint8_t* m_buffer;

    volatile uint16_t m_head;
    volatile uint16_t m_tail;

    volatile bool m_full;
};

#endif // __SERIAL_RB_H__

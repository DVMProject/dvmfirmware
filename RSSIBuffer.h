// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
 *  Serial FIFO Control Copyright (C) 2015 by James McLaughlin, KI6ZUM
 *
 */
/**
 * @file RSSIBuffer.h
 * @ingroup modem_fw
 * @file RSSIBuffer.cpp
 * @ingroup modem_fw
 */
#if !defined(__RSSI_RB_H__)
#define __RSSI_RB_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Implements a circular buffer for RSSI data.
 * @ingroup modem_fw
 */
class DSP_FW_API RSSIBuffer {
public:
    /**
     * @brief Initializes a new instance of the RSSIBuffer class.
     * @param length Length of buffer.
     */
    RSSIBuffer(uint16_t length);

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
     * @brief 
     * @param rssi 
     * @returns bool 
     */
    bool put(uint16_t rssi);

    /**
     * @brief 
     * @param[out] rssi
     * @returns bool 
     */
    bool get(uint16_t& rssi);

    /**
     * @brief Flag indicating whether or not the ring buffer has overflowed.
     * @returns bool Flag indicating whether or not the ring buffer has overflowed.
     */
    bool hasOverflowed();

private:
    uint16_t m_length;
    volatile uint16_t* m_rssi;

    volatile uint16_t m_head;
    volatile uint16_t m_tail;

    volatile bool m_full;

    bool m_overflow;
};

#endif // __RSSI_RB_H__

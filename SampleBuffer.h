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
 * @file SampleBuffer.h
 * @ingroup modem_fw
 * @file SampleBuffer.cpp
 * @ingroup modem_fw
 */
#if !defined(__SAMPLE_RB_H__)
#define __SAMPLE_RB_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Implements a circular ring buffer for sample data.
 * @ingroup modem_fw
 */
class DSP_FW_API SampleBuffer {
public:
    /**
     * @brief Initializes a new instance of the SampleBuffer class.
     * @param length Length of buffer.
     */
    SampleBuffer(uint16_t length);

    /**
     * @brief Helper to get how much space the ring buffer has for samples.
     * @returns uint16_t Amount of space remaining for samples.
     */
    uint16_t getSpace() const;

    /**
     * @brief 
     * @returns uint16_t 
     */
    uint16_t getData() const;

    /**
     * @brief 
     * @param sample 
     * @param control
     * @returns bool 
     */
    bool put(uint16_t sample, uint8_t control);

    /**
     * @brief 
     * @param[out] sample
     * @param[out] control
     * @returns bool 
     */
    bool get(uint16_t& sample, uint8_t& control);

    /**
     * @brief Flag indicating whether or not the ring buffer has overflowed.
     * @returns bool Flag indicating whether or not the ring buffer has overflowed.
     */
    bool hasOverflowed();

private:
    uint16_t m_length;
    volatile uint16_t* m_samples;
    volatile uint8_t* m_control;

    volatile uint16_t m_head;
    volatile uint16_t m_tail;

    volatile bool m_full;

    bool m_overflow;
};

#endif // __SAMPLE_RB_H__

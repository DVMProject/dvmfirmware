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
#include "SampleBuffer.h"

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the SampleBuffer class. */

SampleBuffer::SampleBuffer(uint16_t length) :
    m_length(length),
    m_samples(NULL),
    m_control(NULL),
    m_head(0U),
    m_tail(0U),
    m_full(false),
    m_overflow(false)
{
    m_samples = new uint16_t[length];
    m_control = new uint8_t[length];
}

/* Helper to get how much space the ring buffer has for samples. */

uint16_t SampleBuffer::getSpace() const
{
    uint16_t n = 0U;

    if (m_tail == m_head)
        n = m_full ? 0U : m_length;
    else if (m_tail < m_head)
        n = m_length - m_head + m_tail;
    else
        n = m_tail - m_head;

    if (n > m_length)
        n = 0U;

    return n;
}

/* */

uint16_t SampleBuffer::getData() const
{
    if (m_tail == m_head)
        return m_full ? m_length : 0U;
    else if (m_tail < m_head)
        return m_head - m_tail;
    else
        return m_length - m_tail + m_head;
}

/* */

bool SampleBuffer::put(uint16_t sample, uint8_t control)
{
    if (m_full) {
        m_overflow = true;
        return false;
    }

    m_samples[m_head] = sample;
    m_control[m_head] = control;

    m_head++;
    if (m_head >= m_length)
        m_head = 0U;

    if (m_head == m_tail)
        m_full = true;

    return true;
}

/* */

bool SampleBuffer::get(uint16_t& sample, uint8_t& control)
{
    if (m_head == m_tail && !m_full)
        return false;

    sample = m_samples[m_tail];
    control = m_control[m_tail];

    m_full = false;

    m_tail++;
    if (m_tail >= m_length)
        m_tail = 0U;

    return true;
}

/* Flag indicating whether or not the ring buffer has overflowed. */

bool SampleBuffer::hasOverflowed()
{
    bool overflow = m_overflow;

    m_overflow = false;

    return overflow;
}

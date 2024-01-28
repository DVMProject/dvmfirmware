// SPDX-License-Identifier: GPL-2.0-only
/**
* Digital Voice Modem - Modem Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / Modem Firmware
* @derivedfrom MMDVM (https://github.com/g4klx/MMDVM)
* @license GPLv2 License (https://opensource.org/licenses/GPL-2.0)
*
*   Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
*   Serial FIFO Control Copyright (C) 2015 by James McLaughlin, KI6ZUM
*
*/
#include "SerialBuffer.h"

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/// <summary>
/// Initializes a new instance of the SerialBuffer class.
/// </summary>
SerialBuffer::SerialBuffer(uint16_t length) :
    m_length(length),
    m_buffer(NULL),
    m_head(0U),
    m_tail(0U),
    m_full(false)
{
    m_buffer = new uint8_t[length];
}

/// <summary>
/// Finalizes a instance of the SerialBuffer class.
/// </summary>
SerialBuffer::~SerialBuffer()
{
    delete[] m_buffer;
}

/// <summary>
/// Helper to get how much space the ring buffer has for samples.
/// </summary>
/// <returns></returns>
uint16_t SerialBuffer::getSpace() const
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

/// <summary>
///
/// </summary>
/// <returns></returns>
uint16_t SerialBuffer::getData() const
{
    if (m_tail == m_head)
        return m_full ? m_length : 0U;
    else if (m_tail < m_head)
        return m_head - m_tail;
    else
        return m_length - m_tail + m_head;
}

/// <summary>
/// Helper to reset data values to defaults.
/// </summary>
void SerialBuffer::reset()
{
    m_head = 0U;
    m_tail = 0U;
    m_full = false;
}

/// <summary>
/// Helper to reset and reinitialize data values to defaults.
/// </summary>
void SerialBuffer::reinitialize(uint16_t length)
{
    reset();

    m_length = length;

    delete[] m_buffer;
    m_buffer = new uint8_t[length];
}

/// <summary>
///
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
bool SerialBuffer::put(uint8_t c)
{
    if (m_full)
        return false;

    m_buffer[m_head] = c;

    m_head++;
    if (m_head >= m_length)
        m_head = 0U;

    if (m_head == m_tail)
        m_full = true;

    return true;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint8_t SerialBuffer::peek() const
{
    return m_buffer[m_tail];
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint8_t SerialBuffer::get()
{
    uint8_t value = m_buffer[m_tail];

    m_full = false;

    m_tail++;
    if (m_tail >= m_length)
        m_tail = 0U;

    return value;
}

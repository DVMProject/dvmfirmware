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

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
#if !defined(__SAMPLE_RB_H__)
#define __SAMPLE_RB_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements a circular buffer for sample data.
// ---------------------------------------------------------------------------

class DSP_FW_API SampleBuffer {
public:
    /// <summary>Initializes a new instance of the SampleBuffer class.</summary>
    SampleBuffer(uint16_t length);

    /// <summary>Helper to get how much space the ring buffer has for samples.</summary>
    uint16_t getSpace() const;

    /// <summary></summary>
    uint16_t getData() const;

    /// <summary></summary>
    bool put(uint16_t sample, uint8_t control);

    /// <summary></summary>
    bool get(uint16_t& sample, uint8_t& control);

    /// <summary>Flag indicating whether or not the ring buffer has overflowed.</summary>
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

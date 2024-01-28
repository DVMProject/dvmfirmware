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
*   Copyright (C) 2017 Wojciech Krutnik, N0CALL
*
*/
#if !defined(__RING_BUFFER_H__)
#define __RING_BUFFER_H__

/*
*   FIFO ring buffer source:
*   http://stackoverflow.com/questions/6822548/correct-way-of-implementing-a-uart-receive-buffer-in-a-small-arm-microcontroller (modified)
*
*/

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

#define RINGBUFF_SIZE(ringBuff) (ringBuff.size)    // serial buffer in bytes (power 2)
#define RINGBUFF_MASK(ringBuff) (ringBuff.size-1U) // buffer size mask

// Buffer read / write macros
#define RINGBUFF_RESET(ringBuff)         (ringBuff).rdIdx = ringBuff.wrIdx = 0
#define RINGBUFF_WRITE(ringBuff, dataIn) (ringBuff).data[RINGBUFF_MASK(ringBuff) & ringBuff.wrIdx++] = (dataIn)
#define RINGBUFF_READ(ringBuff)          ((ringBuff).data[RINGBUFF_MASK(ringBuff) & ((ringBuff).rdIdx++)])
#define RINGBUFF_EMPTY(ringBuff)         ((ringBuff).rdIdx == (ringBuff).wrIdx)
#define RINGBUFF_FULL(ringBuff)          ((RINGBUFF_MASK(ringBuff) & ringFifo.rdIdx) == (RINGBUFF_MASK(ringBuff) & ringFifo.wrIdx))
#define RINGBUFF_COUNT(ringBuff)         (RINGBUFF_MASK(ringBuff) & ((ringBuff).wrIdx - (ringBuff).rdIdx))

// Buffer type
#define DECLARE_RINGBUFFER_TYPE(name, _size)       \
    typedef struct {                               \
        uint32_t   size;                           \
        uint32_t   wrIdx;                          \
        uint32_t   rdIdx;                          \
        uint8_t    data[_size];                    \
    } name##_t

#endif // __RING_BUFFER_H__

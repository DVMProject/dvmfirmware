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
*   Copyright (C) 2015 Jonathan Naylor, G4KLX
*
*/
#if !defined(__DMR_IDLE_RX_H__)
#define __DMR_IDLE_RX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for idle DMR mode operation.
    // ---------------------------------------------------------------------------

    class DSP_FW_API DMRIdleRX {
    public:
        /// <summary>Initializes a new instance of the DMRIdleRX class.</summary>
        DMRIdleRX();

        /// <summary>Helper to reset data values to defaults.</summary>
        void reset();

        /// <summary>Sample DMR values from the air interface.</summary>
        void samples(const q15_t* samples, uint8_t length);

        /// <summary>Sets the DMR color code.</summary>
        void setColorCode(uint8_t colorCode);

    private:
        uint32_t m_bitBuffer[DMR_RADIO_SYMBOL_LENGTH];
        q15_t m_buffer[DMR_FRAME_LENGTH_SAMPLES];
        uint16_t m_bitPtr;
        uint16_t m_dataPtr;
        uint16_t m_endPtr;

        q31_t m_maxCorr;
        q15_t m_centre;
        q15_t m_threshold;

        uint8_t m_colorCode;

        /// <summary>Helper to perform sample processing.</summary>
        void processSample(q15_t sample);

        /// <summary></summary>
        void samplesToBits(uint16_t start, uint8_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);
    };
} // namespace dmr

#endif // __DMR_IDLE_RX_H__

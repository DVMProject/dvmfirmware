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
 *  Copyright (C) 2015 Jonathan Naylor, G4KLX
 *
 */
/**
 * @file DMRIdleRX.h
 * @ingroup dmr_mfw
 * @file DMRIdleRX.cpp
 * @ingroup dmr_mfw
 */
#if !defined(__DMR_IDLE_RX_H__)
#define __DMR_IDLE_RX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements receiver logic for idle DMR mode operation.
     * @ingroup dmr_mfw
     */
    class DSP_FW_API DMRIdleRX {
    public:
        /**
         * @brief Initializes a new instance of the DMRIdleRX class.
         */
        DMRIdleRX();

        /**
         * @brief Helper to reset data values to defaults.
         */
        void reset();

        /**
         * @brief Sample DMR values from the air interface.
         * @param[in] samples 
         * @param length 
         */
        void samples(const q15_t* samples, uint8_t length);

        /**
         * @brief Sets the DMR color code.
         * @param colorCode 
         */
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

        /**
         * @brief Helper to perform sample processing.
         * @param sample 
         */
        void processSample(q15_t sample);

        /**
         * @brief 
         * @param start 
         * @param count
         * @param buffer
         * @param offset
         * @param centre
         * @param threshold 
         */
        void samplesToBits(uint16_t start, uint8_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);
    };
} // namespace dmr

#endif // __DMR_IDLE_RX_H__

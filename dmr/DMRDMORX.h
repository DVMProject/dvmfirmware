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
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *
 */
/**
 * @file DMRDMORX.h
 * @ingroup dmr_mfw
 * @file DMRDMORX.h
 * @ingroup dmr_mfw
 */
#if !defined(__DMR_DMO_RX_H__)
#define __DMR_DMO_RX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    const uint16_t DMO_BUFFER_LENGTH_SAMPLES = 1440U;   // 60ms at 24 kHz

    /**
     * @brief DMR DMO Receiver State
     */
    enum DMORX_STATE {
        DMORXS_NONE,        //! None
        DMORXS_VOICE,       //! Voice Data
        DMORXS_DATA         //! PDU Data
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements receiver logic for DMR DMO mode operation.
     * @ingroup dmr_mfw
     */
    class DSP_FW_API DMRDMORX {
    public:
        /**
         * @brief Initializes a new instance of the DMRDMORX class.
         */
        DMRDMORX();

        /**
         * @brief Helper to reset data values to defaults.
         */
        void reset();

        /**
         * @brief Sample DMR values from the air interface.
         * @param[in] samples 
         * @param[in] rssi
         * @param length 
         */
        void samples(const q15_t* samples, const uint16_t* rssi, uint8_t length);

        /**
         * @brief Sets the DMR color code.
         * @param colorCode 
         */
        void setColorCode(uint8_t colorCode);

    private:
        uint32_t m_bitBuffer[DMR_RADIO_SYMBOL_LENGTH];
        q15_t m_buffer[DMO_BUFFER_LENGTH_SAMPLES];

        uint16_t m_bitPtr;
        uint16_t m_dataPtr;
        uint16_t m_syncPtr;
        uint16_t m_startPtr;
        uint16_t m_endPtr;

        q31_t m_maxCorr;
        q15_t m_centre[4U];
        q15_t m_threshold[4U];
        uint8_t m_averagePtr;

        uint8_t m_control;
        uint8_t m_syncCount;

        uint8_t m_colorCode;

        DMORX_STATE m_state;

        uint8_t m_n;

        uint8_t m_type;

        uint16_t m_rssi[DMO_BUFFER_LENGTH_SAMPLES];

        /**
         * @brief Helper to perform sample processing.
         * @param sample 
         * @param rssi 
         */
        bool processSample(q15_t sample, uint16_t rssi);

        /**
         * @brief Frame synchronization correlator.
         * @param first 
         */
        void correlateSync(bool first);

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
        /**
         * @brief 
         * @param frame 
         */
        void writeRSSIData(uint8_t* frame);
    };
} // namespace dmr

#endif // __DMR_DMO_RX_H__

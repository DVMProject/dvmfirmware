// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017,2018,2020 Jonathan Naylor, G4KLX
 *
 */
/**
 * @file NXDNRX.h
 * @ingroup nxdn_mfw
 * @file NXDNRX.cpp
 * @ingroup nxdn_mfw
 */
#if !defined(__NXDN_RX_H__)
#define __NXDN_RX_H__

#include "Defines.h"
#include "nxdn/NXDNDefines.h"

namespace nxdn
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    /**
     * @brief NXDN Receiver State
     * @ingroup nxdn_mfw
     */
    enum NXDNRX_STATE {
        NXDNRXS_NONE,       //! None
        NXDNRXS_DATA        //! Data
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements receiver logic for NXDN mode operation.
     * @ingroup nxdn_mfw
     */
    class DSP_FW_API NXDNRX {
    public:
        /**
         * @brief Initializes a new instance of the NXDNRX class.
         */
        NXDNRX();

        /**
         * @brief Helper to reset data values to defaults.
         */
        void reset();

        /**
         * @brief Sample NXDN values from the air interface.
         * @param[in] samples 
         * @param rssi
         * @param length 
         */
        void samples(const q15_t* samples, uint16_t* rssi, uint8_t length);

        /**
         * @brief Sets the NXDN sync correlation countdown.
         * @param count Sync Correlation Countdown.
         */
        void setCorrCount(uint8_t count);

    private:
        uint16_t m_bitBuffer[NXDN_RADIO_SYMBOL_LENGTH];
        q15_t m_buffer[NXDN_FRAME_LENGTH_SAMPLES];

        uint16_t m_bitPtr;
        uint16_t m_dataPtr;

        uint16_t m_startPtr;
        uint16_t m_endPtr;

        uint16_t m_fswPtr;
        uint16_t m_minFSWPtr;
        uint16_t m_maxFSWPtr;

        q31_t m_maxCorr;
        q15_t m_centre[16U];
        q15_t m_centreVal;
        q15_t m_threshold[16U];
        q15_t m_thresholdVal;
        uint8_t m_averagePtr;

        uint16_t m_lostCount;
        uint8_t m_countdown;

        uint8_t m_corrCountdown;

        NXDNRX_STATE m_state;

        uint32_t m_rssiAccum;
        uint16_t m_rssiCount;

        /**
         * @brief Helper to process NXDN samples.
         * @param sample 
         */
        void processSample(q15_t sample);
        /**
         * @brief Helper to process NXDN data samples.
         * @param sample 
         */
        void processData(q15_t sample);

        /**
         * @brief Frame synchronization correlator.
         * @returns bool 
         */
        bool correlateSync();

        /**
         * @brief 
         * @param start 
         * @param count 
         */
        void calculateLevels(uint16_t start, uint16_t count);
        /**
         * @brief 
         * @param start 
         * @param count
         * @param buffer
         * @param offset
         * @param centre
         * @param threshold 
         */
        void samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);
    };
} // namespace nxdn

#endif // __NXDN_RX_H__

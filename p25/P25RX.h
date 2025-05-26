// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2017-2025 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file P25RX.h
 * @ingroup p25_mfw
 * @file P25RX.cpp
 * @ingroup p25_mfw
 */
#if !defined(__P25_RX_H__)
#define __P25_RX_H__

#include "Defines.h"
#include "p25/P25Defines.h"

namespace p25
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    /**
     * @brief P25 Receiver State
     * @ingroup p25_mfw
     */
    enum P25RX_STATE {
        P25RXS_NONE,        //! None
        P25RXS_SYNC,        //! Found Sync
        P25RXS_VOICE,       //! Voice Data
        P25RXS_DATA         //! PDU Data
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements receiver logic for P25 mode operation.
     * @ingroup p25_mfw
     */
    class DSP_FW_API P25RX {
    public:
        /**
         * @brief Initializes a new instance of the P25RX class.
         */
        P25RX();

        /**
         * @brief Helper to reset data values to defaults.
         */
        void reset();

        /**
         * @brief Sample P25 values from the air interface.
         * @param[in] samples 
         * @param rssi
         * @param length 
         */
        void samples(const q15_t* samples, uint16_t* rssi, uint8_t length);

        /**
         * @brief Sets the P25 NAC.
         * @param nac Network Access Code.
         */
        void setNAC(uint16_t nac);
        /**
         * @brief Sets the P25 sync correlation countdown.
         * @param count Sync Correlation Countdown.
         */
        void setCorrCount(uint8_t count);

    private:
        uint32_t m_bitBuffer[P25_RADIO_SYMBOL_LENGTH];
        q15_t m_buffer[P25_PDU_FRAME_LENGTH_SAMPLES];

        uint16_t m_bitPtr;
        uint16_t m_dataPtr;

        uint16_t m_minSyncPtr;
        uint16_t m_maxSyncPtr;

        uint16_t m_startPtr;
        uint16_t m_endPtr;
        uint16_t m_pduEndPtr;
        uint16_t m_syncPtr;

        q31_t m_maxCorr;
        q15_t m_centre[16U];
        q15_t m_centreVal;
        q15_t m_threshold[16U];
        q15_t m_thresholdVal;
        uint8_t m_averagePtr;

        uint16_t m_lostCount;
        uint8_t m_countdown;

        uint16_t m_nac;

        uint8_t m_corrCountdown;

        P25RX_STATE m_state;
        bool m_lduSyncPos;

        uint8_t m_duid;

        uint32_t m_rssiAccum;
        uint16_t m_rssiCount;

        /**
         * @brief Helper to process P25 samples.
         * @param sample 
         */
        void processSample(q15_t sample);
        /**
         * @brief Helper to process LDU P25 samples.
         * @param sample 
         */
        void processVoice(q15_t sample);
        /**
         * @brief Helper to write a LDU data frame.
         */
        void writeLDUFrame();

        /**
         * @brief Helper to process PDU P25 samples.
         * @param sample 
         */
        void processData(q15_t sample);

        /**
         * @brief Frame synchronization correlator.
         * @returns bool 
         */
        bool correlateSync();

        /**
         * @brief Helper to decode the P25 NID.
         * @param start 
         * @returns bool True, if P25 NID was decoded, otherwise false.
         */
        bool decodeNid(uint16_t start);

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
} // namespace p25

#endif // __P25_RX_H__

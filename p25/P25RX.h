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
*   Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
*   Copyright (C) 2017-2024 Bryan Biedenkapp, N2PLL
*
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

    enum P25RX_STATE {
        P25RXS_NONE,
        P25RXS_SYNC,
        P25RXS_VOICE,
        P25RXS_DATA
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for P25 mode operation.
    // ---------------------------------------------------------------------------

    class DSP_FW_API P25RX {
    public:
        /// <summary>Initializes a new instance of the P25RX class.</summary>
        P25RX();

        /// <summary>Helper to reset data values to defaults.</summary>
        void reset();

        /// <summary>Sample P25 values from the air interface.</summary>
        void samples(const q15_t* samples, uint16_t* rssi, uint8_t length);

        /// <summary>Sets the P25 NAC.</summary>
        void setNAC(uint16_t nac);
        /// <summary>Sets the P25 sync correlation countdown.</summary>
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

        uint8_t m_duid;

        uint32_t m_rssiAccum;
        uint16_t m_rssiCount;

        /// <summary>Helper to process P25 samples.</summary>
        void processSample(q15_t sample);
        /// <summary>Helper to process LDU P25 samples.</summary>
        void processVoice(q15_t sample);
        /// <summary>Helper to process PDU P25 samples.</summary>
        void processData(q15_t sample);

        /// <summary>Frame synchronization correlator.</summary>
        bool correlateSync();

        /// <summary>Helper to decode the P25 NID.</summary>
        bool decodeNid(uint16_t start);

        /// <summary></summary>
        void calculateLevels(uint16_t start, uint16_t count);
        /// <summary></summary>
        void samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);
    };
} // namespace p25

#endif // __P25_RX_H__

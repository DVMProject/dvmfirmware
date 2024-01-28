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
*
*/
#if !defined(__DMR_SLOT_RX_H__)
#define __DMR_SLOT_RX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    enum DMRRX_STATE {
        DMRRXS_NONE,
        DMRRXS_VOICE,
        DMRRXS_DATA
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for DMR slots.
    // ---------------------------------------------------------------------------

    class DSP_FW_API DMRSlotRX {
    public:
        /// <summary>Initializes a new instance of the DMRSlotRX class.</summary>
        DMRSlotRX(bool slot);

        /// <summary>Helper to set data values for start of Rx.</summary>
        void start();
        /// <summary>Helper to reset data values to defaults.</summary>
        void reset();

        /// <summary>Perform DMR slot sample processing.</summary>
        bool processSample(q15_t sample, uint16_t rssi);

        /// <summary>Sets the DMR color code.</summary>
        void setColorCode(uint8_t colorCode);
        /// <summary>Sets the number of samples to delay before processing.</summary>
        void setRxDelay(uint8_t delay);

    private:
        bool m_slot;

        uint32_t m_bitBuffer[DMR_RADIO_SYMBOL_LENGTH];
        q15_t m_buffer[900U];

        uint16_t m_bitPtr;
        uint16_t m_dataPtr;
        uint16_t m_syncPtr;
        uint16_t m_startPtr;
        uint16_t m_endPtr;
        uint16_t m_delayPtr;

        q31_t m_maxCorr;
        q15_t m_centre[4U];
        q15_t m_threshold[4U];
        uint8_t m_averagePtr;

        uint8_t m_control;
        uint8_t m_syncCount;

        uint8_t m_colorCode;

        uint16_t m_delay;

        DMRRX_STATE m_state;

        uint8_t m_n;

        uint8_t m_type;

        uint16_t m_rssi[900U];

        /// <summary>Frame synchronization correlator.</summary>
        void correlateSync(bool first);

        /// <summary></summary>
        void samplesToBits(uint16_t start, uint8_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);
        /// <summary></summary>
        void writeRSSIData(uint8_t* frame);
    };
} // namespace dmr

#endif // __DMR_SLOT_RX_H__

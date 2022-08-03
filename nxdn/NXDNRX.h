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
 *   Copyright (C) 2015,2016,2017,2018,2020 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

    enum NXDNRX_STATE {
        NXDNRXS_NONE,
        NXDNRXS_DATA
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for DMR slots.
    // ---------------------------------------------------------------------------

    class DSP_FW_API NXDNRX {
    public:
        /// <summary>Initializes a new instance of the NXDNRX class.</summary>
        NXDNRX();

        /// <summary>Helper to reset data values to defaults.</summary>
        void reset();

        /// <summary>Sample NXDN values from the air interface.</summary>
        void samples(const q15_t* samples, uint16_t* rssi, uint8_t length);

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

        NXDNRX_STATE m_state;

        uint32_t m_rssiAccum;
        uint16_t m_rssiCount;

        /// <summary>Helper to process NXDN samples.</summary>
        void processSample(q15_t sample);
        /// <summary>Helper to process NXDN data samples.</summary>
        void processData(q15_t sample);

        /// <summary>Frame synchronization correlator.</summary>
        bool correlateSync();

        /// <summary></summary>
        void calculateLevels(uint16_t start, uint16_t count);
        /// <summary></summary>
        void samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);

        /// <summary></summary>
        void writeRSSIData(uint8_t* data);
    };
} // namespace nxdn

#endif // __NXDN_RX_H__

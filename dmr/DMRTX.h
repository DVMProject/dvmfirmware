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
*   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2016 by Colin Durbridge G4EML
*   Copyright (C) 2021-2022 by Bryan Biedenkapp N2PLL
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
#if !defined(__DMR_TX_H__)
#define __DMR_TX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"
#include "SerialBuffer.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    enum DMRTXSTATE {
        DMRTXSTATE_IDLE,
        DMRTXSTATE_SLOT1,
        DMRTXSTATE_CACH1,
        DMRTXSTATE_SLOT2,
        DMRTXSTATE_CACH2,
        DMRTXSTATE_CAL
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for duplex DMR mode operation.
    // ---------------------------------------------------------------------------

    class DSP_FW_API DMRTX {
    public:
        /// <summary>Initializes a new instance of the DMRTX class.</summary>
        DMRTX();

        /// <summary>Process local buffer and transmit on the air interface.</summary>
        void process();

        /// <summary>Write slot 1 data to the local buffer.</summary>
        uint8_t writeData1(const uint8_t* data, uint8_t length);
        /// <summary>Write slot 2 data to the local buffer.</summary>
        uint8_t writeData2(const uint8_t* data, uint8_t length);

        /// <summary>Write short LC data to the local buffer.</summary>
        uint8_t writeShortLC(const uint8_t* data, uint8_t length);
        /// <summary>Write abort data to the local buffer.</summary>
        uint8_t writeAbort(const uint8_t* data, uint8_t length);

        /// <summary>Helper to set the start state for Tx.</summary>
        void setStart(bool start);
        /// <summary>Helper to set the calibration state for Tx.</summary>
        void setCal(bool start);

        /// <summary>Helper to get how much space the slot 1 ring buffer has for samples.</summary>
        uint8_t getSpace1() const;
        /// <summary>Helper to get how much space the slot 2 ring buffer has for samples.</summary>
        uint8_t getSpace2() const;

        /// <summary>Sets the ignore flags for setting the CACH Access Type bit.</summary>
        void setIgnoreCACH_AT(uint8_t slot);
        /// <summary>Sets the DMR color code.</summary>
        void setColorCode(uint8_t colorCode);
        /// <summary>Sets the fine adjust 4FSK symbol levels.</summary>
        void setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj);

        /// <summary>Helper to reset data values to defaults for slot 1 FIFO.</summary>
        void resetFifo1();
        /// <summary>Helper to reset data values to defaults for slot 2 FIFO.</summary>
        void resetFifo2();

        /// <summary>Helper to resize the FIFO buffer.</summary>
        void resizeBuffer(uint16_t size);

        /// <summary></summary>
        uint32_t getFrameCount();

    private:
        SerialBuffer m_fifo[2U];

        arm_fir_interpolate_instance_q15 m_modFilter;

        q15_t m_modState[16U];    // blockSize + phaseLength - 1, 4 + 9 - 1 plus some spare

        DMRTXSTATE m_state;

        uint8_t m_idle[DMR_FRAME_LENGTH_BYTES];
        uint8_t m_cachPtr;

        uint8_t m_shortLC[12U];
        uint8_t m_newShortLC[12U];

        uint8_t m_markBuffer[40U];

        uint8_t m_poBuffer[40U];
        uint16_t m_poLen;
        uint16_t m_poPtr;

        uint32_t m_frameCount;

        uint32_t m_abortCount[2U];
        bool m_abort[2U];

        int8_t m_symLevel3Adj;
        int8_t m_symLevel1Adj;

        uint8_t m_cachATControl;

        /// <summary></summary>
        void createData(uint8_t slotIndex);
        /// <summary></summary>
        void createCACH(uint8_t txSlotIndex, uint8_t rxSlotIndex);
        /// <summary></summary>
        void createCal();

        /// <summary></summary>
        void writeByte(uint8_t c, uint8_t control);
    };
} // namespace dmr

#endif // __DMR_TX_H__

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
*   Copyright (C) 2009-2015 by Jonathan Naylor G4KLX
*   Copyright (C) 2016 by Colin Durbridge G4EML
*   Copyright (C) 2018 by Andy Uribe CA6JAU
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
#if !defined(__CAL_DMR_H__)
#define __CAL_DMR_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    enum DMR1KCAL {
        DMRCAL1K_IDLE,
        DMRCAL1K_VH,
        DMRCAL1K_VOICE,
        DMRCAL1K_VT,
        DMRCAL1K_WAIT
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements logic for DMR calibration mode.
    // ---------------------------------------------------------------------------

    class DSP_FW_API CalDMR {
    public:
        /// <summary>Initializes a new instance of the CalDMR class.</summary>
        CalDMR();

        /// <summary>Process local state and transmit on the air interface.</summary>
        void process();

        /// <summary></summary>
        void createData1k(uint8_t n);
        /// <summary></summary>
        void createDataDMO1k(uint8_t n);

        /// <summary></summary>
        void dmr1kcal();
        /// <summary></summary>
        void dmrDMO1kcal();

        /// <summary>Write DMR calibration state.</summary>
        uint8_t write(const uint8_t* data, uint8_t length);

    private:
        bool m_transmit;
        DMR1KCAL m_state;
        uint32_t m_frameStart;

        uint8_t m_dmr1k[DMR_FRAME_LENGTH_BYTES + 1U];

        uint8_t m_audioSeq;
    };
} // namespace dmr

#endif // __CAL_DMR_H__

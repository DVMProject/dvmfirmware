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
*   Copyright (C) 2009-2015 Jonathan Naylor, G4KLX
*   Copyright (C) 2016 Colin Durbridge, G4EML
*   Copyright (C) 2018 Andy Uribe, CA6JAU
*
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

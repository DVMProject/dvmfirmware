// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2009-2015 Jonathan Naylor, G4KLX
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2018 Andy Uribe, CA6JAU
 *
 */
/**
 * @file CalDMR.h
 * @ingroup dmr_mfw
 * @file CalDMR.h
 * @ingroup dmr_mfw
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

    /**
     * @brief Calibration States
     */
    enum DMR1KCAL {
        DMRCAL1K_IDLE,      //! Idle
        DMRCAL1K_VH,        //! Voice Header
        DMRCAL1K_VOICE,     //! Voice
        DMRCAL1K_VT,        //! Voice Terminator
        DMRCAL1K_WAIT       //!
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements logic for DMR calibration mode.
    // ---------------------------------------------------------------------------

    class DSP_FW_API CalDMR {
    public:
        /**
         * @brief Initializes a new instance of the CalDMR class.
         */
        CalDMR();

        /**
         * @brief Process local state and transmit on the air interface.
         */
        void process();

        /**
         * @brief 
         * @param n 
         */
        void createData1k(uint8_t n);
        /**
         * @brief 
         * @param n 
         */
        void createDataDMO1k(uint8_t n);

        /**
         * @brief 
         */
        void dmr1kcal();
        /**
         * @brief 
         */
        void dmrDMO1kcal();

        /**
         * @brief Write DMR calibration state.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
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

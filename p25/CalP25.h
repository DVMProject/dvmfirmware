// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2018 Andy Uribe, CA6JAU
 *
 */
/**
 * @file CalP25.h
 * @ingroup p25_mfw
 * @file CalP25.h
 * @ingroup p25_mfw
 */
#if !defined(__CAL_P25_H__)
#define __CAL_P25_H__

#include "Defines.h"
#include "p25/P25Defines.h"

namespace p25
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    /**
     * @brief Calibration States
     * @ingroup p25_mfw
     */
    enum P25CAL1K {
        P25CAL1K_IDLE,      //! Idle
        P25CAL1K_LDU1,      //! LDU1
        P25CAL1K_LDU2       //! LDU2
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements logic for P25 calibration mode.
     * @ingroup p25_mfw
     */
    class DSP_FW_API CalP25 {
    public:
        /**
         * @brief Initializes a new instance of the CalP25 class.
         */
        CalP25();

        /**
         * @brief Process local state and transmit on the air interface.
         */
        void process();

        /**
         * @brief Write P25 calibration state.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
        uint8_t write(const uint8_t* data, uint8_t length);

    private:
        bool m_transmit;
        P25CAL1K m_state;
    };
} // namespace p25

#endif // __CAL_P25_H__

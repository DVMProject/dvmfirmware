// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
 *
 */
/**
 * @file DMRRX.h
 * @ingroup dmr_mfw
 * @file DMRRX.cpp
 * @ingroup dmr_mfw
 */
#if !defined(__DMR_RX_H__)
#define __DMR_RX_H__

#include "Defines.h"
#include "dmr/DMRSlotRX.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements receiver logic for duplex DMR mode operation.
     * @ingroup dmr_mfw
     */
    class DSP_FW_API DMRRX {
    public:
        /**
         * @brief Initializes a new instance of the DMRRX class.
         */
        DMRRX();

        /**
         * @brief Helper to reset data values to defaults.
         */
        void reset();

        /**
         * @brief Sample DMR values from the air interface.
         * @param[in] samples 
         * @param[in] rssi
         * @param[in] control 
         * @param length 
         */
        void samples(const q15_t* samples, const uint16_t* rssi, const uint8_t* control, uint8_t length);

        /**
         * @brief Sets the DMR color code.
         * @param colorCode 
         */
        void setColorCode(uint8_t colorCode);
        /**
         * @brief Sets the number of samples to delay before processing.
         * @param delay 
         */
        void setRxDelay(uint8_t delay);

    private:
        DMRSlotRX m_slot1RX;
        DMRSlotRX m_slot2RX;
    };
} // namespace dmr

#endif // __DMR_RX_H__

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015 Jonathan Naylor, G4KLX
 *
 */
/**
 * @file DMRSlotType.h
 * @ingroup dmr_mfw
 * @file DMRSlotType.h
 * @ingroup dmr_mfw
 */
#if !defined(__DMR_SLOT_TYPE_H__)
#define __DMR_SLOT_TYPE_H__

#include "Defines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Represents DMR slot type.
     * @ingroup dmr_mfw
     */
    class DSP_FW_API DMRSlotType {
    public:
        /**
         * @brief Initializes a new instance of the DMRSlotType class.
         */
        DMRSlotType();

        /**
         * @brief Decodes DMR slot type.
         * @param[in] frame 
         * @param[out] colorCode 
         * @param[out] dataType 
         */
        void decode(const uint8_t* frame, uint8_t& colorCode, uint8_t& dataType) const;
        /**
         * @brief Encodes DMR slot type.
         * @param colorCode 
         * @param dataType 
         * @param[out] frame 
         */
        void encode(uint8_t colorCode, uint8_t dataType, uint8_t* frame) const;

    private:
        /**
         * @brief 
         * @param[in] data 
         * @returns uint8_t 
         */
        uint8_t decode2087(const uint8_t* data) const;
        /**
         * @brief 
         * @param pattern 
         * @returns uint32_t 
         */
        uint32_t getSyndrome1987(uint32_t pattern) const;
    };
} // namespace dmr

#endif // __DMR_SLOT_TYPE_H__

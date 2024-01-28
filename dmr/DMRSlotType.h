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
*   Copyright (C) 2015 Jonathan Naylor, G4KLX
*
*/
#if !defined(__DMR_SLOT_TYPE_H__)
#define __DMR_SLOT_TYPE_H__

#include "Defines.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    //
    // ---------------------------------------------------------------------------

    class DSP_FW_API DMRSlotType {
    public:
        /// <summary>Initializes a new instance of the DMRSlotType class.</summary>
        DMRSlotType();

        /// <summary>Decodes DMR slot type.</summary>
        void decode(const uint8_t* frame, uint8_t& colorCode, uint8_t& dataType) const;
        /// <summary>Encodes DMR slot type.</summary>
        void encode(uint8_t colorCode, uint8_t dataType, uint8_t* frame) const;

    private:
        /// <summary></summary>
        uint8_t decode2087(const uint8_t* data) const;
        /// <summary></summary>
        uint32_t getSyndrome1987(uint32_t pattern) const;
    };
} // namespace dmr

#endif // __DMR_SLOT_TYPE_H__

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
*   Copyright (C) 2015 by Jonathan Naylor G4KLX
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

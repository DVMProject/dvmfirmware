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
*   Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
*
*/
#if !defined(__DMR_RX_H__)
#define __DMR_RX_H__

#include "Defines.h"
#include "dmr/DMRSlotRX.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Class Declaration
    //      Implements receiver logic for duplex DMR mode operation.
    // ---------------------------------------------------------------------------

    class DSP_FW_API DMRRX {
    public:
        /// <summary>Initializes a new instance of the DMRRX class.</summary>
        DMRRX();

        /// <summary>Helper to reset data values to defaults.</summary>
        void reset();

        /// <summary>Sample DMR values from the air interface.</summary>
        void samples(const q15_t* samples, const uint16_t* rssi, const uint8_t* control, uint8_t length);

        /// <summary>Sets the DMR color code.</summary>
        void setColorCode(uint8_t colorCode);
        /// <summary>Sets the number of samples to delay before processing.</summary>
        void setRxDelay(uint8_t delay);

    private:
        DMRSlotRX m_slot1RX;
        DMRSlotRX m_slot2RX;
    };
} // namespace dmr

#endif // __DMR_RX_H__

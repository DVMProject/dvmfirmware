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
#if !defined(__UTILS_H__)
#define __UTILS_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

DSP_FW_API uint8_t countBits8(uint8_t bits);
DSP_FW_API uint8_t countBits32(uint32_t bits);
DSP_FW_API uint8_t countBits64(ulong64_t bits);

#endif // __UTILS_H__

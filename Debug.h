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
*   Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
*
*/
#if !defined(__DEBUG_H__)
#define __DEBUG_H__

#include "Globals.h"

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

#define  DEBUG1(a)          serial.writeDebug((a))
#define  DEBUG2(a,b)        serial.writeDebug((a),(b))
#define  DEBUG3(a,b,c)      serial.writeDebug((a),(b),(c))
#define  DEBUG4(a,b,c,d)    serial.writeDebug((a),(b),(c),(d))
#define  DEBUG5(a,b,c,d,e)  serial.writeDebug((a),(b),(c),(d),(e))

#endif // __DEBUG_H__

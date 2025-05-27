// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2009,2014,2015 Jonathan Naylor, G4KLX
 *  Copyright (C) 2018-2025 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @defgroup utils Common Utility Routines
 * @brief Defines and implements utility routines.
 * @ingroup modem_fw
 * 
 * @file Utils.h
 * @ingroup utils
 * @file Utils.cpp
 * @ingroup utils
 */
#if !defined(__SDR_UTILS_H__)
#define __SDR_UTILS_H__

#include "Defines.h"

#include <cstring>
#include <string>

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Various helper utilities.
 * @ingroup utils
 */
class DSP_FW_API Utils {
public:
    /**
     * @brief Helper to dump the input buffer and display the hexadecimal output in the log.
     * @param title Name of buffer.
     * @param data Buffer to dump.
     * @param length Length of buffer.
     */
    static void dump(const std::string& title, const uint8_t* data, uint32_t length);
    /**
     * @brief Helper to dump the input buffer and display the hexadecimal output in the log.
     * @param level Log level.
     * @param title Name of buffer.
     * @param data Buffer to dump.
     * @param length Length of buffer.
     */
    static void dump(int level, const std::string& title, const uint8_t* data, uint32_t length);
};

#endif // __SDR_UTILS_H__

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
 * @file Utils.h
 * @ingroup modem_fw
 * @file Utils.cpp
 * @ingroup modem_fw
 */
#if !defined(__UTILS_H__)
#define __UTILS_H__

#include "Defines.h"

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

/**
 * @brief Returns the count of bits in the passed 8 byte value.
 * @param bits uint8_t to count bits for.
 * @returns uint8_t Count of bits in passed value.
 */
DSP_FW_API uint8_t countBits8(uint8_t bits);
/**
 * @brief Returns the count of bits in the passed 32 byte value.
 * @param bits uint32_t to count bits for.
 * @returns uint8_t Count of bits in passed value.
 */
DSP_FW_API uint8_t countBits32(uint32_t bits);
/**
 * @brief Returns the count of bits in the passed 64 byte value.
 * @param bits ulong64_t to count bits for.
 * @returns uint8_t Count of bits in passed value.
 */
DSP_FW_API uint8_t countBits64(ulong64_t bits);

#endif // __UTILS_H__

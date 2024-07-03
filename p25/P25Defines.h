// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2017-2024 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @defgroup p25_mfw Project 25
 * @brief Implementation for the TIA-102 Project 25 standard.
 * @ingroup modem_fw
 * 
 * @file P25Defines.h
 * @ingroup p25_mfw
 */
#if !defined(__P25_DEFINES_H__)
#define __P25_DEFINES_H__

#include "Defines.h"

namespace p25
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    /**
     * @addtogroup p25_mfw
     * @{
     */

    const uint32_t  P25_RADIO_SYMBOL_LENGTH = 5U;      // At 24 kHz sample rate

    const uint32_t  P25_HDU_FRAME_LENGTH_BYTES = 99U;
    const uint32_t  P25_HDU_FRAME_LENGTH_BITS = P25_HDU_FRAME_LENGTH_BYTES * 8U;
    const uint32_t  P25_HDU_FRAME_LENGTH_SYMBOLS = P25_HDU_FRAME_LENGTH_BYTES * 4U;
    const uint32_t  P25_HDU_FRAME_LENGTH_SAMPLES = P25_HDU_FRAME_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_TDU_FRAME_LENGTH_BYTES = 18U;
    const uint32_t  P25_TDU_FRAME_LENGTH_BITS = P25_TDU_FRAME_LENGTH_BYTES * 8U;
    const uint32_t  P25_TDU_FRAME_LENGTH_SYMBOLS = P25_TDU_FRAME_LENGTH_BYTES * 4U;
    const uint32_t  P25_TDU_FRAME_LENGTH_SAMPLES = P25_TDU_FRAME_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_LDU_FRAME_LENGTH_BYTES = 216U;
    const uint32_t  P25_LDU_FRAME_LENGTH_BITS = P25_LDU_FRAME_LENGTH_BYTES * 8U;
    const uint32_t  P25_LDU_FRAME_LENGTH_SYMBOLS = P25_LDU_FRAME_LENGTH_BYTES * 4U;
    const uint32_t  P25_LDU_FRAME_LENGTH_SAMPLES = P25_LDU_FRAME_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_TSDU_FRAME_LENGTH_BYTES = 45U;
    const uint32_t  P25_TSDU_FRAME_LENGTH_BITS = P25_TSDU_FRAME_LENGTH_BYTES * 8U;
    const uint32_t  P25_TSDU_FRAME_LENGTH_SYMBOLS = P25_TSDU_FRAME_LENGTH_BYTES * 4U;
    const uint32_t  P25_TSDU_FRAME_LENGTH_SAMPLES = P25_TSDU_FRAME_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_PDU_FRAME_LENGTH_BYTES = 512U;
    const uint32_t  P25_PDU_FRAME_LENGTH_BITS = P25_PDU_FRAME_LENGTH_BYTES * 8U;
    const uint32_t  P25_PDU_FRAME_LENGTH_SYMBOLS = P25_PDU_FRAME_LENGTH_BYTES * 4U;
    const uint32_t  P25_PDU_FRAME_LENGTH_SAMPLES = P25_PDU_FRAME_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_TDULC_FRAME_LENGTH_BYTES = 54U;
    const uint32_t  P25_TDULC_FRAME_LENGTH_BITS = P25_TDULC_FRAME_LENGTH_BYTES * 8U;
    const uint32_t  P25_TDULC_FRAME_LENGTH_SYMBOLS = P25_TDULC_FRAME_LENGTH_BYTES * 4U;
    const uint32_t  P25_TDULC_FRAME_LENGTH_SAMPLES = P25_TDULC_FRAME_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_SYNC_LENGTH_BYTES = 6U;
    const uint32_t  P25_SYNC_LENGTH_BITS = P25_SYNC_LENGTH_BYTES * 8U;
    const uint32_t  P25_SYNC_LENGTH_SYMBOLS = P25_SYNC_LENGTH_BYTES * 4U;
    const uint32_t  P25_SYNC_LENGTH_SAMPLES = P25_SYNC_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint32_t  P25_NID_LENGTH_BYTES = 2U;
    const uint32_t  P25_NID_LENGTH_BITS = P25_NID_LENGTH_BYTES * 8U;
    const uint32_t  P25_NID_LENGTH_SYMBOLS = P25_NID_LENGTH_BYTES * 4U;
    const uint32_t  P25_NID_LENGTH_SAMPLES = P25_NID_LENGTH_SYMBOLS * P25_RADIO_SYMBOL_LENGTH;

    const uint8_t   P25_SYNC_BYTES[] = { 0x55U, 0x75U, 0xF5U, 0xFFU, 0x77U, 0xFFU };
    const uint8_t   P25_SYNC_BYTES_LENGTH = 6U;
    const uint8_t   P25_START_SYNC = 0x5FU;
    const uint8_t   P25_NULL = 0x00U;

    const uint8_t   P25_TEST_PATTERN[] = { 0x55, 0xFF };
    const uint8_t   P25_TEST_PATTERN_LENGTH = 2U;

    const ulong64_t P25_SYNC_BITS = 0x00005575F5FF77FFU;
    const ulong64_t P25_SYNC_BITS_MASK = 0x0000FFFFFFFFFFFFU;

    // 5     5      7     5      F     5      F     F      7     7      F     F
    // 01 01 01 01  01 11 01 01  11 11 01 01  11 11 11 11  01 11 01 11  11 11 11 11
    // +3 +3 +3 +3  +3 -3 +3 +3  -3 -3 +3 +3  -3 -3 -3 -3  +3 -3 +3 -3  -3 -3 -3 -3

    const int8_t    P25_SYNC_SYMBOLS_VALUES[] = { +3, +3, +3, +3, +3, -3, +3, +3, -3, -3, +3, +3, -3, -3, -3, -3, +3, -3, +3, -3, -3, -3, -3, -3 };

    const uint32_t  P25_SYNC_SYMBOLS = 0x00FB30A0U;
    const uint32_t  P25_SYNC_SYMBOLS_MASK = 0x00FFFFFFU;

    // 522 = P25_PDU_FRAME_LENGTH_BYTES + 10 (BUFFER_LEN = P25_PDU_FRAME_LENGTH_BYTES + 10)
    const uint32_t  P25_TX_BUFFER_LEN = 522U;

    // Data Unit ID(s)
    const uint8_t   P25_DUID_HDU = 0x00U;               //! Header Data Unit
    const uint8_t   P25_DUID_TDU = 0x03U;               //! Simple Terminator Data Unit
    const uint8_t   P25_DUID_LDU1 = 0x05U;              //! Logical Link Data Unit 1
    const uint8_t   P25_DUID_VSELP1 = 0x06U;            //! Motorola VSELP 1
    const uint8_t   P25_DUID_TSDU = 0x07U;              //! Trunking System Data Unit
    const uint8_t   P25_DUID_VSELP2 = 0x09U;            //! Motorola VSELP 2
    const uint8_t   P25_DUID_LDU2 = 0x0AU;              //! Logical Link Data Unit 2
    const uint8_t   P25_DUID_PDU = 0x0CU;               //! Packet Data Unit 
    const uint8_t   P25_DUID_TDULC = 0x0FU;             //! Terminator Data Unit with Link Control

    /** @} */
} // namespace p25

#endif // __P25_DEFINES_H__

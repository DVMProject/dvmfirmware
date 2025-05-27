// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Common Library
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2009,2014,2015,2016 Jonathan Naylor, G4KLX
 *  Copyright (C) 2018-2025 Bryan Biedenkapp, N2PLL
 *
 */
#include "sdr/Utils.h"
#include "sdr/Log.h"

#include <cstdio>
#include <cassert>

// ---------------------------------------------------------------------------
//  Static Class Members
// ---------------------------------------------------------------------------

/* Helper to dump the input buffer and display the hexadecimal output in the log. */

void Utils::dump(const std::string& title, const uint8_t* data, uint32_t length)
{
    assert(data != nullptr);

    dump(2U, title, data, length);
}

/* Helper to dump the input buffer and display the hexadecimal output in the log. */

void Utils::dump(int level, const std::string& title, const uint8_t* data, uint32_t length)
{
    assert(data != nullptr);

    ::Log(level, "DUMP", nullptr, 0, nullptr, "%s (len %u)", title.c_str(), length);

    uint32_t offset = 0U;

    while (length > 0U) {
        std::string output;

        uint32_t bytes = (length > 16U) ? 16U : length;

        for (uint32_t i = 0U; i < bytes; i++) {
            char temp[10U];
            ::sprintf(temp, "%02X ", data[offset + i]);
            output += temp;
        }
#if !defined(CATCH2_TEST_COMPILATION)
        for (uint32_t i = bytes; i < 16U; i++)
            output += "   ";

        output += "   *";

        for (uint32_t i = 0U; i < bytes; i++) {
            uint8_t c = data[offset + i];

            if (::isprint(c))
                output += c;
            else
                output += '.';
        }

        output += '*';
#endif
        ::Log(level, "DUMP", nullptr, 0, nullptr, "%04X:  %s", offset, output.c_str());

        offset += 16U;

        if (length >= 16U)
            length -= 16U;
        else
            length = 0U;
    }
}

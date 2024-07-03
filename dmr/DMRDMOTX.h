// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2020 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file DMRDMOTX.h
 * @ingroup dmr_mfw
 * @file DMRDMOTX.h
 * @ingroup dmr_mfw
 */
#if !defined(__DMR_DMO_TX_H__)
#define __DMR_DMO_TX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"
#include "SerialBuffer.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    #define DMRDMO_FIXED_DELAY 300  // 300 = 62.49ms
                                    // Delay Value * 0.2083 = Preamble Length (ms)

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements transmitter logic for DMR DMO mode operation.
     * @ingroup dmr_mfw
     */
    class DSP_FW_API DMRDMOTX {
    public:
        /**
         * @brief Initializes a new instance of the DMRDMOTX class.
         */
        DMRDMOTX();

        /**
         * @brief Process local buffer and transmit on the air interface.
         */
        void process();

        /**
         * @brief Write data to the local buffer.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
        uint8_t writeData(const uint8_t* data, uint8_t length);

        /**
         * @brief Sets the FDMA preamble count.
         * @param preambleCnt FDMA preamble count.
         */
        void setPreambleCount(uint8_t preambleCnt);
        /**
         * @brief Sets the fine adjust 4FSK symbol levels.
         * @param level3Adj +3/-3 Level Adjustment
         * @param level1Adj +1/-1 Level Adjustment
         */
        void setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj);

        /**
         * @brief Helper to resize the FIFO buffer.
         * @param size 
         */
        void resizeBuffer(uint16_t size);

        /**
         * @brief Helper to get how much space the ring buffer has for samples.
         * @returns uint8_t Amount of space in ring buffer for samples. 
         */
        uint8_t getSpace() const;

    private:
        SerialBuffer m_fifo;

        arm_fir_interpolate_instance_q15 m_modFilter;

        q15_t m_modState[16U];    // blockSize + phaseLength - 1, 4 + 9 - 1 plus some spare

        uint8_t m_poBuffer[1200U];
        uint16_t m_poLen;
        uint16_t m_poPtr;

        uint32_t m_preambleCnt;

        int8_t m_symLevel3Adj;
        int8_t m_symLevel1Adj;

        /**
         * @brief Helper to write a raw byte to the DAC.
         * @param c Byte.
         */
        void writeByte(uint8_t c);
        /**
         * @brief 
         */
        void writeSilence();
    };
} // namespace dmr

#endif // __DMR_DMO_TX_H__

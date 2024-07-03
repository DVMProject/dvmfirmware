// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017,2018,2020 Jonathan Naylor, G4KLX
 *  Copyright (C) 2022 by Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file NXDNTX.h
 * @ingroup nxdn_mfw
 * @file NXDNTX.h
 * @ingroup nxdn_mfw
 */
#if !defined(__NXDN_TX_H__)
#define __NXDN_TX_H__

#include "Defines.h"
#include "SerialBuffer.h"

namespace nxdn
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    #define NXDN_FIXED_TX_HANG 600

    /**
     * @brief NXDN Transmitter States
     */
    enum NXDNTXSTATE {
        NXDNTXSTATE_NORMAL,     //! Normal
        NXDNTXSTATE_CAL         //! Calibration
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements transmitter logic for NXDN mode operation.
     * @ingroup nxdn_mfw
     */
    class DSP_FW_API NXDNTX {
    public:
        /**
         * @brief Initializes a new instance of the NXDNTX class.
         */
        NXDNTX();

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
         * @brief Clears the local buffer.
         */
        void clear();

        /**
         * @brief Sets the FDMA preamble count.
         * @param preambleCnt FDMA preamble count.
         */
        void setPreambleCount(uint8_t preambleCnt);
        /**
         * @brief Sets the transmit hang time.
         * @param txHang Transmit hang time.
         */
        void setTxHang(uint8_t txHang);
        /**
         * @brief Sets the fine adjust 4FSK symbol levels.
         * @param level3Adj +3/-3 Level Adjustment
         * @param level1Adj +1/-1 Level Adjustment
         */
        void setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj);
        /**
         * @brief Helper to set the calibration state for Tx.
         * @param start 
         */
        void setCal(bool start);

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

        NXDNTXSTATE m_state;

        arm_fir_interpolate_instance_q15 m_modFilter;
        arm_fir_instance_q15 m_sincFilter;

        q15_t m_modState[16U];  // blockSize + phaseLength - 1, 4 + 9 - 1 plus some spare
        q15_t m_sincState[70U]; // NoTaps + BlockSize - 1, 22 + 40 - 1 plus some spare

        uint8_t m_poBuffer[1200U];
        uint16_t m_poLen;
        uint16_t m_poPtr;

        uint16_t m_preambleCnt;
        uint32_t m_txHang;
        uint32_t m_tailCnt;

        int8_t m_symLevel3Adj;
        int8_t m_symLevel1Adj;

        /**
         * @brief Helper to generate data.
         */
        void createData();

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
} // namespace nxdn

#endif // __NXDN_TX_H__

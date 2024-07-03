// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2021-2022 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file DMRTX.h
 * @ingroup dmr_mfw
 * @file DMRTX.h
 * @ingroup dmr_mfw
 */
#if !defined(__DMR_TX_H__)
#define __DMR_TX_H__

#include "Defines.h"
#include "dmr/DMRDefines.h"
#include "SerialBuffer.h"

namespace dmr
{
    // ---------------------------------------------------------------------------
    //  Constants
    // ---------------------------------------------------------------------------

    /**
     * @brief DMR Duplex Transmitter State
     * @ingroup dmr_mfw
     */
    enum DMRTXSTATE {
        DMRTXSTATE_IDLE,        //! Idle
        DMRTXSTATE_SLOT1,       //! Slot 1
        DMRTXSTATE_CACH1,       //! Common Access Channel 1
        DMRTXSTATE_SLOT2,       //! Slot 2
        DMRTXSTATE_CACH2,       //! Common Access Channel 2
        DMRTXSTATE_CAL          //! Calibration
    };

    // ---------------------------------------------------------------------------
    //  Class Declaration
    // ---------------------------------------------------------------------------

    /**
     * @brief Implements transmitter logic for duplex DMR mode operation.
     * @ingroup dmr_mfw
     */
    class DSP_FW_API DMRTX {
    public:
        /**
         * @brief Initializes a new instance of the DMRTX class.
         */
        DMRTX();

        /**
         * @brief Process local buffer and transmit on the air interface.
         */
        void process();

        /**
         * @brief Write slot 1 data to the local buffer.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
        uint8_t writeData1(const uint8_t* data, uint8_t length);
        /**
         * @brief Write slot 2 data to the local buffer.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
        uint8_t writeData2(const uint8_t* data, uint8_t length);

        /**
         * @brief Write short LC data to the local buffer.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
        uint8_t writeShortLC(const uint8_t* data, uint8_t length);
        /**
         * @brief Write abort data to the local buffer.
         * @param[in] data Buffer.
         * @param length Length of buffer.
         * @returns uint8_t Reason code.
         */
        uint8_t writeAbort(const uint8_t* data, uint8_t length);

        /**
         * @brief Helper to set the start state for Tx.
         * @param start 
         */
        void setStart(bool start);
        /**
         * @brief Helper to set the calibration state for Tx.
         * @param start 
         */
        void setCal(bool start);

        /**
         * @brief Helper to get how much space the slot 1 ring buffer has for samples.
         * @returns uint8_t Amount of space in the slot 1 ring buffer.
         */
        uint8_t getSpace1() const;
        /**
         * @brief Helper to get how much space the slot 2 ring buffer has for samples.
         * @returns uint8_t Amount of space in the slot 2 ring buffer.
         */
        uint8_t getSpace2() const;

        /**
         * @brief Sets the ignore flags for setting the CACH Access Type bit.
         * @param slot DMR slot number.
         */
        void setIgnoreCACH_AT(uint8_t slot);
        /**
         * @brief Sets the DMR color code.
         * @param colorCode Color code.
         */
        void setColorCode(uint8_t colorCode);
        /**
         * @brief Sets the fine adjust 4FSK symbol levels.
         * @param level3Adj +3/-3 Level Adjustment
         * @param level1Adj +1/-1 Level Adjustment
         */
        void setSymbolLvlAdj(int8_t level3Adj, int8_t level1Adj);

        /**
         * @brief Helper to reset data values to defaults for slot 1 FIFO.
         */
        void resetFifo1();
        /**
         * @brief Helper to reset data values to defaults for slot 2 FIFO.
         */
        void resetFifo2();

        /**
         * @brief Helper to resize the FIFO buffer.
         * @param size 
         */
        void resizeBuffer(uint16_t size);

        /**
         * @brief 
         * @returns uint32_t 
         */
        uint32_t getFrameCount();

    private:
        SerialBuffer m_fifo[2U];

        arm_fir_interpolate_instance_q15 m_modFilter;

        q15_t m_modState[16U];    // blockSize + phaseLength - 1, 4 + 9 - 1 plus some spare

        DMRTXSTATE m_state;

        uint8_t m_idle[DMR_FRAME_LENGTH_BYTES];
        uint8_t m_cachPtr;

        uint8_t m_shortLC[12U];
        uint8_t m_newShortLC[12U];

        uint8_t m_markBuffer[40U];

        uint8_t m_poBuffer[40U];
        uint16_t m_poLen;
        uint16_t m_poPtr;

        uint32_t m_frameCount;

        uint32_t m_abortCount[2U];
        bool m_abort[2U];

        int8_t m_symLevel3Adj;
        int8_t m_symLevel1Adj;

        uint8_t m_cachATControl;

        /**
         * @brief Helper to generate data.
         * @param slotIndex 
         */
        void createData(uint8_t slotIndex);
        /**
         * @brief Helper to generate the common access channel.
         * @param txSlotIndex 
         * @param rxSlotIndex 
         */
        void createCACH(uint8_t txSlotIndex, uint8_t rxSlotIndex);
        /**
         * @brief Helper to generate calibration data.
         */
        void createCal();

        /**
         * @brief Helper to write a raw byte to the DAC.
         * @param c Byte.
         * @param control 
         */
        void writeByte(uint8_t c, uint8_t control);
    };
} // namespace dmr

#endif // __DMR_TX_H__

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2018,2021-2024 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file SerialPort.h
 * @ingroup modem_fw
 * @file SerialPort.cpp
 * @ingroup modem_fw
 * @file SerialDue.cpp
 * @ingroup modem_fw
 * @file SerialSTM.cpp
 * @ingroup modem_fw
 */
#if !defined(__SERIAL_PORT_H__)
#define __SERIAL_PORT_H__

#include "Defines.h"
#include "Globals.h"
#include "SerialBuffer.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

/**
 * @addtogroup modem
 * @{
 */

/**
 * @brief Modem operation states.
 */
enum DVM_STATE {
    STATE_IDLE = 0U,                    //! Idle
    // DMR
    STATE_DMR = 1U,                     //! DMR
    // Project 25
    STATE_P25 = 2U,                     //! Project 25
    // NXDN
    STATE_NXDN = 3U,                    //! NXDN

    // CW
    STATE_CW = 10U,                     //! Continuous Wave

    // Calibration States
    STATE_P25_CAL_1K = 92U,             //! Project 25 Calibration 1K

    STATE_DMR_DMO_CAL_1K = 93U,         //! DMR DMO Calibration 1K
    STATE_DMR_CAL_1K = 94U,             //! DMR Calibration 1K
    STATE_DMR_LF_CAL = 95U,             //! DMR Low Frequency Calibration

    STATE_RSSI_CAL = 96U,               //! RSSI Calibration

    STATE_P25_CAL = 97U,                //! Project 25 Calibration
    STATE_DMR_CAL = 98U,                //! DMR Calibration
    STATE_NXDN_CAL = 99U                //! NXDN Calibration
};

/**
 * @brief Modem commands.
 */
enum DVM_COMMANDS {
    CMD_GET_VERSION = 0x00U,            //! Get Modem Version
    CMD_GET_STATUS = 0x01U,             //! Get Modem Status
    CMD_SET_CONFIG = 0x02U,             //! Set Modem Configuration
    CMD_SET_MODE = 0x03U,               //! Set Modem Mode

    CMD_SET_SYMLVLADJ = 0x04U,          //! Set Symbol Level Adjustments
    CMD_SET_RXLEVEL = 0x05U,            //! Set Rx Level
    CMD_SET_RFPARAMS = 0x06U,           //! (Hotspot) Set RF Parameters

    CMD_CAL_DATA = 0x08U,               //! Calibration Data
    CMD_RSSI_DATA = 0x09U,              //! RSSI Data

    CMD_SEND_CWID = 0x0AU,              //! Send Continous Wave ID (Morse)

    CMD_SET_BUFFERS = 0x0FU,            //! Set FIFO Buffer Lengths

    CMD_DMR_DATA1 = 0x18U,              //! DMR Data Slot 1
    CMD_DMR_LOST1 = 0x19U,              //! DMR Data Lost Slot 1
    CMD_DMR_DATA2 = 0x1AU,              //! DMR Data Slot 2
    CMD_DMR_LOST2 = 0x1BU,              //! DMR Data Lost Slot 2
    CMD_DMR_SHORTLC = 0x1CU,            //! DMR Short Link Control
    CMD_DMR_START = 0x1DU,              //! DMR Start Transmit
    CMD_DMR_ABORT = 0x1EU,              //! DMR Abort
    CMD_DMR_CACH_AT_CTRL = 0x1FU,       //! DMR Set CACH AT Control
    CMD_DMR_CLEAR1 = 0x20U,             //! DMR Clear Slot 1 Buffer
    CMD_DMR_CLEAR2 = 0x21U,             //! DMR Clear Slot 2 Buffer

    CMD_P25_DATA = 0x31U,               //! Project 25 Data
    CMD_P25_LOST = 0x32U,               //! Project 25 Data Lost
    CMD_P25_CLEAR = 0x33U,              //! Project 25 Clear Buffer

    CMD_NXDN_DATA = 0x41U,              //! NXDN Data
    CMD_NXDN_LOST = 0x42U,              //! NXDN Data Lost
    CMD_NXDN_CLEAR = 0x43U,             //! NXDN Clear Buffer

    CMD_ACK = 0x70U,                    //! Command ACK
    CMD_NAK = 0x7FU,                    //! Command NACK

    CMD_FLSH_READ = 0xE0U,              //! Read Flash Partition
    CMD_FLSH_WRITE = 0xE1U,             //! Write Flash Partition

    CMD_RESET_MCU = 0xEAU,              //! Soft Reboot MCU

    CMD_DEBUG1 = 0xF1U,                 //!
    CMD_DEBUG2 = 0xF2U,                 //!
    CMD_DEBUG3 = 0xF3U,                 //!
    CMD_DEBUG4 = 0xF4U,                 //!
    CMD_DEBUG5 = 0xF5U,                 //!
    CMD_DEBUG_DUMP = 0xFAU,             //!
};

/**
 * @brief Modem response reason codes.
 */
enum CMD_REASON_CODE {
    RSN_OK = 0U,                        //! OK
    RSN_NAK = 1U,                       //! Negative Acknowledge

    RSN_ILLEGAL_LENGTH = 2U,            //! Illegal Length
    RSN_INVALID_REQUEST = 4U,           //! Invalid Request
    RSN_RINGBUFF_FULL = 8U,             //! Ring Buffer Full

    RSN_INVALID_FDMA_PREAMBLE = 10U,    //! Invalid FDMA Preamble Length
    RSN_INVALID_MODE = 11U,             //! Invalid Mode

    RSN_INVALID_DMR_CC = 12U,           //! Invalid DMR CC
    RSN_INVALID_DMR_SLOT = 13U,         //! Invalid DMR Slot
    RSN_INVALID_DMR_START = 14U,        //! Invaild DMR Start Transmit
    RSN_INVALID_DMR_RX_DELAY = 15U,     //! Invalid DMR Rx Delay

    RSN_INVALID_P25_CORR_COUNT = 16U,   //! Invalid P25 Correlation Count

    RSN_NO_INTERNAL_FLASH = 20U,        //! No Internal Flash
    RSN_FAILED_ERASE_FLASH = 21U,       //! Failed to erase flash partition
    RSN_FAILED_WRITE_FLASH = 22U,       //! Failed to write flash partition
    RSN_FLASH_WRITE_TOO_BIG = 23U,      //! Data to large for flash partition

    RSN_DMR_DISABLED = 63U,             //! DMR Disabled
    RSN_P25_DISABLED = 64U,             //! Project 25 Disabled
    RSN_NXDN_DISABLED = 65U             //! NXDN Disabled
};

const uint8_t DVM_SHORT_FRAME_START = 0xFEU;
const uint8_t DVM_LONG_FRAME_START = 0xFDU;

#define SERIAL_FB_LEN 518U
#define SERIAL_SPEED 115200
/** @} */

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Implements the RS232 serial bus to communicate with the HOST S/W.
 * @ingroup modem_fw
 */
class DSP_FW_API SerialPort {
public:
    /**
     * @brief Initializes a new instance of the SerialPort class.
     */
    SerialPort();

    /**
     * @brief Starts serial port communications.
     */
    void start();

    /**
     * @brief Process data from serial port.
     */
    void process();

    /**
     * @brief Helper to check if the modem is in a calibration state.
     * @param state 
     * @returns bool 
     */
    bool isCalState(DVM_STATE state);
    /**
     * @brief Helper to determine digital mode if the modem is in a calibration state.
     * @param state 
     * @returns DVM_STATE 
     */
    DVM_STATE calRelativeState(DVM_STATE state);

    /**
     * @brief Write DMR frame data to serial port.
     * @param slot DMR slot number.
     * @param[in] data Data to write.
     * @param length Length of data to write.
     */
    void writeDMRData(bool slot, const uint8_t* data, uint8_t length);
    /**
     * @brief Write lost DMR frame data to serial port.
     * @param slot DMR slot number.
     */
    void writeDMRLost(bool slot);

    /**
     * @brief Write P25 frame data to serial port.
     * @param[in] data Data to write.
     * @param length Length of data to write.
     */
    void writeP25Data(const uint8_t* data, uint16_t length);
    /**
     * @brief Write lost P25 frame data to serial port.
     */
    void writeP25Lost();

    /**
     * @brief Write NXDN frame data to serial port.
     * @param[in] data Data to write.
     * @param length Length of data to write.
     */
    void writeNXDNData(const uint8_t* data, uint8_t length);
    /**
     * @brief Write lost NXDN frame data to serial port.
     */
    void writeNXDNLost();
    
    /**
     * @brief Write calibration frame data to serial port.
     * @param[in] data 
     * @param length
     */
    void writeCalData(const uint8_t* data, uint8_t length);
    /**
     * @brief Write RSSI frame data to serial port.
     * @param[in] data Data to write.
     * @param length Length of data to write.
     */
    void writeRSSIData(const uint8_t* data, uint8_t length);

    /**
     * @brief 
     * @param[in] text
     */
    void writeDebug(const char* text);
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     */
    void writeDebug(const char* text, int16_t n1);
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     * @param n2 
     */
    void writeDebug(const char* text, int16_t n1, int16_t n2);
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     * @param n2 
     * @param n3
     */
    void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3);
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     * @param n2 
     * @param n3
     * @param n4 
     */
    void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4);
    /**
     * @brief 
     * @param[in] data 
     * @param length
     */
    void writeDump(const uint8_t* data, uint16_t length);

private:
    uint8_t m_buffer[SERIAL_FB_LEN];
    uint16_t m_ptr;
    uint16_t m_len;
    bool m_dblFrame;

    bool m_debug;

    SerialBuffer m_repeat;

    /**
     * @brief Write acknowlegement.
     */
    void sendACK();
    /**
     * @brief Write negative acknowlegement.
     * @param err 
     */
    void sendNAK(uint8_t err);
    /**
     * @brief Write modem DSP status.
     */
    void getStatus();
    /**
     * @brief Write modem DSP version.
     */
    void getVersion();
    /**
     * @brief Helper to validate the passed modem state is valid.
     * @param state 
     * @returns uint8_t Reason code.
     */
    uint8_t modemStateCheck(DVM_STATE state);
    /**
     * @brief Set modem DSP configuration from serial port data.
     * @param[in] data Buffer containing configuration frame.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t setConfig(const uint8_t* data, uint8_t length);
    /**
     * @brief Set modem DSP mode from serial port data.
     * @param[in] data Buffer containing mode frame.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t setMode(const uint8_t* data, uint8_t length);
    /**
     * @brief Sets the modem state.
     * @param modemState 
     */
    void setMode(DVM_STATE modemState);
    /**
     * @brief Sets the fine-tune symbol levels.
     * @param[in] data Buffer containing symbol level frame.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t setSymbolLvlAdj(const uint8_t* data, uint8_t length);
    /**
     * @brief Sets the software Rx sample level.
     * @param[in] data Buffer containing RX level frame.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t setRXLevel(const uint8_t* data, uint8_t length);
    /**
     * @brief Sets the RF parameters.
     * @param[in] data Buffer containing RF parameters frame.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t setRFParams(const uint8_t* data, uint8_t length);
    /**
     * @brief Sets the protocol ring buffer sizes.
     * @param[in] data Buffer containing set buffers frame.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t setBuffers(const uint8_t* data, uint8_t length);

    /**
     * @brief Reads data from the modem flash parititon.
     */
    void flashRead();
    /**
     * @brief Writes data to the modem flash partition.
     * @param[in] data Buffer containing data to write to flash partition.
     * @param length Length of buffer.
     * @returns uint8_t Reason code.
     */
    uint8_t flashWrite(const uint8_t* data, uint8_t length);

    // Hardware specific routines
    /**
     * @brief 
     * @param n 
     * @param speed 
     */
    void beginInt(uint8_t n, int speed);
    /**
     * @brief 
     * @param n 
     */
    int availableInt(uint8_t n);
    /**
     * @brief 
     * @param n 
     */
    int availableForWriteInt(uint8_t n);
    /**
     * @brief 
     * @param n
     */    
    uint8_t readInt(uint8_t n);
    /**
     * @brief 
     * @param n
     * @param[in] data 
     * @param length
     * @param flush 
     */
    void writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush = false);
};

#endif // __SERIAL_PORT_H__

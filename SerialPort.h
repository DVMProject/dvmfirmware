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
*   Copyright (C) 2018,2021-2024 Bryan Biedenkapp, N2PLL
*
*/
#if !defined(__SERIAL_PORT_H__)
#define __SERIAL_PORT_H__

#include "Defines.h"
#include "Globals.h"
#include "SerialBuffer.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

enum DVM_STATE {
    STATE_IDLE = 0U,
    // DMR
    STATE_DMR = 1U,
    // Project 25
    STATE_P25 = 2U,
    // NXDN
    STATE_NXDN = 3U,

    // CW
    STATE_CW = 10U,

    // Calibration States
    STATE_P25_CAL_1K = 92U,

    STATE_DMR_DMO_CAL_1K = 93U,
    STATE_DMR_CAL_1K = 94U,
    STATE_DMR_LF_CAL = 95U,

    STATE_RSSI_CAL = 96U,

    STATE_P25_CAL = 97U,
    STATE_DMR_CAL = 98U,
    STATE_NXDN_CAL = 99U
};

enum DVM_COMMANDS {
    CMD_GET_VERSION = 0x00U,
    CMD_GET_STATUS = 0x01U,
    CMD_SET_CONFIG = 0x02U,
    CMD_SET_MODE = 0x03U,

    CMD_SET_SYMLVLADJ = 0x04U,
    CMD_SET_RXLEVEL = 0x05U,
    CMD_SET_RFPARAMS = 0x06U,

    CMD_CAL_DATA = 0x08U,
    CMD_RSSI_DATA = 0x09U,

    CMD_SEND_CWID = 0x0AU,

    CMD_SET_BUFFERS = 0x0FU,

    CMD_DMR_DATA1 = 0x18U,
    CMD_DMR_LOST1 = 0x19U,
    CMD_DMR_DATA2 = 0x1AU,
    CMD_DMR_LOST2 = 0x1BU,
    CMD_DMR_SHORTLC = 0x1CU,
    CMD_DMR_START = 0x1DU,
    CMD_DMR_ABORT = 0x1EU,
    CMD_DMR_CACH_AT_CTRL = 0x1FU,
    CMD_DMR_CLEAR1 = 0x20U,
    CMD_DMR_CLEAR2 = 0x21U,

    CMD_P25_DATA = 0x31U,
    CMD_P25_LOST = 0x32U,
    CMD_P25_CLEAR = 0x33U,

    CMD_NXDN_DATA = 0x41U,
    CMD_NXDN_LOST = 0x42U,
    CMD_NXDN_CLEAR = 0x43U,

    CMD_ACK = 0x70U,
    CMD_NAK = 0x7FU,

    CMD_FLSH_READ = 0xE0U,
    CMD_FLSH_WRITE = 0xE1U,

    CMD_DEBUG1 = 0xF1U,
    CMD_DEBUG2 = 0xF2U,
    CMD_DEBUG3 = 0xF3U,
    CMD_DEBUG4 = 0xF4U,
    CMD_DEBUG5 = 0xF5U,
    CMD_DEBUG_DUMP = 0xFAU,
};

enum CMD_REASON_CODE {
    RSN_OK = 0U,
    RSN_NAK = 1U,

    RSN_ILLEGAL_LENGTH = 2U,
    RSN_INVALID_REQUEST = 4U,
    RSN_RINGBUFF_FULL = 8U,

    RSN_INVALID_FDMA_PREAMBLE = 10U,
    RSN_INVALID_MODE = 11U,
    
    RSN_INVALID_DMR_CC = 12U,
    RSN_INVALID_DMR_SLOT = 13U,
    RSN_INVALID_DMR_START = 14U,
    RSN_INVALID_DMR_RX_DELAY = 15U,

    RSN_INVALID_P25_CORR_COUNT = 16U,

    RSN_NO_INTERNAL_FLASH = 20U,
    RSN_FAILED_ERASE_FLASH = 21U,
    RSN_FAILED_WRITE_FLASH = 22U,
    RSN_FLASH_WRITE_TOO_BIG = 23U,

    RSN_DMR_DISABLED = 63U,
    RSN_P25_DISABLED = 64U,
    RSN_NXDN_DISABLED = 65U
};

const uint8_t DVM_SHORT_FRAME_START = 0xFEU;
const uint8_t DVM_LONG_FRAME_START = 0xFDU;

#define SERIAL_SPEED 115200

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements the RS232 serial bus to communicate with the HOST S/W.
// ---------------------------------------------------------------------------

class DSP_FW_API SerialPort {
public:
    /// <summary>Initializes a new instance of the SerialPort class.</summary>
    SerialPort();

    /// <summary>Starts serial port communications.</summary>
    void start();

    /// <summary>Process data from serial port.</summary>
    void process();

    ///  <summary>Helper to check if the modem is in a calibration state.</summary>
    bool isCalState(DVM_STATE state);
    ///  <summary>Helper to determine digital mode if the modem is in a calibration state.</summary>
    DVM_STATE calRelativeState(DVM_STATE state);

    /// <summary>Write DMR frame data to serial port.</summary>
    void writeDMRData(bool slot, const uint8_t* data, uint8_t length);
    /// <summary>Write lost DMR frame data to serial port.</summary>
    void writeDMRLost(bool slot);

    /// <summary>Write P25 frame data to serial port.</summary>
    void writeP25Data(const uint8_t* data, uint16_t length);
    /// <summary>Write lost P25 frame data to serial port.</summary>
    void writeP25Lost();

    /// <summary>Write NXDN frame data to serial port.</summary>
    void writeNXDNData(const uint8_t* data, uint8_t length);
    /// <summary>Write lost NXDN frame data to serial port.</summary>
    void writeNXDNLost();
    
    /// <summary>Write calibration frame data to serial port.</summary>
    void writeCalData(const uint8_t* data, uint8_t length);
    /// <summary>Write RSSI frame data to serial port.</summary>
    void writeRSSIData(const uint8_t* data, uint8_t length);

    /// <summary></summary>
    void writeDebug(const char* text);
    /// <summary></summary>
    void writeDebug(const char* text, int16_t n1);
    /// <summary></summary>
    void writeDebug(const char* text, int16_t n1, int16_t n2);
    /// <summary></summary>
    void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3);
    /// <summary></summary>
    void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4);
    /// <summary></summary>
    void writeDump(const uint8_t* data, uint16_t length);

private:
    uint8_t m_buffer[256U];
    uint8_t m_ptr;
    uint8_t m_len;
    bool m_dblFrame;

    bool m_debug;

    SerialBuffer m_repeat;

    /// <summary>Write acknowlegement.</summary>
    void sendACK();
    /// <summary>Write negative acknowlegement.</summary>
    void sendNAK(uint8_t err);
    /// <summary>Write modem DSP status.</summary>
    void getStatus();
    /// <summary>Write modem DSP version.</summary>
    void getVersion();
    ///  <summary>Helper to validate the passed modem state is valid.</summary>
    uint8_t modemStateCheck(DVM_STATE state);
    /// <summary>Set modem DSP configuration from serial port data.</summary>
    uint8_t setConfig(const uint8_t* data, uint8_t length);
    /// <summary>Set modem DSP mode from serial port data.</summary>
    uint8_t setMode(const uint8_t* data, uint8_t length);
    /// <summary>Sets the modem state.</summary>
    void setMode(DVM_STATE modemState);
    /// <summary>Sets the fine-tune symbol levels.</summary>
    uint8_t setSymbolLvlAdj(const uint8_t* data, uint8_t length);
    /// <summary>Sets the software Rx sample level.</summary>
    uint8_t setRXLevel(const uint8_t* data, uint8_t length);
    /// <summary>Sets the RF parameters.</summary>
    uint8_t setRFParams(const uint8_t* data, uint8_t length);
    /// <summary>Sets the protocol ring buffer sizes.</summary>
    uint8_t setBuffers(const uint8_t* data, uint8_t length);

    /// <summary></summary>
    void flashRead();
    /// <summary></summary>
    uint8_t flashWrite(const uint8_t* data, uint8_t length);

    // Hardware specific routines
    /// <summary></summary>
    void beginInt(uint8_t n, int speed);
    /// <summary></summary>
    int availableInt(uint8_t n);
    /// <summary></summary>
    int availableForWriteInt(uint8_t n);
    /// <summary></summary>
    uint8_t readInt(uint8_t n);
    /// <summary></summary>
    void writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush = false);
};

#endif // __SERIAL_PORT_H__

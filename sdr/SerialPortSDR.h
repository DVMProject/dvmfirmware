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
 * @file SerialPortSDR.h
 * @ingroup modem_fw
 * @file SerialPortSDR.cpp
 * @ingroup modem_fw
 */
#if !defined(__SERIAL_PORT_SDR_H__)
#define __SERIAL_PORT_SDR_H__

#include "Defines.h"
#include "SerialPort.h"

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Implements the RS232 serial bus to communicate with the HOST S/W.
 * @ingroup modem_fw
 */
class DSP_FW_API SerialPortSDR : public SerialPort {
public:
    /**
     * @brief Initializes a new instance of the SerialPortSDR class.
     */
    SerialPortSDR();

    /**
     * @brief 
     * @param[in] text
     */
    void writeDebug(const char* text) override;
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     */
    void writeDebug(const char* text, int16_t n1) override;
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     * @param n2 
     */
    void writeDebug(const char* text, int16_t n1, int16_t n2) override;
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     * @param n2 
     * @param n3
     */
    void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3) override;
    /**
     * @brief 
     * @param[in] text
     * @param n1 
     * @param n2 
     * @param n3
     * @param n4 
     */
    void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4) override;
    /**
     * @brief 
     * @param[in] data 
     * @param length
     */
    void writeDump(const uint8_t* data, uint16_t length) override;
};

#endif // __SERIAL_PORT_SDR_H__

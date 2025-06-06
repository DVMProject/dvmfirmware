// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2013,2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2017-2024 Bryan Biedenkapp, N2PLL
 *
 */
#include "Globals.h"
#include "SerialPort.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#define concat(a, b, c) a " (build " b " " c ")"
const char HARDWARE[] = concat(DESCRIPTION, __TIME__, __DATE__);

const uint8_t PROTOCOL_VERSION = 4U;

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Initializes a new instance of the SerialPort class. */

SerialPort::SerialPort() :
    m_buffer(),
    m_ptr(0U),
    m_len(0U),
    m_dblFrame(false),
    m_debug(false),
    m_repeat()
{
    // stub
}

/* Starts serial port communications. */

void SerialPort::start()
{
    beginInt(1U, SERIAL_SPEED);
}

/* Process data from serial port. */

void SerialPort::process()
{
    while (availableInt(1U)) {
        uint8_t c = readInt(1U);

        if (m_ptr == 0U) {
            if (c == DVM_SHORT_FRAME_START) {
                // Handle the frame start correctly
                m_buffer[0U] = c;
                m_ptr = 1U;
                m_len = 0U;
                m_dblFrame = false;
            }
            else if (c == DVM_LONG_FRAME_START) {
                // Handle the frame start correctly
                m_buffer[0U] = c;
                m_ptr = 1U;
                m_len = 0U;
                m_dblFrame = true;
            }
            else {
                m_ptr = 0U;
                m_len = 0U;
                m_dblFrame = false;
            }
        }
        else if (m_ptr == 1U) {
            // Handle the frame length
            if (m_dblFrame) {
                m_buffer[m_ptr] = c;
                m_len = ((c & 0xFFU) << 8);
                // DEBUG3("long frame, len msb", m_len, c);
            } else {
                m_len = m_buffer[m_ptr] = c;
                // DEBUG2("short frame, len", m_len);
            }
            m_ptr = 2U;
        }
        else if (m_ptr == 2U && m_dblFrame) {
            // Handle the frame length
            m_buffer[m_ptr] = c;
            m_len = (m_len + (c & 0xFFU));
            if (m_len > SERIAL_FB_LEN)
                m_len = SERIAL_FB_LEN; // don't allow length to be longer then the buffer
            // DEBUG3("long frame, len lsb", m_len, c);
            m_ptr = 3U;
        }
        else {
            // Any other bytes are added to the buffer
            m_buffer[m_ptr] = c;
            m_ptr++;

            // The full packet has been received, process it
            if (m_ptr == m_len) {
                uint8_t err = 2U;
                uint8_t offset = 2U;
                if (m_dblFrame)
                    offset = 3U;

                // DEBUG4("m_buffer [b0 - b2]", m_buffer[0], m_buffer[1], m_buffer[2]);
                // DEBUG4("m_buffer [b3 - b5]", m_buffer[3], m_buffer[4], m_buffer[5]);

                switch (m_buffer[offset]) {
                case CMD_GET_STATUS:
                    getStatus();
                    break;

                case CMD_GET_VERSION:
                    getVersion();
                    break;

                case CMD_SET_CONFIG:
                    err = setConfig(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK)
                        sendACK();
                    else
                        sendNAK(err);
                    break;

                case CMD_SET_MODE:
                    err = setMode(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK)
                        sendACK();
                    else
                        sendNAK(err);
                    break;

                case CMD_SET_SYMLVLADJ:
                    err = setSymbolLvlAdj(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK)
                        sendACK();
                    else
                        sendNAK(err);
                    break;

                case CMD_SET_RXLEVEL:
                    err = setRXLevel(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK)
                        sendACK();
                    else
                        sendNAK(err);
                    break;

                case CMD_SET_RFPARAMS:
                    err = setRFParams(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK)
                        sendACK();
                    else
                        sendNAK(err);
                    break;

                case CMD_CAL_DATA:
                    if (m_modemState == STATE_DMR_DMO_CAL_1K || m_modemState == STATE_DMR_CAL_1K ||
                        m_modemState == STATE_DMR_LF_CAL || m_modemState == STATE_DMR_CAL)
                        err = calDMR.write(m_buffer + 3U, m_len - 3U);
                    if (m_modemState == STATE_P25_CAL_1K || m_modemState == STATE_P25_CAL)
                        err = calP25.write(m_buffer + 3U, m_len - 3U);
                    if (m_modemState == STATE_NXDN_CAL)
                        err = calNXDN.write(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK) {
                        sendACK();
                    }
                    else {
                        DEBUG2("SerialPort::process() received invalid calibration data", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_FLSH_READ:
                    flashRead();
                    break;

                case CMD_FLSH_WRITE:
                    err = flashWrite(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK) {
                        sendACK();
                    }
                    else {
                        DEBUG2("SerialPort::process() received invalid data to write to flash", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_RESET_MCU:
                    io.resetMCU();
                    break;

                case CMD_SET_BUFFERS:
                    err = setBuffers(m_buffer + 3U, m_len - 3U);
                    if (err == RSN_OK) {
                        sendACK();
                    }
                    else {
                        DEBUG2("SerialPort::process() received invalid data to set buffers", err);
                        sendNAK(err);
                    }
                    break;

                /** CW */
                case CMD_SEND_CWID:
                    err = RSN_RINGBUFF_FULL;
                    if (m_modemState == STATE_IDLE)
                        err = cwIdTX.write(m_buffer + 3U, m_len - 3U);
                    if (err != RSN_OK) {
                        DEBUG2("SerialPort::process() invalid CW Id data", err);
                        sendNAK(err);
                    }
                    break;

                /** Digital Mobile Radio */
                case CMD_DMR_DATA1:
                    if (m_dmrEnable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_DMR) {
                            if (m_duplex)
                                err = dmrTX.writeData1(m_buffer + 3U, m_len - 3U);
                        }
                    }
                    if (err == RSN_OK) {
                        if (m_modemState == STATE_IDLE)
                            setMode(STATE_DMR);
                    }
                    else {
                        DEBUG2("SerialPort: process() received invalid DMR data", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_DMR_DATA2:
                    if (m_dmrEnable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_DMR) {
                            if (m_duplex)
                                err = dmrTX.writeData2(m_buffer + 3U, m_len - 3U);
                            else
                                err = dmrDMOTX.writeData(m_buffer + 3U, m_len - 3U);
                        }
                    }
                    if (err == RSN_OK) {
                        if (m_modemState == STATE_IDLE)
                            setMode(STATE_DMR);
                    }
                    else {
                        DEBUG2("SerialPort::process() received invalid DMR data", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_DMR_START:
                    if (m_dmrEnable) {
                        err = RSN_INVALID_DMR_START;
                        if (m_len == 4U) {
                            if (m_buffer[3U] == 0x01U && m_modemState == STATE_DMR) {
                                if (!m_tx)
                                    dmrTX.setStart(true);
                                err = RSN_OK;
                            }
                            else if (m_buffer[3U] == 0x00U && m_modemState == STATE_DMR) {
                                if (m_tx)
                                    dmrTX.setStart(false);
                                err = RSN_OK;
                            }
                        }
                    }
                    if (err != RSN_OK) {
                        DEBUG3("SerialPort::process() received invalid DMR start", err, m_len);
                        sendNAK(err);
                    }
                    break;

                case CMD_DMR_SHORTLC:
                    if (m_dmrEnable)
                        err = dmrTX.writeShortLC(m_buffer + 3U, m_len - 3U);
                    if (err != RSN_OK) {
                        DEBUG2("SerialPort::process() received invalid DMR Short LC", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_DMR_ABORT:
                    if (m_dmrEnable)
                        err = dmrTX.writeAbort(m_buffer + 3U, m_len - 3U);
                    if (err != RSN_OK) {
                        DEBUG2("SerialPort::process() received invalid DMR Abort", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_DMR_CACH_AT_CTRL:
                    if (m_dmrEnable) {
                        err = RSN_INVALID_REQUEST;
                        if (m_len == 4U) {
                            dmrTX.setIgnoreCACH_AT(m_buffer[3U]);
                            err = RSN_OK;
                        }
                    }
                    if (err != RSN_OK) {
                        DEBUG2("SerialPort::process() received invalid DMR CACH AT Control", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_DMR_CLEAR1:
                    if (m_dmrEnable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_P25)
                            dmrTX.resetFifo1();
                    }
                    break;
                case CMD_DMR_CLEAR2:
                    if (m_dmrEnable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_P25)
                            dmrTX.resetFifo2();
                    }
                    break;

                /** Project 25 */
                case CMD_P25_DATA:
                    if (m_p25Enable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_P25) {
                            if (m_dblFrame)
                                err = p25TX.writeData(m_buffer + 4U, m_len - 4U);
                            else
                                err = p25TX.writeData(m_buffer + 3U, m_len - 3U);
                        }
                    }
                    if (err == RSN_OK) {
                        if (m_modemState == STATE_IDLE)
                            setMode(STATE_P25);
                    }
                    else {
                        DEBUG2("SerialPort::process() received invalid P25 data", err);
                        sendNAK(err);
                    }
                    break;

                case CMD_P25_CLEAR:
                    if (m_p25Enable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_P25)
                            p25TX.clear();
                    }
                    break;

                /** Next Generation Digital Narrowband */
                case CMD_NXDN_DATA:
                    if (m_nxdnEnable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_NXDN)
                            err = nxdnTX.writeData(m_buffer + 3U, m_len - 3U);
                    }
                    if (err == RSN_OK) {
                        if (m_modemState == STATE_IDLE)
                            setMode(STATE_NXDN);
                    }
                    else {
                        DEBUG2("SerialPort::process() received invalid NXDN data", err);
                        sendNAK(err);
                    }
                    break;
                case CMD_NXDN_CLEAR:
                    if (m_nxdnEnable) {
                        if (m_modemState == STATE_IDLE || m_modemState == STATE_P25)
                            nxdnTX.clear();
                    }
                    break;

                default:
                    // Handle this, send a NAK back
                    sendNAK(RSN_NAK);
                    break;
                }

                m_ptr = 0U;
                m_len = 0U;
                m_dblFrame = false;
            }
        }
    }

    if (io.getWatchdog() >= 48000U) {
        m_ptr = 0U;
        m_len = 0U;
        m_dblFrame = false;
    }
}

/* Helper to check if the modem is in a calibration state. */

bool SerialPort::isCalState(DVM_STATE state)
{
    // calibration mode check
    if (state == STATE_P25_CAL_1K ||
        state == STATE_DMR_DMO_CAL_1K || state == STATE_DMR_CAL_1K ||
        state == STATE_DMR_LF_CAL ||
        state == STATE_RSSI_CAL ||
        state == STATE_P25_CAL || state == STATE_DMR_CAL || state == STATE_NXDN_CAL) {
        return true;
    }

    return false;
}

/* Helper to determine digital mode if the modem is in a calibration state. */

DVM_STATE SerialPort::calRelativeState(DVM_STATE state)
{
    if (isCalState(state)) {
        if (state == STATE_DMR_DMO_CAL_1K || state == STATE_DMR_CAL_1K ||
            state == STATE_DMR_LF_CAL || state == STATE_DMR_CAL ||
            state == STATE_RSSI_CAL) {
            return STATE_DMR;
        } else if (state == STATE_P25_CAL_1K ||
            state == STATE_P25_CAL) {
            return STATE_P25;
        } else if (state == STATE_NXDN_CAL) {
            return STATE_NXDN;
        }
    }

    return STATE_CW;
}

/* Write DMR frame data to serial port. */

void SerialPort::writeDMRData(bool slot, const uint8_t* data, uint8_t length)
{
    if (m_modemState != STATE_DMR && m_modemState != STATE_IDLE)
        return;

    if (!m_dmrEnable)
        return;

    if (length + 3U > 40U) {
        m_buffer[2U] = slot ? CMD_DMR_DATA2 : CMD_DMR_DATA1;
        sendNAK(RSN_ILLEGAL_LENGTH);
        return;
    }

    uint8_t reply[40U];
    ::memset(reply, 0x00U, 40U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = length + 3U;
    reply[2U] = slot ? CMD_DMR_DATA2 : CMD_DMR_DATA1;

    ::memcpy(reply + 3U, data, length);

    writeInt(1U, reply, length + 3U);
}

/* Write lost DMR frame data to serial port. */

void SerialPort::writeDMRLost(bool slot)
{
    if (m_modemState != STATE_DMR && m_modemState != STATE_IDLE)
        return;

    if (!m_dmrEnable)
        return;

    uint8_t reply[3U];

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 3U;
    reply[2U] = slot ? CMD_DMR_LOST2 : CMD_DMR_LOST1;

    writeInt(1U, reply, 3);
}

/* Write P25 frame data to serial port. */

void SerialPort::writeP25Data(const uint8_t* data, uint16_t length)
{
    if (m_modemState != STATE_P25 && m_modemState != STATE_IDLE)
        return;

    if (!m_p25Enable)
        return;

    if (length + 4U > 520U) {
        m_buffer[2U] = CMD_P25_DATA;
        sendNAK(RSN_ILLEGAL_LENGTH);
        return;
    }

    uint8_t reply[520U];
    ::memset(reply, 0x00U, 520U);

    if (length < 252U) {
        reply[0U] = DVM_SHORT_FRAME_START;
        reply[1U] = length + 3U;
        reply[2U] = CMD_P25_DATA;
        ::memcpy(reply + 3U, data, length);

        writeInt(1U, reply, length + 3U);
    }
    else {
        length += 4U;
        reply[0U] = DVM_LONG_FRAME_START;
        reply[1U] = (length >> 8U) & 0xFFU;
        reply[2U] = (length & 0xFFU);
        reply[3U] = CMD_P25_DATA;
        ::memcpy(reply + 4U, data, length);

        writeInt(1U, reply, length + 4U);
    }
}

/* Write lost P25 frame data to serial port. */

void SerialPort::writeP25Lost()
{
    if (m_modemState != STATE_P25 && m_modemState != STATE_IDLE)
        return;

    if (!m_p25Enable)
        return;

    uint8_t reply[3U];

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 3U;
    reply[2U] = CMD_P25_LOST;

    writeInt(1U, reply, 3);
}

/* Write NXDN frame data to serial port. */

void SerialPort::writeNXDNData(const uint8_t* data, uint8_t length)
{
    if (m_modemState != STATE_NXDN && m_modemState != STATE_IDLE)
        return;

    if (!m_nxdnEnable)
        return;

    if (length + 3U > 130U) {
        m_buffer[2U] = CMD_NXDN_DATA;
        sendNAK(RSN_ILLEGAL_LENGTH);
        return;
    }

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = length + 3U;
    reply[2U] = CMD_NXDN_DATA;

    ::memcpy(reply + 3U, data, length);

    writeInt(1U, reply, length + 3U);
}

/* Write lost NXDN frame data to serial port. */

void SerialPort::writeNXDNLost()
{
    if (m_modemState != STATE_NXDN && m_modemState != STATE_IDLE)
        return;

    if (!m_nxdnEnable)
        return;

    uint8_t reply[3U];

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 3U;
    reply[2U] = CMD_NXDN_LOST;

    writeInt(1U, reply, 3);
}

/* Write calibration frame data to serial port. */

void SerialPort::writeCalData(const uint8_t* data, uint8_t length)
{
    if (length + 3U > 130U) {
        m_buffer[2U] = CMD_CAL_DATA;
        sendNAK(RSN_ILLEGAL_LENGTH);
        return;
    }

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = length + 3U;
    reply[2U] = CMD_CAL_DATA;

    ::memcpy(reply + 3U, data, length);

    writeInt(1U, reply, length + 3U);
}

/* Write RSSI frame data to serial port. */

void SerialPort::writeRSSIData(const uint8_t* data, uint8_t length)
{
    if (m_modemState != STATE_RSSI_CAL)
        return;

    if (length + 3U > 130U) {
        m_buffer[2U] = CMD_RSSI_DATA;
        sendNAK(RSN_ILLEGAL_LENGTH);
        return;
    }

    uint8_t reply[30U];
    ::memset(reply, 0x00U, 30U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = length + 3U;
    reply[2U] = CMD_RSSI_DATA;

    ::memcpy(reply + 3U, data, length);

    writeInt(1U, reply, length + 3U);
}

/* */

void SerialPort::writeDebug(const char* text)
{
    if (!m_debug)
        return;

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 0U;
    reply[2U] = CMD_DEBUG1;

    uint8_t count = 3U;
    for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
        reply[count] = text[i];

    reply[1U] = count;

    writeInt(1U, reply, count, true);
}

/* */

void SerialPort::writeDebug(const char* text, int16_t n1)
{
    if (!m_debug)
        return;

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 0U;
    reply[2U] = CMD_DEBUG2;

    uint8_t count = 3U;
    for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
        reply[count] = text[i];

    reply[count++] = (n1 >> 8) & 0xFF;
    reply[count++] = (n1 >> 0) & 0xFF;

    reply[1U] = count;

    writeInt(1U, reply, count, true);
}

/* */

void SerialPort::writeDebug(const char* text, int16_t n1, int16_t n2)
{
    if (!m_debug)
        return;

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 0U;
    reply[2U] = CMD_DEBUG3;

    uint8_t count = 3U;
    for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
        reply[count] = text[i];

    reply[count++] = (n1 >> 8) & 0xFF;
    reply[count++] = (n1 >> 0) & 0xFF;

    reply[count++] = (n2 >> 8) & 0xFF;
    reply[count++] = (n2 >> 0) & 0xFF;

    reply[1U] = count;

    writeInt(1U, reply, count, true);
}

/* */

void SerialPort::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3)
{
    if (!m_debug)
        return;

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 0U;
    reply[2U] = CMD_DEBUG4;

    uint8_t count = 3U;
    for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
        reply[count] = text[i];

    reply[count++] = (n1 >> 8) & 0xFF;
    reply[count++] = (n1 >> 0) & 0xFF;

    reply[count++] = (n2 >> 8) & 0xFF;
    reply[count++] = (n2 >> 0) & 0xFF;

    reply[count++] = (n3 >> 8) & 0xFF;
    reply[count++] = (n3 >> 0) & 0xFF;

    reply[1U] = count;

    writeInt(1U, reply, count, true);
}

/* */

void SerialPort::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4)
{
    if (!m_debug)
        return;

    uint8_t reply[130U];
    ::memset(reply, 0x00U, 130U);

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 0U;
    reply[2U] = CMD_DEBUG5;

    uint8_t count = 3U;
    for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
        reply[count] = text[i];

    reply[count++] = (n1 >> 8) & 0xFF;
    reply[count++] = (n1 >> 0) & 0xFF;

    reply[count++] = (n2 >> 8) & 0xFF;
    reply[count++] = (n2 >> 0) & 0xFF;

    reply[count++] = (n3 >> 8) & 0xFF;
    reply[count++] = (n3 >> 0) & 0xFF;

    reply[count++] = (n4 >> 8) & 0xFF;
    reply[count++] = (n4 >> 0) & 0xFF;

    reply[1U] = count;

    writeInt(1U, reply, count, true);
}

/* */

void SerialPort::writeDump(const uint8_t* data, uint16_t length)
{
    if (!m_debug)
        return;

    if (length + 4U > 516U) {
        m_buffer[2U] = CMD_DEBUG_DUMP;
        sendNAK(RSN_ILLEGAL_LENGTH);
        return;
    }

    uint8_t reply[516U];
    ::memset(reply, 0x00U, 516U);

    if (length > 252U) {
        reply[0U] = DVM_LONG_FRAME_START;
        reply[1U] = (length >> 8U) & 0xFFU;
        reply[2U] = (length & 0xFFU);
        reply[3U] = CMD_DEBUG_DUMP;

        for (uint8_t i = 0U; i < length; i++)
            reply[i + 4U] = data[i];

        writeInt(1U, reply, length + 4U);
    }
    else {
        reply[0U] = DVM_SHORT_FRAME_START;
        reply[1U] = length + 3U;
        reply[2U] = CMD_DEBUG_DUMP;

        for (uint8_t i = 0U; i < length; i++)
            reply[i + 3U] = data[i];

        writeInt(1U, reply, length + 3U);
    }
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Write acknowlegement. */

void SerialPort::sendACK()
{
    uint8_t reply[4U];

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 4U;
    reply[2U] = CMD_ACK;
    reply[3U] = m_buffer[2U];

    writeInt(1U, reply, 4);
}

/* Write negative acknowlegement. */

void SerialPort::sendNAK(uint8_t err)
{
    uint8_t reply[5U];

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 5U;
    reply[2U] = CMD_NAK;
    reply[3U] = m_buffer[2U];
    reply[4U] = err;

    writeInt(1U, reply, 5);
}

/* Write modem DSP status. */

void SerialPort::getStatus()
{
    io.resetWatchdog();

    uint8_t reply[15U];

    // send all sorts of interesting internal values
    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 12U;
    reply[2U] = CMD_GET_STATUS;

    reply[3U] = 0x00U;
    if (m_dmrEnable)
        reply[3U] |= 0x02U;
    if (m_p25Enable)
        reply[3U] |= 0x08U;
    if (m_nxdnEnable)
        reply[3U] |= 0x10U;

    reply[4U] = uint8_t(m_modemState);

    reply[5U] = m_tx ? 0x01U : 0x00U;

    bool adcOverflow;
    bool dacOverflow;
    io.getOverflow(adcOverflow, dacOverflow);

    if (adcOverflow)
        reply[5U] |= 0x02U;

    if (io.hasRXOverflow())
        reply[5U] |= 0x04U;

    if (io.hasTXOverflow())
        reply[5U] |= 0x08U;

    if (io.hasLockout())
        reply[5U] |= 0x10U;

    if (dacOverflow)
        reply[5U] |= 0x20U;

    reply[5U] |= m_dcd ? 0x40U : 0x00U;

    reply[6U] = 0U;

    if (m_dmrEnable) {
        if (m_duplex) {
            reply[7U] = dmrTX.getSpace1();
            reply[8U] = dmrTX.getSpace2();
        }
        else {
            reply[7U] = 10U;
            reply[8U] = dmrDMOTX.getSpace();
        }
    }
    else {
        reply[7U] = 0U;
        reply[8U] = 0U;
    }

    reply[9U] = 0U;

    if (m_p25Enable)
        reply[10U] = p25TX.getSpace();
    else
        reply[10U] = 0U;

    if (m_nxdnEnable)
        reply[11U] = nxdnTX.getSpace();
    else
        reply[11U] = 0U;

    writeInt(1U, reply, 12);
}

/* Write modem DSP version. */

void SerialPort::getVersion()
{
    uint8_t reply[200U];

    reply[0U] = DVM_SHORT_FRAME_START;
    reply[1U] = 0U;
    reply[2U] = CMD_GET_VERSION;

    reply[3U] = PROTOCOL_VERSION;

    reply[4U] = io.getCPU();

    // Reserve 16 bytes for the UDID
    ::memset(reply + 5U, 0x00U, 16U);
    io.getUDID(reply + 5U);

    uint8_t count = 21U;
    for (uint8_t i = 0U; HARDWARE[i] != 0x00U; i++, count++)
        reply[count] = HARDWARE[i];

    reply[1U] = count;

    writeInt(1U, reply, count);
}

/* Helper to validate the passed modem state is valid. */

uint8_t SerialPort::modemStateCheck(DVM_STATE state)
{
    // invalid mode check
    if (state != STATE_IDLE && state != STATE_DMR && state != STATE_P25 && state != STATE_NXDN && 
        state != STATE_P25_CAL_1K &&
        state != STATE_DMR_DMO_CAL_1K && state != STATE_DMR_CAL_1K &&
        state != STATE_DMR_LF_CAL &&
        state != STATE_RSSI_CAL &&
        state != STATE_P25_CAL && state != STATE_DMR_CAL &&
        state != STATE_NXDN_CAL)
        return RSN_INVALID_MODE;
/*
    // DMR without DMR being enabled
    if (state == STATE_DMR && !m_dmrEnable)
        return RSN_DMR_DISABLED;
    // P25 without P25 being enabled
    if (state == STATE_P25 && !m_p25Enable)
        return RSN_P25_DISABLED;
    // NXDN without NXDN being enabled
    if (state == STATE_NXDN && !m_nxdnEnable)
        return RSN_NXDN_DISABLED;
*/
    return RSN_OK;
}

/* Set modem DSP configuration from serial port data. */

uint8_t SerialPort::setConfig(const uint8_t* data, uint8_t length)
{
    if (length < 21U)
        return RSN_ILLEGAL_LENGTH;

    bool rxInvert = (data[0U] & 0x01U) == 0x01U;
    bool txInvert = (data[0U] & 0x02U) == 0x02U;
    bool pttInvert = (data[0U] & 0x04U) == 0x04U;
    bool simplex = (data[0U] & 0x80U) == 0x80U;

    m_debug = (data[0U] & 0x10U) == 0x10U;

    bool dcBlockerEnable = (data[1U] & 0x01U) == 0x01U;
    bool cosLockoutEnable = (data[1U] & 0x04U) == 0x04U;

    bool dmrEnable = (data[1U] & 0x02U) == 0x02U;
    bool p25Enable = (data[1U] & 0x08U) == 0x08U;
    bool nxdnEnable = (data[1U] & 0x10U) == 0x10U;

    uint8_t fdmaPreamble = data[2U];
    if (fdmaPreamble > 255U)
        return RSN_INVALID_FDMA_PREAMBLE;

    DVM_STATE modemState = DVM_STATE(data[3U]);

    uint8_t ret = modemStateCheck(modemState);
    if (ret != RSN_OK)
        return ret;

    uint8_t rxLevel = data[4U];

    uint8_t colorCode = data[6U];
    if (colorCode > 15U)
        return RSN_INVALID_DMR_CC;

    uint8_t dmrRxDelay = data[7U];
    if (dmrRxDelay > 255U)
        return RSN_INVALID_DMR_RX_DELAY;

    uint16_t nac = (data[8U] << 4) + (data[9U] >> 4);

    uint8_t cwIdTXLevel = data[5U];
    uint8_t dmrTXLevel = data[10U];
    uint8_t p25TXLevel = data[12U];
    uint8_t nxdnTXLevel = data[15U];

    int16_t txDCOffset = int16_t(data[13U]) - 128;
    int16_t rxDCOffset = int16_t(data[14U]) - 128;

    uint8_t p25CorrCount = data[11U];
    if (p25CorrCount > 255U)
        return RSN_INVALID_P25_CORR_COUNT;

    //uint8_t nxdnCorrCount = data[22U];

    m_modemState = modemState;

    m_dcBlockerEnable = dcBlockerEnable;
    m_cosLockoutEnable = cosLockoutEnable;

    m_dmrEnable = dmrEnable;
    m_p25Enable = p25Enable;
    m_nxdnEnable = nxdnEnable;
    m_duplex = !simplex;

    p25TX.setPreambleCount(fdmaPreamble);
    dmrDMOTX.setPreambleCount(fdmaPreamble);
    //nxdnTX.setPreambleCount(fdmaPreamble);

    p25RX.setNAC(nac);
    p25RX.setCorrCount(p25CorrCount);

    dmrTX.setColorCode(colorCode);
    dmrRX.setColorCode(colorCode);
    dmrRX.setRxDelay(dmrRxDelay);
    dmrDMORX.setColorCode(colorCode);
    dmrIdleRX.setColorCode(colorCode);

    //nxdnRX.setCorrCount(nxdnCorrCount);

#if defined(DIGIPOT_ENABLED)
    uint8_t rxCoarse = data[16U];
    uint8_t rxFine = data[17U];

    uint8_t txCoarse = data[18U];
    uint8_t txFine = data[19U];

    uint8_t rssiCoarse = data[20U];
    uint8_t rssiFine = data[21U];

    io.SetTxDigipot(txCoarse);
    io.SetRxDigipot(rxCoarse);
    io.SetRsDigipot(rssiCoarse);
#endif

    io.setParameters(rxInvert, txInvert, pttInvert, rxLevel, cwIdTXLevel, dmrTXLevel, p25TXLevel, nxdnTXLevel, txDCOffset, rxDCOffset);

    setMode(m_modemState);

    io.start();

    return RSN_OK;
}

/* Set modem DSP mode from serial port data. */

uint8_t SerialPort::setMode(const uint8_t* data, uint8_t length)
{
    if (length < 1U)
        return RSN_ILLEGAL_LENGTH;

    DVM_STATE modemState = DVM_STATE(data[0U]);

    if (modemState == m_modemState)
        return RSN_OK;

    uint8_t ret = modemStateCheck(modemState);
    if (ret != RSN_OK)
        return ret;

    setMode(modemState);

    return RSN_OK;
}

/* Sets the modem state. */

void SerialPort::setMode(DVM_STATE modemState)
{
    switch (modemState) {
    case STATE_DMR:
        DEBUG1("SerialPort::setMode() mode set to DMR");
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_P25:
        DEBUG1("SerialPort::setMode() mode set to P25");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_NXDN:
        DEBUG1("SerialPort::setMode() mode set to NXDN");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_DMR_CAL:
        DEBUG1("SerialPort::setMode() mode set to DMR Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_P25_CAL:
        DEBUG1("SerialPort::setMode() mode set to P25 Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_NXDN_CAL:
        DEBUG1("SerialPort::setMode() mode set to NXDN Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_RSSI_CAL:
        DEBUG1("SerialPort::setMode() mode set to RSSI Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_DMR_LF_CAL:
        DEBUG1("SerialPort::setMode() mode set to DMR 80Hz Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_DMR_CAL_1K:
        DEBUG1("SerialPort::setMode() mode set to DMR BS 1031Hz Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_DMR_DMO_CAL_1K:
        DEBUG1("SerialPort::setMode() mode set to DMR MS 1031Hz Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    case STATE_P25_CAL_1K:
        DEBUG1("SerialPort::setMode() mode set to P25 1011Hz Calibrate");
        dmrIdleRX.reset();
        dmrDMORX.reset();
        dmrRX.reset();
        p25RX.reset();
        nxdnRX.reset();
        cwIdTX.reset();
        break;
    default:
        DEBUG1("SerialPort::setMode() mode set to Idle");
        // STATE_IDLE
        break;
    }

    m_modemState = modemState;

    io.setMode();
}

/* Sets the fine-tune symbol levels. */

uint8_t SerialPort::setSymbolLvlAdj(const uint8_t* data, uint8_t length)
{
    if (length < 6U)
        return RSN_ILLEGAL_LENGTH;

    int8_t dmrSymLvl3Adj = int8_t(data[0U]) - 128;
    if (dmrSymLvl3Adj > 128)
        return RSN_INVALID_REQUEST;
    if (dmrSymLvl3Adj < -128)
        return RSN_INVALID_REQUEST;

    int8_t dmrSymLvl1Adj = int8_t(data[1U]) - 128;
    if (dmrSymLvl1Adj > 128)
        return RSN_INVALID_REQUEST;
    if (dmrSymLvl1Adj < -128)
        return RSN_INVALID_REQUEST;

    int8_t p25SymLvl3Adj = int8_t(data[2U]) - 128;
    if (p25SymLvl3Adj > 128)
        return RSN_INVALID_REQUEST;
    if (p25SymLvl3Adj < -128)
        return RSN_INVALID_REQUEST;

    int8_t p25SymLvl1Adj = int8_t(data[3U]) - 128;
    if (p25SymLvl1Adj > 128)
        return RSN_INVALID_REQUEST;
    if (p25SymLvl1Adj < -128)
        return RSN_INVALID_REQUEST;

    int8_t nxdnSymLvl3Adj = int8_t(data[4U]) - 128;
    if (nxdnSymLvl3Adj > 128)
        return RSN_INVALID_REQUEST;
    if (nxdnSymLvl3Adj < -128)
        return RSN_INVALID_REQUEST;

    int8_t nxdnSymLvl1Adj = int8_t(data[5U]) - 128;
    if (nxdnSymLvl1Adj > 128)
        return RSN_INVALID_REQUEST;
    if (nxdnSymLvl1Adj < -128)
        return RSN_INVALID_REQUEST;

    p25TX.setSymbolLvlAdj(p25SymLvl3Adj, p25SymLvl1Adj);

    dmrDMOTX.setSymbolLvlAdj(dmrSymLvl3Adj, dmrSymLvl1Adj);
    dmrTX.setSymbolLvlAdj(dmrSymLvl3Adj, dmrSymLvl1Adj);

    nxdnTX.setSymbolLvlAdj(nxdnSymLvl3Adj, nxdnSymLvl1Adj);

    return RSN_OK;
}

/* Sets the software Rx sample level. */

uint8_t SerialPort::setRXLevel(const uint8_t* data, uint8_t length)
{
    if (length < 1U)
        return RSN_ILLEGAL_LENGTH;

    uint8_t rxLevel = data[0U];

    io.setRXLevel(rxLevel);

    return RSN_OK;
}

/* Sets the RF parameters. */

uint8_t SerialPort::setRFParams(const uint8_t* data, uint8_t length)
{
    // unused on dedicated modem -- see firmware_hs for implementation
    return RSN_OK;
}

/* Sets the protocol ring buffer sizes. */

uint8_t SerialPort::setBuffers(const uint8_t* data, uint8_t length)
{
    if (length < 1U)
        return RSN_ILLEGAL_LENGTH;
    if (m_modemState != STATE_IDLE)
        return RSN_INVALID_MODE;

    uint16_t dmrBufSize = dmr::DMR_TX_BUFFER_LEN;
    uint16_t p25BufSize = p25::P25_TX_BUFFER_LEN;
    uint16_t nxdnBufSize = nxdn::NXDN_TX_BUFFER_LEN;

    dmrBufSize = (data[0U] << 8) + (data[1U]);
    p25BufSize = (data[2U] << 8) + (data[3U]);
    nxdnBufSize = (data[4U] << 8) + (data[5U]);

    p25TX.resizeBuffer(p25BufSize);
    nxdnTX.resizeBuffer(nxdnBufSize);

    dmrTX.resizeBuffer(dmrBufSize);
    dmrDMOTX.resizeBuffer(dmrBufSize);

    return RSN_OK;
}

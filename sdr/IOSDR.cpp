// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2025 Bryan Biedenkapp N2PLL
 *
 */
#include "Globals.h"
#include "IO.h"
#include "sdr/Log.h"

#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <zmq.hpp>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

const uint16_t DC_OFFSET = 2048U;

// ---------------------------------------------------------------------------
//  Globals Variables
// ---------------------------------------------------------------------------

static pthread_t m_threadTx;
static pthread_mutex_t m_txLock;
static pthread_t m_threadRx;
static pthread_mutex_t m_rxLock;
static pthread_t m_threadStatus;

zmq::context_t m_zmqContextTx;
zmq::socket_t m_zmqSocketTx;
static std::vector<short> m_audioBufTx = std::vector<short>();

zmq::context_t m_zmqContextRx;
zmq::socket_t m_zmqSocketRx;
static std::vector<short> m_audioBufRx = std::vector<short>();

static bool m_abort = false;

static bool m_cosPrev = false;
static bool m_cosInt = false;

static bool m_pttPrev = false;
static bool m_ptt = false;

static bool m_dmrModeToggle = false;
static bool m_dmrMode = false;
static bool m_p25ModeToggle = false;
static bool m_p25Mode = false;
static bool m_nxdnModeToggle = false;
static bool m_nxdnMode = false;

/*  */

static void* modemStatusHelper(void* arg)
{
    IO* io = (IO*)arg;
    if (io != nullptr) {
        while (!m_abort) {
            // log flag statuses
            if (m_cosPrev != m_cosInt) {
                ::LogMessage("COS %s", m_cosInt ? "DETECT" : "NO CARRIER");
                m_cosPrev = m_cosInt;
            }

            if (m_pttPrev != m_ptt) {
                ::LogMessage("PTT %s", m_ptt ? "TRANSMIT" : "IDLE");
                m_pttPrev = m_ptt;
            }

            if (m_dmrModeToggle) {
                ::LogMessage("DMR Mode %s", m_dmrMode ? "ENABLED" : "DISABLED");
                m_dmrModeToggle = false;
            }

            if (m_p25ModeToggle) {
                ::LogMessage("P25 Mode %s", m_p25Mode ? "ENABLED" : "DISABLED");
                m_p25ModeToggle = false;
            }

            if (m_nxdnModeToggle) {
                ::LogMessage("NXDN Mode %s", m_nxdnMode ? "ENABLED" : "DISABLED");
                m_nxdnModeToggle = false;
            }

            ::usleep(1000U);
        }
    }

    return NULL;
}

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Finalizes a instance of the IO class. */

IO::~IO()
{
    m_abort = true;

    if (m_threadTx) {
        ::pthread_join(m_threadTx, NULL);
    }

    if (m_threadRx) {
        ::pthread_join(m_threadRx, NULL);
    }

    if (m_threadStatus) {
        ::pthread_join(m_threadStatus, NULL);
    }

    ::pthread_mutex_destroy(&m_txLock);
    ::pthread_mutex_destroy(&m_rxLock);

    m_zmqSocketTx.close();
    m_zmqSocketRx.close();
}

/* Hardware interrupt handler. */

void IO::interrupt()
{
    uint16_t sample = DC_OFFSET;
    uint8_t control = MARK_NONE;

    ::pthread_mutex_lock(&m_txLock);
    while (m_txBuffer.get(sample, control)) {
        sample *= 5; // amplify by 12dB

        if (m_audioBufTx.size() >= 720) {
            zmq::message_t reply = zmq::message_t(720 * sizeof(short));
            ::memcpy(reply.data(), (unsigned char*)m_audioBufTx.data(), 720 * sizeof(short));

            try
            {
                m_zmqSocketTx.send(reply, zmq::send_flags::dontwait);
            }
            catch(const zmq::error_t& zmqE) { /* stub */ }

            ::usleep(9600 * 3);
            
            m_audioBufTx.erase(m_audioBufTx.begin(), m_audioBufTx.begin() + 720);
            m_audioBufTx.push_back((short)sample);
        }
        else
            m_audioBufTx.push_back((short)sample);
    }
    ::pthread_mutex_unlock(&m_txLock);
   
    sample = 2048U;
    m_watchdog++;
}

/* Gets the CPU type the firmware is running on. */

uint8_t IO::getCPU() const
{
    return CPU_TYPE_NATIVE_SDR;
}

/// <summary>
/// Gets the unique identifier for the air interface.
/// </summary>
/// <returns></returns>
void IO::getUDID(uint8_t* buffer)
{
    /* stub */
}

/* */

void IO::resetMCU()
{
    /* not supported for SDR devices */
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Initializes hardware interrupts. */

void IO::initInt()
{
    /* stub */
}

/* Starts hardware interrupts. */

void IO::startInt()
{
    ::LogMessage("Host connected, starting IO operations...");

    m_zmqSocketTx.close();
    if (m_zmqSocketRx.connected()) {
        m_zmqSocketRx.close();
    }

    m_zmqContextTx = zmq::context_t(1);
    m_zmqSocketTx = zmq::socket_t(m_zmqContextTx, ZMQ_PUSH);

    m_zmqContextRx = zmq::context_t(1);
    m_zmqSocketRx = zmq::socket_t(m_zmqContextRx, ZMQ_PULL);

    try
    {
        ::LogMessage("Binding Tx socket to %s", m_zmqTx.c_str());
        m_zmqSocketTx.bind(m_zmqTx);
    }
    catch(const zmq::error_t& zmqE) { ::LogError("ZMQ Tx Socket: %s", zmqE.what()); }
    catch(const std::exception& e) { ::LogError("ZMQ Tx Socket: %s", e.what()); }

    try
    {
        ::LogMessage("Connecting Rx socket to %s", m_zmqRx.c_str());
        m_zmqSocketRx.connect(m_zmqRx);
        if (m_zmqSocketRx.connected()) {
            ::LogMessage("ZMQ connected to remote ZMQ listener", m_zmqRx.c_str());
        } else {
            ::LogWarning("ZMQ failed to remote ZMQ listener, will continue to retry to connect", m_zmqRx.c_str());
        }
    }
    catch(const zmq::error_t& zmqE) { ::LogError("ZMQ Rx Socket: %s", zmqE.what()); }
    catch(const std::exception& e) { ::LogError("ZMQ Rx Socket: %s", e.what()); }

    m_audioBufTx = std::vector<short>();
    m_audioBufRx = std::vector<short>();

    if (::pthread_mutex_init(&m_txLock, NULL) != 0) {
        ::LogError("Tx thread lock failed?");
        ::LogFinalise();
        exit(-1);
    }

    if (::pthread_mutex_init(&m_rxLock, NULL) != 0) {
        ::LogError("Rx thread lock failed?");
        ::LogFinalise();
        exit(-2);
    }

    ::pthread_create(&m_threadTx, NULL, txThreadHelper, this);
    ::pthread_create(&m_threadRx, NULL, rxThreadHelper, this);
    ::pthread_create(&m_threadStatus, NULL, modemStatusHelper, this);
}

/*  */

bool IO::getCOSInt()
{
    return m_cosInt;
}

/*  */

void IO::setLEDInt(bool on)
{
    /* stub */
}

/*  */

void IO::setPTTInt(bool on)
{
    m_ptt = on;
}

/*  */

void IO::setCOSInt(bool on)
{
    m_cosInt = on;
}

/*  */

void IO::setDMRInt(bool on)
{
    if (on != m_dmrMode)
        m_dmrModeToggle = true;

    m_dmrMode = on;
}

/*  */

void IO::setP25Int(bool on)
{
    if (on != m_p25Mode)
        m_p25ModeToggle = true;

    m_p25Mode = on;
}

/*  */

void IO::setNXDNInt(bool on)
{
    if (on != m_nxdnMode)
        m_nxdnModeToggle = true;

    m_nxdnMode = on;
}

/*  */

void IO::delayInt(unsigned int dly)
{
    ::usleep(dly * 1000U);
}

/*  */

void* IO::txThreadHelper(void* arg)
{
    IO* p = (IO*)arg;

    while (!m_abort)
    {
        if (p->m_txBuffer.getData() < 1)
            usleep(20);
        p->interrupt();
    }

    return NULL;
}

/*  */

void IO::interruptRx()
{
    uint16_t sample = DC_OFFSET;
    uint8_t control = MARK_NONE;
    
    zmq::message_t msg;
    zmq::recv_result_t recv;
    try
    {
        recv = m_zmqSocketRx.recv(msg, zmq::recv_flags::none);
    }
    catch(const zmq::error_t& zmqE) 
    {
        if (zmqE.num() == ENOTSOCK || zmqE.num() == ENOTCONN ||
            zmqE.num() == ECONNABORTED || zmqE.num() == ECONNRESET ||
            zmqE.num() == ENETDOWN || zmqE.num() == ENETUNREACH || zmqE.num() == ENETRESET) {
            try
            {
                m_zmqSocketRx.connect(m_zmqRx);
            }
            catch(const zmq::error_t& zmqE) { /* stub */}
            return;
        } else {
            ::LogError("IO::interruptRx(): %s (%u)", zmqE.what(), zmqE.num()); 
        }
    }

    int size = msg.size();
    if (size < 1)
        return;

    ::pthread_mutex_lock(&m_rxLock);
    uint16_t space = m_rxBuffer.getSpace();

    for (int i = 0; i < size; i += 2) {
        short sample = 0;
        ::memcpy(&sample, (unsigned char*)msg.data() + i, sizeof(short));

        m_rxBuffer.put((uint16_t)sample, control);
        m_rssiBuffer.put(3U);
    }

    ::pthread_mutex_unlock(&m_rxLock);
}

/*  */

void* IO::rxThreadHelper(void* arg)
{
    IO* p = (IO*)arg;

    while (!m_abort)
        p->interruptRx();

    return NULL;
}

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (c) 2020 Jonathan Naylor, G4KLX
 *  Copyright (c) 2020 Geoffrey Merck, F4FXL - KC3FRA
 *
 */
/**
 * @file STM_UART.h
 * @ingroup modem_fw
 * @file STM_UART.cpp
 * @ingroup modem_fw
 */
#if defined(STM32F4XX)
#if !defined(__STM_UART_H__)
#define __STM_UART_H__

#include "Defines.h"

#if defined(STM32F4XX)
#include "stm32f4xx.h"
#endif

const uint16_t BUFFER_SIZE = 2048U; //needs to be a power of 2 !
const uint16_t BUFFER_MASK = BUFFER_SIZE - 1;

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief This class represents a FIFO buffer on a STM32 UART.
 * @ingroup modem_fw
 */
class DSP_FW_API STM_UARTFIFO {
public:
    /**
     * @brief Initializes a new instance of the STM_UARTFIFO class.
     */
    STM_UARTFIFO() :
        m_head(0U),
        m_tail(0U)
    {
        /* stub */
    }

    /**
     * @brief 
     * @returns uint8_t 
     */
    uint8_t get()
    {
        return m_buffer[BUFFER_MASK & (m_tail++)];
    }

    /**
     * @brief 
     * @param data 
     */
    void put(uint8_t data)
    {
        m_buffer[BUFFER_MASK & (m_head++)] = data;
    }

    /**
     * @brief Helper to reset data values to defaults.
     */
    void reset()
    {
        m_tail = 0U;
        m_head = 0U;
    }

    /**
     * @brief 
     * @returns bool 
     */
    bool isEmpty()
    {
        return m_tail == m_head;
    }

    /**
     * @brief 
     * @returns bool 
     */
    bool isFull()
    {
        return ((m_head + 1U) & BUFFER_MASK) == (m_tail & BUFFER_MASK);
    }

private:
    volatile uint8_t  m_buffer[BUFFER_SIZE];
    volatile uint16_t m_head;
    volatile uint16_t m_tail;
};

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief This class represents an STM32 UART.
 * @ingroup modem_fw
 */
class STM_UART {
public:
    /**
     * @brief Initializes a new instance of the STM_UART class.
     */
    STM_UART();

    /**
     * @brief Initializes the UART.
     * @param usart 
     */
    void init(USART_TypeDef* usart);
    
    /**
     * @brief 
     * @returns uint8_t 
     */
    uint8_t read();
    /**
     * @brief 
     * @param[in] data 
     * @param length
     */
    void write(const uint8_t* data, uint16_t length);

    /**
     * @brief 
     */
    void handleIRQ();

    /**
     * @brief Flushes the transmit shift register.
     * 
     * This call is blocking!
     */
    void flush();

    /**
     * @brief 
     * @returns uint16_t 
     */
    uint16_t available();

    /**
     * @brief 
     * @returns uint16_t 
     */
    uint16_t availableForWrite();

private:
    USART_TypeDef* m_usart;
    
    STM_UARTFIFO m_rxFifo;
    STM_UARTFIFO m_txFifo;
};

#endif // __SERIAL_PORT_H__
#endif 

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
*   Copyright (c) 2020 Jonathan Naylor, G4KLX
*   Copyright (c) 2020 Geoffrey Merck, F4FXL - KC3FRA
*
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
//      This class represents a FIFO buffer on a STM32 UART.
// ---------------------------------------------------------------------------

class DSP_FW_API STM_UARTFIFO {
public:
    /// <summary>Initializes a new instance of the STM_UARTFIFO class.</summary>
    STM_UARTFIFO() :
        m_head(0U),
        m_tail(0U)
    {
        /* stub */
    }

    /// <summary></summary>
    uint8_t get()
    {
        return m_buffer[BUFFER_MASK & (m_tail++)];
    }

    /// <summary></summary>
    void put(uint8_t data)
    {
        m_buffer[BUFFER_MASK & (m_head++)] = data;
    }

    /// <summary>Helper to reset data values to defaults.</summary>
    void reset()
    {
        m_tail = 0U;
        m_head = 0U;
    }

    /// <summary></summary>
    bool isEmpty()
    {
        return m_tail == m_head;
    }

    /// <summary></summary>
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
//      This class represents an STM32 UART.
// ---------------------------------------------------------------------------

class STM_UART {
public:
    /// <summary>Initializes a new instance of the STM_UART class.</summary>
    STM_UART();

    /// <summary></summary>
    void init(USART_TypeDef* usart);
    
    /// <summary></summary>
    uint8_t read();
    /// <summary></summary>
    void write(const uint8_t* data, uint16_t length);

    /// <summary></summary>
    void handleIRQ();

    /// <summary>Flushes the transmit shift register.</summary>
    void flush();

    /// <summary></summary>
    uint16_t available();

    /// <summary></summary>
    uint16_t availableForWrite();

private:
    USART_TypeDef* m_usart;
    
    STM_UARTFIFO m_rxFifo;
    STM_UARTFIFO m_txFifo;
};

#endif // __SERIAL_PORT_H__
#endif 

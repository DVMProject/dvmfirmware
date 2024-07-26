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
#if defined(STM32F4XX) || defined(STM32F7XX)

#include "STM_UART.h"

 // ---------------------------------------------------------------------------
 //  Public Class Members
 // ---------------------------------------------------------------------------
 
/* Initializes a new instance of the STM_UART class. */

STM_UART::STM_UART() :
    m_usart(NULL)
{
    /* stub */
}

/* Initializes the UART. */

void STM_UART::init(USART_TypeDef* usart)
{
    m_usart = usart;
}

/*  */

uint8_t STM_UART::read()
{
    return m_rxFifo.get();
}

/*  */

void STM_UART::write(const uint8_t* data, uint16_t length)
{
    if (length == 0U || m_usart == NULL)
        return;


    m_txFifo.put(data[0]);
    USART_ITConfig(m_usart, USART_IT_TXE, ENABLE); // switch TX IRQ is on

    for (uint16_t i = 1U; i < length; i++) {
        m_txFifo.put(data[i]);
    }

    USART_ITConfig(m_usart, USART_IT_TXE, ENABLE); // make sure TX IRQ is on
}

/*  */

void STM_UART::handleIRQ()
{
    if (m_usart == NULL)
        return;

    if (USART_GetITStatus(m_usart, USART_IT_RXNE)) {
        if (!m_rxFifo.isFull())
            m_rxFifo.put((uint8_t)USART_ReceiveData(m_usart));
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    if (USART_GetITStatus(m_usart, USART_IT_TXE)) {
        if (!m_txFifo.isEmpty())
            USART_SendData(m_usart, m_txFifo.get());

        USART_ClearITPendingBit(m_usart, USART_IT_TXE);

        if (m_txFifo.isEmpty()) // if there's no more data to transmit then turn off TX interrupts
            USART_ITConfig(m_usart, USART_IT_TXE, DISABLE);
    }
}

/* Flushes the transmit shift register. */

void STM_UART::flush()
{
    if (m_usart == NULL)
        return;

    // wait until the TXE shows the shift register is empty
    while (USART_GetITStatus(m_usart, USART_FLAG_TXE))
        ;
}

/*  */

uint16_t STM_UART::available()
{
    return m_rxFifo.isEmpty() ? 0U : 1U;
}

/*  */

uint16_t STM_UART::availableForWrite()
{
    return m_txFifo.isFull() ? 0U : 1U;
}

#endif

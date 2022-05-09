/**
* Digital Voice Modem - DSP Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / DSP Firmware
*
*/
//
// Based on code from the MMDVM project. (https://github.com/g4klx/MMDVM)
// Licensed under the GPLv2 License (https://opensource.org/licenses/GPL-2.0)
//
/*
*   Copyright (c) 2020 by Jonathan Naylor G4KLX
*   Copyright (c) 2020 by Geoffrey Merck F4FXL - KC3FRA
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#if defined(STM32F4XX)

#include "STM_UART.h"

 // ---------------------------------------------------------------------------
 //  Public Class Members
 // ---------------------------------------------------------------------------
 
 /// <summary>
 /// Initializes a new instance of the STM_UART class.
 /// </summary>
STM_UART::STM_UART() :
    m_usart(NULL)
{
    /* stub */
}

/// <summary></summary>
/// <param name="usart"></param>
void STM_UART::init(USART_TypeDef* usart)
{
    m_usart = usart;
}

/// <summary></summary>
/// <param name="data"></param>
/// <param name="length"></param>
void STM_UART::write(const uint8_t* data, uint16_t length)
{
    if (length == 0U || m_usart == NULL)
        return;


    m_txFifo.put(data[0]);
    USART_ITConfig(m_usart, USART_IT_TXE, ENABLE);//switch TX IRQ is on

    for (uint16_t i = 1U; i < length; i++) {
        m_txFifo.put(data[i]);
    }

    USART_ITConfig(m_usart, USART_IT_TXE, ENABLE);//make sure TX IRQ is on
}

/// <summary></summary>
/// <returns></returns>
uint8_t STM_UART::read()
{
    return m_rxFifo.get();
}

/// <summary></summary>
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

/// <summary>
/// Flushes the transmit shift register.
/// </summary>
/// <remarks>
/// This call is blocking!
/// </remarks>
void STM_UART::flush()
{
    if (m_usart == NULL)
        return;

    // wait until the TXE shows the shift register is empty
    while (USART_GetITStatus(m_usart, USART_FLAG_TXE))
        ;
}

/// <summary></summary>
/// <returns></returns>
uint16_t STM_UART::available()
{
    return m_rxFifo.isEmpty() ? 0U : 1U;
}

/// <summary></summary>
/// <returns></returns>
uint16_t STM_UART::availableForWrite()
{
    return m_txFifo.isFull() ? 0U : 1U;
}

#endif

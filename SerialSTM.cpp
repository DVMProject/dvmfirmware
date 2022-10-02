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
*   Copyright (C) 2016 by Jim McLaughlin KI6ZUM
*   Copyright (C) 2016,2017,2018 by Andy Uribe CA6JAU
*   Copyright (c) 2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2018,2022 Bryan Biedenkapp N2PLL
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
#include "Globals.h"
#include "SerialPort.h"
#include "STM_UART.h"

#if defined(STM32F4XX)
#include <stm32f4xx_flash.h>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#define STM32_CNF_PAGE_ADDR (uint32_t)0x08010000
#define STM32_CNF_PAGE      ((uint32_t *)0x08010000)
#define STM32_CNF_SECTOR    FLASH_Sector_4

/*
    Pin definitions:

    - Host communication:
    USART1 - TXD PA9  - RXD PA10 (STM32F4 Pi Board (MMDVM-Pi board), STM32F4 POG Board)

    - Serial repeater:
    UART5  - TXD PC12 - RXD PD2 (STM32F4 Pi Board (MMDVM-Pi board), STM32F4 POG Board)
*/

// ---------------------------------------------------------------------------
//  Global Functions and Variables
// ---------------------------------------------------------------------------

extern "C" {
    void USART1_IRQHandler();
    void UART5_IRQHandler();
}

#if defined(STM32F4_PI) || defined(STM32F4_POG) || defined(STM32F4_EDA_405) || defined(STM32F4_EDA_446)
// ---------------------------------------------------------------------------
//  UART1
// ---------------------------------------------------------------------------

static STM_UART m_USART1;

/// <summary>
///
/// </summary>
void USART1_IRQHandler()
{
    m_USART1.handleIRQ();
}

/// <summary>
///
/// </summary>
/// <param name="speed"></param>
void InitUSART1(int speed)
{
    // USART1 - TXD PA9  - RXD PA10 - pins on mmdvm pi board
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    // USART IRQ init
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    // Configure USART as alternate function
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //  Tx | Rx
    GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Configure USART baud rate
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = speed;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    m_USART1.init(USART1);
}
#endif

// ---------------------------------------------------------------------------
//  UART5
// ---------------------------------------------------------------------------

static STM_UART m_USART5;

/// <summary>
///
/// </summary>
void UART5_IRQHandler()
{
    m_USART5.handleIRQ();
}

/// <summary>
///
/// </summary>
/// <param name="speed"></param>
void InitUART5(int speed)
{
    // UART5 - TXD PC12 - RXD PD2
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

    // USART IRQ init
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);

    // Configure USART as alternate function
    GPIO_StructInit(&GPIO_InitStructure);
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12; //  Tx
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //  Rx
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Configure USART baud rate
    USART_StructInit(&USART_InitStructure);
    USART_InitStructure.USART_BaudRate = speed;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART5, &USART_InitStructure);

    USART_Cmd(UART5, ENABLE);

    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);

    m_USART5.init(UART5);
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/// <summary>
///
/// </summary>
void SerialPort::flashRead()
{
    uint8_t reply[249U];

    reply[0U] = DVM_FRAME_START;
    reply[1U] = 249U;
    reply[2U] = CMD_FLSH_READ;

    ::memcpy(reply + 3U, (void*)STM32_CNF_PAGE, 246U);

    writeInt(1U, reply, 249U);
}

/// <summary>
///
/// </summary>
/// <param name="data"></param>
/// <param name="length"></param>
uint8_t SerialPort::flashWrite(const uint8_t* data, uint8_t length)
{
    if (length > 249U) {
        return RSN_FLASH_WRITE_TOO_BIG;
    }

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPERR);

#if defined(STM32F4XX)
    if (FLASH_EraseSector(STM32_CNF_SECTOR, VoltageRange_3) != FLASH_COMPLETE) {
        FLASH_Lock();
        return RSN_FAILED_ERASE_FLASH;
    }
#elif defined(STM32F10X_MD)
    if (FLASH_ErasePage(STM32_CNF_PAGE_ADDR) != FLASH_COMPLETE) {
        FLASH_Lock();
        return RSN_FAILED_ERASE_FLASH;
    }
#endif

    // write data to the user flash area
    uint32_t address = STM32_CNF_PAGE_ADDR;
    uint8_t i = 0U;
    while (i < length) {
        uint32_t word =
            (data[i + 3] << 24) +
            (data[i + 2] << 16) +
            (data[i + 1] << 8) +
            (data[i + 0] << 0);

        if (FLASH_ProgramWord(address, word) == FLASH_COMPLETE) {
            address += 4;
            i += 4;
        }
        else {
            FLASH_Lock();
            return RSN_FAILED_WRITE_FLASH;
        }
    }

    FLASH_Lock();
    return RSN_OK;
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <param name="speed"></param>
void SerialPort::beginInt(uint8_t n, int speed)
{
    switch (n) {
    case 1U:
#if defined(STM32F4_PI) || defined(STM32F4_POG) || defined(STM32F4_EDA_405) || defined(STM32F4_EDA_446)
        InitUSART1(speed);
#endif
        break;
    case 3U:
        InitUART5(speed);
        break;
    default:
        break;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
int SerialPort::availableInt(uint8_t n)
{
    switch (n) {
    case 1U:
#if defined(STM32F4_PI) || defined(STM32F4_POG) || defined(STM32F4_EDA_405) || defined(STM32F4_EDA_446)
        return m_USART1.available();
#endif
    case 3U:
        return m_USART5.available();
    default:
        return 0;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
int SerialPort::availableForWriteInt(uint8_t n)
{
    switch (n) {
    case 1U:
#if defined(STM32F4_PI) || defined(STM32F4_POG) || defined(STM32F4_EDA_405) || defined(STM32F4_EDA_446)
        return m_USART1.availableForWrite();
#endif
    case 3U:
        return m_USART5.availableForWrite();
    default:
        return 0;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <returns></returns>
uint8_t SerialPort::readInt(uint8_t n)
{
    switch (n) {
    case 1U:
#if defined(STM32F4_PI) || defined(STM32F4_POG) || defined(STM32F4_EDA_405) || defined(STM32F4_EDA_446)
        return m_USART1.read();
#endif
    case 3U:
        return m_USART5.read();
    default:
        return 0U;
    }
}

/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <param name="data"></param>
/// <param name="length"></param>
/// <param name="flush"></param>
void SerialPort::writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush)
{
    switch (n) {
    case 1U:
#if defined(STM32F4_PI) || defined(STM32F4_POG) || defined(STM32F4_EDA_405) || defined(STM32F4_EDA_446)
        m_USART1.write(data, length);
        if (flush)
            m_USART1.flush();
#endif
        break;
    case 3U:
        m_USART5.write(data, length);
        if (flush)
            m_USART5.flush();
        break;
    default:
        break;
    }
}
#endif

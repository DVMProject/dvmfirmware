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
*   Copyright (C) 2018 Bryan Biedenkapp N2PLL
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

/*
    Pin definitions:

    - Host communication:
    USART1 - TXD PA9  - RXD PA10 (STM32F4 Pi Board (MMDVM-Pi board), STM32F4 POG Board)

    - Serial repeater:
    UART5  - TXD PC12 - RXD PD2 (STM32F4 Pi Board (MMDVM-Pi board), STM32F4 POG Board)
*/

#if defined(STM32F4XX)
// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#define TX_SERIAL_FIFO_SIZE 512U
#define RX_SERIAL_FIFO_SIZE 512U

// ---------------------------------------------------------------------------
//  Global Functions and Variables
// ---------------------------------------------------------------------------

extern "C" {
    void USART1_IRQHandler();
    void UART5_IRQHandler();
}

#if defined(STM32F4_PI) || defined(STM32F4_POG)
// ---------------------------------------------------------------------------
//  UART1
// ---------------------------------------------------------------------------

volatile uint8_t TXSerialfifo1[TX_SERIAL_FIFO_SIZE];
volatile uint8_t RXSerialfifo1[RX_SERIAL_FIFO_SIZE];
volatile uint16_t TXSerialfifohead1, TXSerialfifotail1;
volatile uint16_t RXSerialfifohead1, RXSerialfifotail1;

/// <summary>
///
/// </summary>
void TXSerialfifoinit1()
{
    TXSerialfifohead1 = 0U;
    TXSerialfifotail1 = 0U;
}

/// <summary>
///
/// </summary>
void RXSerialfifoinit1()
{
    RXSerialfifohead1 = 0U;
    RXSerialfifotail1 = 0U;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint16_t TXSerialfifolevel1()
{
    uint32_t tail = TXSerialfifotail1;
    uint32_t head = TXSerialfifohead1;

    if (tail > head)
        return TX_SERIAL_FIFO_SIZE + head - tail;
    else
        return head - tail;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint16_t RXSerialfifolevel1()
{
    uint32_t tail = RXSerialfifotail1;
    uint32_t head = RXSerialfifohead1;

    if (tail > head)
        return RX_SERIAL_FIFO_SIZE + head - tail;
    else
        return head - tail;
}

/// <summary>
///
/// </summary>
void TXSerialFlush1()
{
    // wait until the TXE shows the shift register is empty
    while (USART_GetITStatus(USART1, USART_FLAG_TXE))
        ;
}

/// <summary>
///
/// </summary>
/// <param name="next"></param>
/// <returns></returns>
uint8_t TXSerialfifoput1(uint8_t next)
{
    if (TXSerialfifolevel1() < TX_SERIAL_FIFO_SIZE) {
        TXSerialfifo1[TXSerialfifohead1] = next;

        TXSerialfifohead1++;
        if (TXSerialfifohead1 >= TX_SERIAL_FIFO_SIZE)
            TXSerialfifohead1 = 0U;

        // make sure transmit interrupts are enabled as long as there is data to send
        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
        return 1U;
    }
    else {
        return 0U; // signal an overflow occurred by returning a zero count
    }
}

/// <summary>
///
/// </summary>
void USART1_IRQHandler()
{
    uint8_t c;

    if (USART_GetITStatus(USART1, USART_IT_RXNE)) {
        c = (uint8_t)USART_ReceiveData(USART1);

        if (RXSerialfifolevel1() < RX_SERIAL_FIFO_SIZE) {
            RXSerialfifo1[RXSerialfifohead1] = c;

            RXSerialfifohead1++;
            if (RXSerialfifohead1 >= RX_SERIAL_FIFO_SIZE)
                RXSerialfifohead1 = 0U;
        }
        else {
            // TODO - do something if rx fifo is full?
        }

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

    if (USART_GetITStatus(USART1, USART_IT_TXE)) {
        c = 0U;

        if (TXSerialfifohead1 != TXSerialfifotail1) { // if the fifo is not empty
            c = TXSerialfifo1[TXSerialfifotail1];

            TXSerialfifotail1++;
            if (TXSerialfifotail1 >= TX_SERIAL_FIFO_SIZE)
                TXSerialfifotail1 = 0U;

            USART_SendData(USART1, c);
        }
        else { // if there's no more data to transmit then turn off TX interrupts
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }

        USART_ClearITPendingBit(USART1, USART_IT_TXE);
    }
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
    GPIO_InitStructure.GPIO_Speed = GPIO_Fast_Speed;
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

    // initialize the fifos
    TXSerialfifoinit1();
    RXSerialfifoinit1();
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint8_t AvailUSART1()
{
    if (RXSerialfifolevel1() > 0U)
        return 1U;
    else
        return 0U;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
int AvailForWriteUSART1()
{
    return TX_SERIAL_FIFO_SIZE - TXSerialfifolevel1();
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint8_t ReadUSART1()
{
    uint8_t data_c = RXSerialfifo1[RXSerialfifotail1];

    RXSerialfifotail1++;
    if (RXSerialfifotail1 >= RX_SERIAL_FIFO_SIZE)
        RXSerialfifotail1 = 0U;

    return data_c;
}

/// <summary>
///
/// </summary>
/// <param name="data"></param>
/// <param name="length"></param>
void WriteUSART1(const uint8_t* data, uint16_t length)
{
    for (uint16_t i = 0U; i < length; i++)
        TXSerialfifoput1(data[i]);

    USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}
#endif

// ---------------------------------------------------------------------------
//  UART5
// ---------------------------------------------------------------------------

volatile uint8_t TXSerialfifo5[TX_SERIAL_FIFO_SIZE];
volatile uint8_t RXSerialfifo5[RX_SERIAL_FIFO_SIZE];
volatile uint16_t TXSerialfifohead5, TXSerialfifotail5;
volatile uint16_t RXSerialfifohead5, RXSerialfifotail5;

/// <summary>
///
/// </summary>
void TXSerialfifoinit5()
{
    TXSerialfifohead5 = 0U;
    TXSerialfifotail5 = 0U;
}

/// <summary>
///
/// </summary>
void RXSerialfifoinit5()
{
    RXSerialfifohead5 = 0U;
    RXSerialfifotail5 = 0U;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint16_t TXSerialfifolevel5()
{
    uint32_t tail = TXSerialfifotail5;
    uint32_t head = TXSerialfifohead5;

    if (tail > head)
        return TX_SERIAL_FIFO_SIZE + head - tail;
    else
        return head - tail;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint16_t RXSerialfifolevel5()
{
    uint32_t tail = RXSerialfifotail5;
    uint32_t head = RXSerialfifohead5;

    if (tail > head)
        return RX_SERIAL_FIFO_SIZE + head - tail;
    else
        return head - tail;
}

/// <summary>
///
/// </summary>
void TXSerialFlush5()
{
    // wait until the TXE shows the shift register is empty
    while (USART_GetITStatus(UART5, USART_FLAG_TXE))
        ;
}

/// <summary>
///
/// </summary>
/// <param name="next"></param>
/// <returns></returns>
uint8_t TXSerialfifoput5(uint8_t next)
{
    if (TXSerialfifolevel5() < TX_SERIAL_FIFO_SIZE) {
        TXSerialfifo5[TXSerialfifohead5] = next;

        TXSerialfifohead5++;
        if (TXSerialfifohead5 >= TX_SERIAL_FIFO_SIZE)
            TXSerialfifohead5 = 0U;

        // make sure transmit interrupts are enabled as long as there is data to send
        USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
        return 1U;
    }
    else {
        return 0U; // signal an overflow occurred by returning a zero count
    }
}

/// <summary>
///
/// </summary>
void UART5_IRQHandler()
{
    uint8_t c;

    if (USART_GetITStatus(UART5, USART_IT_RXNE)) {
        c = (uint8_t)USART_ReceiveData(UART5);

        if (RXSerialfifolevel5() < RX_SERIAL_FIFO_SIZE) {
            RXSerialfifo5[RXSerialfifohead5] = c;

            RXSerialfifohead5++;
            if (RXSerialfifohead5 >= RX_SERIAL_FIFO_SIZE)
                RXSerialfifohead5 = 0U;
        }
        else {
            // TODO - do something if rx fifo is full?
        }

        USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    }

    if (USART_GetITStatus(UART5, USART_IT_TXE)) {
        c = 0U;

        if (TXSerialfifohead5 != TXSerialfifotail5) { // if the fifo is not empty
            c = TXSerialfifo5[TXSerialfifotail5];

            TXSerialfifotail5++;
            if (TXSerialfifotail5 >= TX_SERIAL_FIFO_SIZE)
                TXSerialfifotail5 = 0U;

            USART_SendData(UART5, c);
        }
        else { // if there's no more data to transmit then turn off TX interrupts
            USART_ITConfig(UART5, USART_IT_TXE, DISABLE);
        }

        USART_ClearITPendingBit(UART5, USART_IT_TXE);
    }
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

    // initialize the fifos
    TXSerialfifoinit5();
    RXSerialfifoinit5();
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint8_t AvailUART5()
{
    if (RXSerialfifolevel5() > 0U)
        return 1U;
    else
        return 0U;
}

/// <summary>
///
/// </summary>
/// <returns></returns>
int AvailForWriteUART5()
{
    return TX_SERIAL_FIFO_SIZE - TXSerialfifolevel5();
}

/// <summary>
///
/// </summary>
/// <returns></returns>
uint8_t ReadUART5()
{
    uint8_t data_c = RXSerialfifo5[RXSerialfifotail5];

    RXSerialfifotail5++;
    if (RXSerialfifotail5 >= RX_SERIAL_FIFO_SIZE)
        RXSerialfifotail5 = 0U;

    return data_c;
}

/// <summary>
///
/// </summary>
/// <param name="data"></param>
/// <param name="length"></param>
void WriteUART5(const uint8_t* data, uint16_t length)
{
    for (uint16_t i = 0U; i < length; i++)
        TXSerialfifoput5(data[i]);

    USART_ITConfig(UART5, USART_IT_TXE, ENABLE);
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------
/// <summary>
///
/// </summary>
/// <param name="n"></param>
/// <param name="speed"></param>
void SerialPort::beginInt(uint8_t n, int speed)
{
    switch (n) {
    case 1U:
#if defined(STM32F4_PI) || defined(STM32F4_POG)
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
#if defined(STM32F4_PI) || defined(STM32F4_POG)
        return AvailUSART1();
#endif
    case 3U:
        return AvailUART5();
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
#if defined(STM32F4_PI) || defined(STM32F4_POG)
        return AvailForWriteUSART1();
#endif
    case 3U:
        return AvailForWriteUART5();
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
#if defined(STM32F4_PI) || defined(STM32F4_POG)
        return ReadUSART1();
#endif
    case 3U:
        return ReadUART5();
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
#if defined(STM32F4_PI) || defined(STM32F4_POG)
        WriteUSART1(data, length);
        if (flush)
            TXSerialFlush1();
#endif
        break;
    case 3U:
        WriteUART5(data, length);
        if (flush)
            TXSerialFlush5();
        break;
    default:
        break;
    }
}
#endif

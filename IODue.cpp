// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2015 Jim Mclaughlin, KI6ZUM
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2017-2018 Bryan Biedenkapp, N2PLL
 *
 */
#include "Globals.h"
#include "IO.h"

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#if defined(__SAM3X8E__) && defined(ARDUINO_SAM_DUE)
/*
    Pin definitions for Arduino Due (SAM3X8E) Board:

    PTT      PA14                   [P23]    output
    COSLED   PB25                   [P22]    output
    COS      PB21    (AD14/RXD3)    [P52]    input

    DMR      PC22    (PWM8)         [P8]     output
    P25      PC21    (PWM9)         [P9]     output

    RX       PA11    (AD11/TXD3)    [P11]    analog input
    RSSI     PA3     (AD6)          [P6]     analog input
    TX       PB16    (DAC1)         [DAC1]   analog output

    EXT_CLK  PA4     (AD5)          [P5]     input
*/
#define PIN_COS                52
#define PIN_PTT                23
#define PIN_COSLED             22

#define PIN_DMR                8
#define PIN_P25                6

#define ADC_CHER_Chan          (1 << 13)                 // ADC on Due pin A11 - Due AD13 - (1 << 13)
#define ADC_ISR_EOC_Chan       ADC_ISR_EOC13
#define ADC_CDR_Chan           13

#define DACC_MR_USER_SEL_Chan  DACC_MR_USER_SEL_CHANNEL1 // DAC on Due DAC1
#define DACC_CHER_Chan         DACC_CHER_CH1

#define RSSI_CHER_Chan         (1 << 1)                  // ADC on Due pin A6  - Due AD1 - (1 << 1)
#define RSSI_CDR_Chan          1

const uint16_t DC_OFFSET = 2048U;

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

extern "C" {
    void ADC_Handler() { io.interrupt(); }
}

// ---------------------------------------------------------------------------
//  Public Class Members
// ---------------------------------------------------------------------------

/* Hardware interrupt handler. */

void IO::interrupt()
{
    if ((ADC->ADC_ISR & ADC_ISR_EOC_Chan) == ADC_ISR_EOC_Chan) {    // Ensure there was an End-of-Conversion and we read the ISR reg
        uint8_t control = MARK_NONE;
        uint16_t sample = DC_OFFSET;

        m_txBuffer.get(sample, control);
        DACC->DACC_CDR = sample;

        sample = ADC->ADC_CDR[ADC_CDR_Chan];
        m_rxBuffer.put(sample, control);
#if defined(SEND_RSSI_DATA)
        m_rssiBuffer.put(ADC->ADC_CDR[RSSI_CDR_Chan]);
#else
        m_rssiBuffer.put(0U);
#endif
        m_watchdog++;
    }
}

/* Gets the CPU type the firmware is running on. */

uint8_t IO::getCPU() const
{
    return CPU_TYPE_ARDUINO_DUE;
}

/* Gets the unique identifier for the air interface. 
 * Code taken from https://github.com/emagii/at91sam3s/blob/master/examples/eefc_uniqueid/main.c
 */

void IO::getUDID(uint8_t* buffer)
{
    uint32_t status;

    EFC1->EEFC_FCR = (0x5A << 24) | EFC_FCMD_STUI;
    do {
        status = EFC1->EEFC_FSR;
    } while ((status & EEFC_FSR_FRDY) == EEFC_FSR_FRDY);

    for (uint8_t i = 0; i < 16; i += 4) {
        buffer[i + 0] = *(uint32_t*)(IFLASH1_ADDR + i) >> 24;
        buffer[i + 1] = *(uint32_t*)(IFLASH1_ADDR + i) >> 16;
        buffer[i + 2] = *(uint32_t*)(IFLASH1_ADDR + i) >> 8;
        buffer[i + 3] = *(uint32_t*)(IFLASH1_ADDR + i) >> 0;
    }

    EFC1->EEFC_FCR = (0x5A << 24) | EFC_FCMD_SPUI;
    do {
        status = EFC1->EEFC_FSR;
    } while ((status & EEFC_FSR_FRDY) != EEFC_FSR_FRDY);
}

/* */

void IO::resetMCU()
{
    /* not supported for Due devices */
}

// ---------------------------------------------------------------------------
//  Private Class Members
// ---------------------------------------------------------------------------

/* Initializes hardware interrupts. */

void IO::initInt()
{
    // Set up the TX, COS and LED pins
    pinMode(PIN_PTT, OUTPUT);
    pinMode(PIN_COSLED, OUTPUT);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_COS, INPUT);

    // Set up the mode output pins
    pinMode(PIN_DMR, OUTPUT);
    pinMode(PIN_P25, OUTPUT);
}

/* Starts hardware interrupts. */

void IO::startInt()
{
    if (ADC->ADC_ISR & ADC_ISR_EOC_Chan)         // Ensure there was an End-of-Conversion and we read the ISR reg
        io.interrupt();

    // Set up the ADC
    NVIC_EnableIRQ(ADC_IRQn);                    // Enable ADC interrupt vector
    ADC->ADC_IDR = 0xFFFFFFFF;                   // Disable interrupts
    ADC->ADC_IER = ADC_CHER_Chan;                // Enable End-Of-Conv interrupt
    ADC->ADC_CHDR = 0xFFFF;                      // Disable all channels
    ADC->ADC_CHER = ADC_CHER_Chan;               // Enable rx input channel
#if defined(SEND_RSSI_DATA)
    ADC->ADC_CHER |= RSSI_CHER_Chan;             // and RSSI input
#endif
    ADC->ADC_CGR = 0x15555555;                    // All gains set to x1
    ADC->ADC_COR = 0x00000000;                    // All offsets off
    ADC->ADC_MR = (ADC->ADC_MR & 0xFFFFFFF0) | (1 << 1) | ADC_MR_TRGEN; // 1 = trig source TIO from TC0

#if defined(EXTERNAL_OSC)
    // Set up the external clock input on PA4 = AD5
    REG_PIOA_ODR = 0x10;                         // Set pin as input
    REG_PIOA_PDR = 0x10;                         // Disable PIO A bit 4
    REG_PIOA_ABSR &= ~0x10;                      // Select A peripheral = TCLK1 Input
#endif

    // Set up the timer
    pmc_enable_periph_clk(TC_INTERFACE_ID + 0 * 3 + 0); // Clock the TC0 channel 0
    TcChannel* t = &(TC0->TC_CHANNEL)[0];        // Pointer to TC0 registers for its channel 0
    t->TC_CCR = TC_CCR_CLKDIS;                   // Disable internal clocking while setup regs
    t->TC_IDR = 0xFFFFFFFF;                      // Disable interrupts
    t->TC_SR;                                    // Read int status reg to clear pending
#if defined(EXTERNAL_OSC)
    t->TC_CMR = TC_CMR_TCCLKS_XC1 |              // Use XC1 = TCLK1 external clock
#else
    t->TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK1 |     // Use TCLK1 (prescale by 2, = 42MHz)
#endif
        TC_CMR_WAVE |                            // Waveform mode
        TC_CMR_WAVSEL_UP_RC |                    // Count-up PWM using RC as threshold
        TC_CMR_EEVT_XC0 |                        // Set external events from XC0 (this setup TIOB as output)
        TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
        TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR;
#if defined(EXTERNAL_OSC)
    t->TC_RC = EXTERNAL_OSC / 24000;             // Counter resets on RC, so sets period in terms of the external clock
    t->TC_RA = EXTERNAL_OSC / 48000;             // Roughly square wave
#else
    t->TC_RC = 1750;                             // Counter resets on RC, so sets period in terms of 42MHz internal clock
    t->TC_RA = 880;                              // Roughly square wave
#endif
    t->TC_CMR = (t->TC_CMR & 0xFFF0FFFF) | TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET;  // Set clear and set from RA and RC compares
    t->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;     // re-enable local clocking and switch to hardware trigger source.

    // Set up the DAC
    pmc_enable_periph_clk(DACC_INTERFACE_ID);    // Start clocking DAC
    DACC->DACC_CR = DACC_CR_SWRST;               // Reset DAC
    DACC->DACC_MR =
        DACC_MR_TRGEN_EN | DACC_MR_TRGSEL(1) |   // Trigger 1 = TIO output of TC0
        DACC_MR_USER_SEL_Chan |                  // Select channel
        (24 << DACC_MR_STARTUP_Pos);             // 24 = 1536 cycles which I think is in range 23..45us since DAC clock = 42MHz
    DACC->DACC_IDR = 0xFFFFFFFF;                 // No interrupts
    DACC->DACC_CHER = DACC_CHER_Chan;            // Enable channel

    digitalWrite(PIN_PTT, m_pttInvert ? HIGH : LOW);
    digitalWrite(PIN_COSLED, LOW);
    digitalWrite(PIN_LED, HIGH);
}

/*  */

bool IO::getCOSInt()
{
    return digitalRead(PIN_COS) == HIGH;
}

/*  */

void IO::setLEDInt(bool on)
{
    digitalWrite(PIN_LED, on ? HIGH : LOW);
}

/*  */

void IO::setPTTInt(bool on)
{
    digitalWrite(PIN_PTT, on ? HIGH : LOW);
}

/*  */

void IO::setCOSInt(bool on)
{
    digitalWrite(PIN_COSLED, on ? HIGH : LOW);
}

/*  */

void IO::setDMRInt(bool on)
{
    digitalWrite(PIN_DMR, on ? HIGH : LOW);
}

/*  */

void IO::setP25Int(bool on)
{
    digitalWrite(PIN_P25, on ? HIGH : LOW);
}

/*  */

void IO::setNXDNInt(bool on)
{
    /* stub */
}

/*  */

void IO::delayInt(unsigned int dly)
{
    delay(dly);
}

/*  */

void* IO::txThreadHelper(void* arg)
{
    return NULL;
}

/*  */

void IO::interruptRx()
{
    /* stub */
}

/*  */

void* IO::rxThreadHelper(void* arg)
{
    return NULL;
}

#endif // defined(__SAM3X8E__) && defined(ARDUINO_SAM_DUE)

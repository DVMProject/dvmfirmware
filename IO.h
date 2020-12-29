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
*   Copyright (C) 2015,2016,2017 by Jonathan Naylor G4KLX
*   Copyright (C) 2017-2018 Bryan Biedenkapp N2PLL
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
#if !defined(__IO_H__)
#define __IO_H__

#include "Defines.h"
#include "Globals.h"
#include "SampleBuffer.h"
#include "RSSIBuffer.h"

// ---------------------------------------------------------------------------
//  Class Declaration
//      Implements the input/output data path with the radio air interface.
// ---------------------------------------------------------------------------

class DSP_FW_API IO {
public:
    /// <summary>Initializes a new instance of the IO class.</summary>
    IO();

    /// <summary>Starts air interface sampler.</summary>
    void start();

    /// <summary>Process samples from air interface.</summary>
    void process();

    /// <summary>Write samples to air interface.</summary>
    void write(DVM_STATE mode, q15_t* samples, uint16_t length, const uint8_t* control = NULL);

    /// <summary>Helper to get how much space the transmit ring buffer has for samples.</summary>
    uint16_t getSpace() const;

    /// <summary></summary>
    void setDecode(bool dcd);
    /// <summary></summary>
    void setADCDetection(bool detect);
    /// <summary>Helper to set the modem air interface state.</summary>
    void setMode();
    /// <summary>Helper to assert or deassert radio PTT.</summary>
    void setTransmit();

    /// <summary>Hardware interrupt handler.</summary>
    void interrupt();

    /// <summary>Sets various air interface parameters.</summary>
    void setParameters(bool rxInvert, bool txInvert, bool pttInvert, uint8_t rxLevel, uint8_t cwIdTXLevel, uint8_t dmrTXLevel,
                       uint8_t p25TXLevel, uint16_t txDCOffset, uint16_t rxDCOffset);
    /// <summary>Sets the software Rx sample level.</summary>
    void setRXLevel(uint8_t rxLevel);

    /// <summary>Helper to get the state of the ADC and DAC overflow flags.</summary>
    void getOverflow(bool& adcOverflow, bool& dacOverflow);

    /// <summary>Flag indicating the TX ring buffer has overflowed.</summary>
    bool hasTXOverflow();
    /// <summary>Flag indicating the RX ring buffer has overflowed.</summary>
    bool hasRXOverflow();

    /// <summary>Flag indicating the air interface is locked out from transmitting.</summary>
    bool hasLockout() const;

    /// <summary></summary>
    void resetWatchdog();
    /// <summary></summary>
    uint32_t getWatchdog();

    /// <summary></summary>
    void selfTest();

private:
    bool m_started;

    SampleBuffer m_rxBuffer;
    SampleBuffer m_txBuffer;
    RSSIBuffer m_rssiBuffer;

    arm_fir_instance_q15 m_rrcFilter;
    arm_fir_instance_q15 m_gaussianFilter;
    arm_fir_instance_q15 m_boxcarFilter;
    q15_t m_rrcState[70U];           // NoTaps + BlockSize - 1, 42 + 20 - 1 plus some spare
    q15_t m_gaussianState[40U];      // NoTaps + BlockSize - 1, 12 + 20 - 1 plus some spare
    q15_t m_boxcarState[30U];        // NoTaps + BlockSize - 1, 6 + 20 - 1 plus some spare

    bool m_pttInvert;
    q15_t m_rxLevel;
    bool m_rxInvert;
    q15_t m_cwIdTXLevel;
    q15_t m_dmrTXLevel;
    q15_t m_p25TXLevel;

    uint16_t m_rxDCOffset;
    uint16_t m_txDCOffset;

    uint32_t m_ledCount;
    bool m_ledValue;

    bool m_detect;

    uint16_t m_adcOverflow;
    uint16_t m_dacOverflow;

    volatile uint32_t m_watchdog;

    bool m_lockout;

    arm_biquad_casd_df1_inst_q31 m_dcFilter;
    q31_t m_dcState[4];

    // Hardware specific routines
    /// <summary>Initializes hardware interrupts.</summary>
    void initInt();
    /// <summary>Starts hardware interrupts.</summary>
    void startInt();

    /// <summary></summary>
    bool getCOSInt();

    /// <summary></summary>
    void setLEDInt(bool on);
    /// <summary></summary>
    void setPTTInt(bool on);
    /// <summary></summary>
    void setCOSInt(bool on);

    /// <summary></summary>
    void setDMRInt(bool on);
    /// <summary></summary>
    void setP25Int(bool on);

    /// <summary></summary>
    void delayInt(unsigned int dly);
};

#endif // __IO_H__

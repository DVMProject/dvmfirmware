// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017 Jonathan Naylor, G4KLX
 *  Copyright (C) 2017-2024 Bryan Biedenkapp, N2PLL
 *
 */
/**
 * @file IO.h
 * @ingroup modem_fw
 * @file IO.cpp
 * @ingroup modem_fw
 * @file IODue.cpp
 * @ingroup modem_fw
 * @file IOSTM.cpp
 * @ingroup modem_fw
 */
#if !defined(__IO_H__)
#define __IO_H__

#include "Defines.h"
#include "Globals.h"
#include "SampleBuffer.h"
#include "RSSIBuffer.h"

// ---------------------------------------------------------------------------
//  Class Declaration
// ---------------------------------------------------------------------------

/**
 * @brief Implements the input/output data path with the radio air interface.
 * @ingroup modem_fw
 */
class DSP_FW_API IO {
public:
    /**
     * @brief Initializes a new instance of the IO class.
     */
    IO();
#if defined(NATIVE_SDR)
    /**
     * @brief Finalizes a new instance of the IO class.
     */
    ~IO();
#endif

    /**
     * @brief Initializes the air interface sampler.
     */
    void init();
    /**
     * @brief Starts air interface sampler.
     */
    void start();

    /**
     * @brief Process samples from air interface.
     */
    void process();

    /**
     * @brief Write samples to air interface.
     * @param mode 
     * @param samples Samples to write.
     * @param length Length of samples buffer.
     * @param control 
     */
    void write(DVM_STATE mode, q15_t* samples, uint16_t length, const uint8_t* control = NULL);

    /**
     * @brief Helper to get how much space the transmit ring buffer has for samples.
     * @returns uint16_t Amount of space in the transmit ring buffer for samples.
     */
    uint16_t getSpace() const;

    /**
     * @brief 
     * @param dcd 
     */
    void setDecode(bool dcd);
    /**
     * @brief 
     * @param detect 
     */
    void setADCDetection(bool detect);
    /**
     * @brief Helper to set the modem air interface state.
     */
    void setMode();
    /**
     * @brief Helper to assert or deassert radio PTT.
     */
    void setTransmit();

    /**
     * @brief Hardware interrupt handler.
     */
    void interrupt();

    /**
     * @brief Sets various air interface parameters.
     * @param rxInvert Flag indicating the Rx polarity should be inverted.
     * @param txInvert Flag indicating the Tx polarity should be inverted.
     * @param pttInvert Flag indicating the PTT polarity should be inverted.
     * @param rxLevel Rx Level.
     * @param cwIdTXLevel CWID Transmit Level.
     * @param dmrTXLevel DMR Transmit Level.
     * @param p25TXLevel P25 Transmit Level.
     * @param nxdnTXLevel NXDN Transmit Level.
     * @param txDCOffset Tx DC offset parameter.
     * @param rxDCOffset Rx DC offset parameter.
     */
    void setParameters(bool rxInvert, bool txInvert, bool pttInvert, uint8_t rxLevel, uint8_t cwIdTXLevel, uint8_t dmrTXLevel,
                       uint8_t p25TXLevel, uint8_t nxdnTXLevel, uint16_t txDCOffset, uint16_t rxDCOffset);
    /**
     * @brief Sets the software Rx sample level.
     * @param rxLevel Rx Level.
     */
    void setRXLevel(uint8_t rxLevel);

    /**
     * @brief Helper to get the state of the ADC and DAC overflow flags.
     * @param[out] adcOverflow 
     * @param[out] dacOverflow 
     */
    void getOverflow(bool& adcOverflow, bool& dacOverflow);

    /**
     * @brief Flag indicating the TX ring buffer has overflowed.
     * @returns bool Flag indicating the TX ring buffer has overflowed.
     */
    bool hasTXOverflow();
    /**
     * @brief Flag indicating the RX ring buffer has overflowed.
     * @returns bool Flag indicating the RX ring buffer has overflowed.
     */
    bool hasRXOverflow();

    /**
     * @brief Flag indicating the air interface is locked out from transmitting.
     * @returns bool Flag indicating the air interface is locked out from transmitting.
     */
    bool hasLockout() const;

    /**
     * @brief 
     */
    void resetWatchdog();
    /**
     * @brief 
     * @returns uint32_t 
     */
    uint32_t getWatchdog();

    /**
     * @brief Gets the CPU type the firmware is running on.
     * @returns uint8_t 
     */
    uint8_t getCPU() const;

    /**
     * @brief Gets the unique identifier for the air interface.
     * @param buffer 
     */
    void getUDID(uint8_t* buffer);

    /**
     * @brief 
     */
    void selfTest();

    /**
     * @brief 
     */
    void resetMCU();

#if SPI_ENABLED
    /**
     * @brief 
     * @param bytes 
     * @param length 
     */
    void SPI_Write(uint8_t* bytes, uint8_t length);

    /**
     * @brief 
     * @returns uint16_t 
     */
    uint16_t SPI_Read();
#endif

#if DIGIPOT_ENABLED
    /**
     * @brief 
     * @param value 
     */
    void SetDigipot(uint8_t value);

    /**
     * @brief 
     * @param value 
     */
    void SetTxDigipot(uint8_t value);

    /**
     * @brief 
     * @param value 
     */
    void SetRxDigipot(uint8_t value);

    /**
     * @brief 
     * @param value 
     */
    void SetRsDigipot(uint8_t value);
#endif

private:
    bool m_started;

    SampleBuffer m_rxBuffer;
    SampleBuffer m_txBuffer;
    RSSIBuffer m_rssiBuffer;

    arm_fir_instance_q15 m_rrc_0_2_Filter;
    arm_fir_instance_q15 m_boxcar_5_Filter;

    arm_biquad_casd_df1_inst_q31 m_dcFilter;

    q15_t m_rrc_0_2_State[70U];      // NoTaps + BlockSize - 1, 42 + 20 - 1 plus some spare
    q15_t m_boxcar_5_State[30U];     // NoTaps + BlockSize - 1, 6 + 20 - 1 plus some spare

#if NXDN_BOXCAR_FILTER
    arm_fir_instance_q15 m_boxcar_10_Filter;

    q15_t m_boxcar_10_State[40U];   // NoTaps + BlockSize - 1, 10 + 20 - 1 plus some spare
#else
    arm_fir_instance_q15 m_nxdn_0_2_Filter;
    arm_fir_instance_q15 m_nxdn_ISinc_Filter;
    
    q15_t m_nxdn_0_2_State[110U];   // NoTaps + BlockSize - 1, 82 + 20 - 1 plus some spare
    q15_t m_nxdn_ISinc_State[60U];  // NoTaps + BlockSize - 1, 32 + 20 - 1 plus some spare
#endif

    q31_t m_dcState[4];

    bool m_pttInvert;
    q15_t m_rxLevel;
    bool m_rxInvert;
    q15_t m_cwIdTXLevel;
    q15_t m_dmrTXLevel;
    q15_t m_p25TXLevel;
    q15_t m_nxdnTXLevel;

    uint16_t m_rxDCOffset;
    uint16_t m_txDCOffset;

    uint32_t m_ledCount;
    bool m_ledValue;

    bool m_detect;

    uint16_t m_adcOverflow;
    uint16_t m_dacOverflow;

    volatile uint32_t m_watchdog;

    bool m_lockout;

    // Hardware specific routines
    /**
     * @brief Initializes hardware interrupts.
     */
    void initInt();
    /**
     * @brief Starts hardware interrupts.
     */
    void startInt();

    /**
     * @brief 
     */
    bool getCOSInt();

    /**
     * @brief 
     * @param on 
     */
    void setLEDInt(bool on);
    /**
     * @brief 
     * @param on 
     */
    void setPTTInt(bool on);
    /**
     * @brief 
     * @param on 
     */
    void setCOSInt(bool on);

    /**
     * @brief 
     * @param on 
     */
    void setDMRInt(bool on);
    /**
     * @brief 
     * @param on 
     */
    void setP25Int(bool on);
    /**
     * @brief 
     * @param on 
     */
    void setNXDNInt(bool on);

    /**
     * @brief 
     * @param dly 
     */
    void delayInt(unsigned int dly);

    /**
     * @brief 
     * @param arg 
     * @returns void* 
     */
    static void* txThreadHelper(void* arg);
    /**
     * @brief 
     * @param interruptRx 
     */
    void interruptRx();
    /**
     * @brief 
     * @param arg 
     * @returns void* 
     */
    static void* rxThreadHelper(void* arg);
};

#endif // __IO_H__

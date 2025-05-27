// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016,2017,2018 Jonathan Naylor, G4KLX
 *  Copyright (C) 2016 Mathis Schmieder, DB9MAT
 *  Copyright (C) 2016 Colin Durbridge, G4EML
 *  Copyright (C) 2018,2024,2025 Bryan Biedenkapp, N2PLL
 *
 */
#include "Globals.h"

#if defined(NATIVE_SDR)
#include "sdr/port/PseudoPTYPort.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <pwd.h>
#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>

#include <zmq.hpp>
#endif

// ---------------------------------------------------------------------------
//	Macros
// ---------------------------------------------------------------------------

#if defined(NATIVE_SDR)
#define IS(s) (::strcmp(argv[i], s) == 0)
#endif

// ---------------------------------------------------------------------------
//  Globals Variables
// ---------------------------------------------------------------------------

DVM_STATE m_modemState = STATE_IDLE;

bool m_dmrEnable = true;
bool m_p25Enable = true;
bool m_nxdnEnable = true;

bool m_dcBlockerEnable = true;
bool m_cosLockoutEnable = false;

bool m_duplex = true;

bool m_tx = false;
bool m_dcd = false;

/* DMR BS */
dmr::DMRIdleRX dmrIdleRX;
dmr::DMRRX dmrRX;
dmr::DMRTX dmrTX;

/* DMR MS-DMO */
dmr::DMRDMORX dmrDMORX;
dmr::DMRDMOTX dmrDMOTX;

/* P25 */
p25::P25RX p25RX;
p25::P25TX p25TX;

/* NXDN */
nxdn::NXDNRX nxdnRX;
nxdn::NXDNTX nxdnTX;

/* Calibration */
dmr::CalDMR calDMR;
p25::CalP25 calP25;
nxdn::CalNXDN calNXDN;
CalRSSI calRSSI;

/* CW */
CWIdTX cwIdTX;

/* RS232 and Air Interface I/O */
SerialPort serial;
IO io;

#if defined(NATIVE_SDR)
std::string g_progExe = std::string(__EXE_NAME__);

std::string m_zmqRx = std::string("ipc:///tmp/dvm-rx.ipc");
std::string m_zmqTx = std::string("ipc:///tmp/dvm-tx.ipc");

std::string m_ptyPort = std::string("/dev/ptmx");

std::string g_logFileName = std::string("dsp.log");

bool g_debug = false;

int g_signal = 0;
bool g_killed = false;

bool g_daemon = false;

extern sdr::port::PseudoPTYPort* m_serialPort;

extern zmq::socket_t m_zmqSocketTx;
extern zmq::socket_t m_zmqSocketRx;
#endif

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

void setup()
{
    io.init();

    serial.start();
}

void loop()
{
    serial.process();

    io.process();

    // The following is for transmitting
    if (m_dmrEnable && m_modemState == STATE_DMR) {
        if (m_duplex)
            dmrTX.process();
        else
            dmrDMOTX.process();
    }

    if (m_p25Enable && m_modemState == STATE_P25)
        p25TX.process();

    if (m_nxdnEnable && m_modemState == STATE_NXDN)
        nxdnTX.process();

    if (m_modemState == STATE_DMR_DMO_CAL_1K || m_modemState == STATE_DMR_CAL_1K ||
        m_modemState == STATE_DMR_LF_CAL || m_modemState == STATE_DMR_CAL)
        calDMR.process();

    if (m_modemState == STATE_P25_CAL_1K || m_modemState == STATE_P25_CAL)
        calP25.process();

    if (m_modemState == STATE_NXDN_CAL)
        calNXDN.process();

    if (m_modemState == STATE_CW || m_modemState == STATE_IDLE)
        cwIdTX.process();
}

#if defined(__SAM3X8E__) && defined(ARDUINO_SAM_DUE)
/*
    main.cpp - Main loop for Arduino sketches
    Copyright (c) 2005-2013 Arduino Team.  All right reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#define ARDUINO_MAIN
#include "Arduino.h"

/*
* Cortex-M3 Systick IT handler
*/
/*
extern void SysTick_Handler (void)
{
    // Increment tick count each ms
    TimeTick_Increment();
}
*/

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

// ---------------------------------------------------------------------------
//  Firmware Entry Point
// ---------------------------------------------------------------------------

int main(void)
{
    // Initialize watchdog
    watchdogSetup();

    init();

    initVariant();

    delay(1);

#if defined(SAM3XE_OVERCLOCK)
    #define SYS_BOARD_PLLAR (CKGR_PLLAR_ONE | CKGR_PLLAR_MULA(18UL) | CKGR_PLLAR_PLLACOUNT(0x3fUL) | CKGR_PLLAR_DIVA(1UL))
    #define SYS_BOARD_MCKR ( PMC_MCKR_PRES_CLK_2 | PMC_MCKR_CSS_PLLA_CLK)

    // Set FWS according to SYS_BOARD_MCKR configuration 
    EFC0->EEFC_FMR = EEFC_FMR_FWS(4); //4 waitstate flash access
    EFC1->EEFC_FMR = EEFC_FMR_FWS(4);

    // Initialize PLLA to 114MHz
    PMC->CKGR_PLLAR = SYS_BOARD_PLLAR;
    while (!(PMC->PMC_SR & PMC_SR_LOCKA)) {}
    PMC->PMC_MCKR = SYS_BOARD_MCKR;
    while (!(PMC->PMC_SR & PMC_SR_MCKRDY)) {}

    SystemCoreClockUpdate();
#endif

#if defined(USBCON)
    USBDevice.attach();
#endif

    setup();

    for (;;)
    {
        loop();
        if (serialEventRun) 
            serialEventRun();
    }

    return 0;
}

extern "C" void __cxa_pure_virtual() { while (true); }
#endif // defined(__SAM3X8E__) && defined(ARDUINO_SAM_DUE)

#if defined(STM32F4XX) || defined(STM32F7XX)
#if defined(STM32F4XX)
#include <stm32f4xx_flash.h>
#endif
#if defined(STM32F7XX)
#include <stm32f7xx_flash.h>
#endif

#define STM32_CNF_PAGE_ADDR (uint32_t)0x08010000
#define STM32_CNF_PAGE      ((uint32_t *)0x08010000)
#define STM32_CNF_SECTOR    FLASH_Sector_4
#define STM32_CNF_PAGE_24   24U

// --------------------------------------------------------------------------
//  Firmware Entry Point
// --------------------------------------------------------------------------

void jumpToBootLoader()
{
    // Disable RCC, set it to default (after reset) settings Internal clock, no PLL, etc.
    RCC_DeInit();
    ADC_DeInit();
    DAC_DeInit();
    TIM_DeInit(TIM2);
    USART_DeInit(USART1);
    USART_DeInit(UART5);

    // Disable Systick timer
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    // Clear Interrupt Enable Register & Interrupt Pending Register
    for (uint8_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    SYSCFG->MEMRMP = 0x01;

#if defined(STM32F4XX)
    volatile uint32_t addr = 0x1FFF0000;
#elif defined(STM32F7XX)
    volatile uint32_t addr = 0x1FF00000;
#endif

    void (*SysMemBootJump)(void);
    SysMemBootJump = (void (*)(void))(*((uint32_t *)(addr + 4)));
    __set_MSP(*(uint32_t *)addr);
    SysMemBootJump();
}

int main()
{
    // does the configuration page contain the request bootloader flag?
    if ((STM32_CNF_PAGE[STM32_CNF_PAGE_24] != 0xFFFFFFFFU) && (STM32_CNF_PAGE[STM32_CNF_PAGE_24] != 0x00U)) {
        uint8_t bootloadMode = (STM32_CNF_PAGE[STM32_CNF_PAGE_24] >> 8) & 0xFFU;
        if ((bootloadMode & 0x20U) == 0x20U) {
            // we unfortunately need to discard the configuration area entirely for bootloader mode...
            FLASH_Unlock();
            FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPERR);

#if defined(STM32F4XX) || defined(STM32F7XX)
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
            jumpToBootLoader();
        }
    }

    setup();

    for (;;)
        loop();
}
#endif // defined(STM32F4XX)

#if defined(NATIVE_SDR)
void fatal(const char* message)
{
    ::fprintf(stderr, "%s: %s\n", g_progExe.c_str(), message);
    exit(EXIT_FAILURE);
}

void usage(const char* message, const char* arg)
{
    ::fprintf(stdout, "" DESCRIPTION " (built %s)\r\n", __BUILD__);
    ::fprintf(stdout, "Copyright (c) 2025 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors.\n");
    ::fprintf(stdout, "Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others\n\n");
    if (message != nullptr) {
        ::fprintf(stderr, "%s: ", g_progExe.c_str());
        ::fprintf(stderr, message, arg);
        ::fprintf(stderr, "\n\n");
    }

    ::fprintf(stdout, 
        "usage: %s [-bdvh]"
        " [--syslog]" 
        " [-r <ZeroMQ Rx IPC Endpoint>] [-t <ZeroMQ Tx IPC Endpoint>]"
        " [-p <PTY port>]"
        " [-l <log filename>]\n\n"
        "  -b       background process\n"
        "\n"
        "  -d       enable debug\n"
        "  -v       show version information\n"
        "  -h       show this screen\n"
        "\n"
        "  --syslog  force logging to syslog\n"
        "\n"
        "  -r       ZeroMQ Rx IPC Endpoint\n"
        "  -t       ZeroMQ Tx IPC Endpoint\n"
        "\n"
        "  -p       PTY Port\n"
        "\n"
        "  -l       Log Filename\n"
        "\n"
        "  --       stop handling options\n",
        g_progExe.c_str());
    exit(EXIT_FAILURE);
}

int checkArgs(int argc, char* argv[])
{
    int i, p = 0;

    // iterate through arguments
    for (i = 1; i <= argc; i++)
    {
        if (argv[i] == nullptr) {
            break;
        }

        if (*argv[i] != '-') {
            continue;
        }
        else if (IS("--")) {
            ++p;
            break;
        }
        else if (IS("-r")) {
            if ((argc - 1) <= 0)
                usage("error: %s", "must specify the ZeroMQ Rx IPC Endpoint");
            m_zmqRx = std::string(argv[++i]);

            if (m_zmqRx == "")
                usage("error: %s", "IPC endpoint cannot be blank!");

            p += 2;
        }
        else if (IS("-t")) {
            if ((argc - 1) <= 0)
                usage("error: %s", "must specify the ZeroMQ Tx IPC Endpoint");
            m_zmqTx = std::string(argv[++i]);

            if (m_zmqTx == "")
                usage("error: %s", "IPC endpoint cannot be blank!");

            p += 2;
        }
        else if (IS("-p")) {
            if ((argc - 1) <= 0)
                usage("error: %s", "must specify the PTY port");
            m_ptyPort = std::string(argv[++i]);

            if (m_ptyPort == "")
                usage("error: %s", "PTY port cannot be blank!");

            p += 2;
        }
        else if (IS("-l")) {
            if ((argc - 1) <= 0)
                usage("error: %s", "must specify the log filename");
            g_logFileName = std::string(argv[++i]);

            if (g_logFileName == "")
                usage("error: %s", "log filename cannot be blank!");

            p += 2;
        }
        else if (IS("-b")) {
            ++p;
            g_daemon = true;
        }
        else if (IS("-d")) {
            ++p;
            g_debug = true;
        }
        else if (IS("--syslog")) {
            g_useSyslog = true;
        }
        else if (IS("-v")) {
            ::fprintf(stdout, "" DESCRIPTION " (built %s)\r\n", __BUILD__);
            ::fprintf(stdout, "Copyright (c) 2022 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors.\r\n");
            ::fprintf(stdout, "Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others\r\n");
            if (argc == 2)
                exit(EXIT_SUCCESS);
        }
        else if (IS("-h")) {
            usage(nullptr, nullptr);
            if (argc == 2)
                exit(EXIT_SUCCESS);
        }
        else {
            usage("unrecognized option `%s'", argv[i]);
        }
    }

    if (p < 0 || p > argc) {
        p = 0;
    }

    return ++p;
}

static void sigHandler(int signum)
{
    g_killed = true;
    g_signal = signum;
}

// ---------------------------------------------------------------------------
//  Program Entry Point
// ---------------------------------------------------------------------------

int main(int argc, char** argv)
{
    m_zmqRx = std::string("ipc:///tmp/dvm-rx.ipc");
    m_zmqTx = std::string("ipc:///tmp/dvm-tx.ipc");

    if (argv[0] != nullptr && *argv[0] != 0)
        g_progExe = std::string(argv[0]);

    if (argc > 1) {
        // check arguments
        int i = checkArgs(argc, argv);
        if (i < argc) {
            argc -= i;
            argv += i;
        }
        else {
            argc--;
            argv++;
        }
    }

    ::signal(SIGINT, sigHandler);
    ::signal(SIGTERM, sigHandler);
    ::signal(SIGHUP, sigHandler);

    // initialize system logging
    bool ret = ::LogInitialise(".", g_logFileName.c_str(), 1U, 1U);
    if (!ret) {
        ::fprintf(stderr, "unable to open the log file\n");
        return 1;
    }

    // handle POSIX process forking
    if (g_daemon) {
        // create new process
        pid_t pid = ::fork();
        if (pid == -1) {
            ::fprintf(stderr, "%s: Couldn't fork() , exiting\n", g_progExe.c_str());
            ::LogFinalise();
            return EXIT_FAILURE;
        }
        else if (pid != 0) {
            ::LogFinalise();
            exit(EXIT_SUCCESS);
        }

        // create new session and process group
        if (::setsid() == -1) {
            ::fprintf(stderr, "%s: Couldn't setsid(), exiting\n", g_progExe.c_str());
            ::LogFinalise();
            return EXIT_FAILURE;
        }

        // set the working directory to the root directory
        if (::chdir("/") == -1) {
            ::fprintf(stderr, "%s: Couldn't cd /, exiting\n", g_progExe.c_str());
            ::LogFinalise();
            return EXIT_FAILURE;
        }

        ::close(STDIN_FILENO);
        ::close(STDOUT_FILENO);
        ::close(STDERR_FILENO);
    }

    do {
        g_signal = 0;

        {
            ::LogInfo("" DESCRIPTION " (built %s)", __BUILD__);
            ::LogInfo("Copyright (c) 2017-2025 Bryan Biedenkapp, N2PLL and DVMProject (https://github.com/dvmproject) Authors.");
            ::LogInfo("Portions Copyright (c) 2015-2021 by Jonathan Naylor, G4KLX and others");

            ::LogInfoEx("DSP is performing initialization and warmup");
            setup();

            ::LogInfoEx("DSP is up and running");
            while (!g_killed) {
                loop();
                ::usleep(1);
            }
        }

        if (g_signal == 2)
            ::LogInfoEx("Exited on receipt of SIGINT");

        if (g_signal == 15)
            ::LogInfoEx("Exited on receipt of SIGTERM");

        if (g_signal == 1)
            ::LogInfoEx("Restarting on receipt of SIGHUP");
    } while (g_signal == 1);

    ::LogInfoEx("DSP is shutting down");

    if (m_serialPort != nullptr) {
        m_serialPort->close();
        delete m_serialPort;
    }

    try
    {
        m_zmqSocketTx.close();
        m_zmqSocketRx.close();
    }
    catch(const zmq::error_t& zmqE) { /* stub */ }
    catch(const std::exception& e) { /* stub */ }

    ::LogFinalise();
    return 0;
}
#endif // defined(NATIVE_SDR)

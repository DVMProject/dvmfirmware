// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
 *  Copyright (C) 2018-2025 Bryan Biedenkapp, N2PLL
 *
 */
#if !defined(__LOG_H__)
#define __LOG_H__

#include "Defines.h"

#include <string>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#define LOG_DSP     "DSP"

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

/**
 * @brief Macro helper to create a debug log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogDebug(fmt, ...)             Log(1U, LOG_DSP, __FILE__, __LINE__, nullptr, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a debug log entry.
 * @param _func Name of function generating log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogDebugEx(_func, fmt, ...)    Log(1U, LOG_DSP, __FILE__, __LINE__, _func, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a message log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogMessage(fmt, ...)           Log(2U, LOG_DSP, nullptr, 0, nullptr, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a informational log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function. LogInfo() does not use a module
 * name when creating a log entry.
 */
#define LogInfo(fmt, ...)               Log(3U, nullptr, nullptr, 0, nullptr, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a informational log entry with module name.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogInfoEx(fmt, ...)            Log(3U, LOG_DSP, nullptr, 0, nullptr, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a warning log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogWarning(fmt, ...)           Log(4U, LOG_DSP, nullptr, 0, nullptr, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a error log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogError(fmt, ...)             Log(5U, LOG_DSP, nullptr, 0, nullptr, fmt, ##__VA_ARGS__)
/**
 * @brief Macro helper to create a fatal log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function.
 */
#define LogFatal(fmt, ...)             Log(6U, LOG_DSP, nullptr, 0, nullptr, fmt, ##__VA_ARGS__)

// ---------------------------------------------------------------------------
//  Externs
// ---------------------------------------------------------------------------

/**
 * @brief (Global) Flag indicating whether or not logging goes to the syslog.
 */
extern bool g_useSyslog;

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

/**
 * @brief Initializes the diagnostics log.
 * @param filePath File path for the log file.
 * @param fileRoot Root name for log file.
 * @param fileLevel File log level.
 * @param displaylevel Display log level.
 * @param displayTimeDisplay Flag to disable the date and time stamp for the log entries.
 * @param syslog Flag indicating whether or not logs will be sent to syslog.
 * @returns 
 */
extern DSP_FW_API bool LogInitialise(const std::string& filePath, const std::string& fileRoot, uint32_t fileLevel, uint32_t displayLevel, bool disableTimeDisplay = false, bool useSyslog = false);
/**
 * @brief Finalizes the diagnostics log.
 */
extern DSP_FW_API void LogFinalise();
/**
 * @brief Writes a new entry to the diagnostics log.
 * @param level Log level for entry.
 * @param module Name of module generating log entry.
 * @param file Name of source code file generating log entry.
 * @param line Line number in source code file generating log entry.
 * @param func Name of function generating log entry.
 * @param fmt String format.
 * 
 * This is a variable argument function. This shouldn't be called directly, utilize the LogXXXX macros above, instead.
 */
extern DSP_FW_API void Log(uint32_t level, const char* module, const char* file, const int lineNo, const char* func, const char* fmt, ...);

#endif // __LOG_H__

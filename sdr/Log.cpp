// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Common Library
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2015,2016 Jonathan Naylor, G4KLX
 *  Copyright (C) 2018-2025 Bryan Biedenkapp, N2PLL
 *
 */
#include "Log.h"

#include <sys/time.h>
#if !defined(_WIN32)
#include <syslog.h>
#endif // !defined(_WIN32)

#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <ctime>
#include <cassert>
#include <cstring>

// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------

#define EOL    "\r\n"

const uint32_t LOG_BUFFER_LEN = 4096U;

// ---------------------------------------------------------------------------
//  Global Variables
// ---------------------------------------------------------------------------

static uint32_t m_fileLevel = 0U;
static std::string m_filePath;
static std::string m_fileRoot;

static FILE* m_fpLog = nullptr;

static uint32_t g_logDisplayLevel = 2U;
static bool g_disableTimeDisplay = false;

bool g_useSyslog = false;

static struct tm m_tm;

static char LEVELS[] = " DMIWEF";

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

/* Helper to open the detailed log file, file handle. */

static bool LogOpen()
{
    if (m_fileLevel == 0U)
        return true;

    if (!g_useSyslog) {
        time_t now;
        ::time(&now);

        struct tm* tm = ::localtime(&now);

        if (tm->tm_mday == m_tm.tm_mday && tm->tm_mon == m_tm.tm_mon && tm->tm_year == m_tm.tm_year) {
            if (m_fpLog != nullptr)
                return true;
        }
        else {
            if (m_fpLog != nullptr)
                ::fclose(m_fpLog);
        }

        char filename[200U];
        ::sprintf(filename, "%s/%s-%04d-%02d-%02d.log", m_filePath.c_str(), m_fileRoot.c_str(), tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        m_fpLog = ::fopen(filename, "a+t");
        m_tm = *tm;

        return m_fpLog != nullptr;
    }
    else {
#if !defined(_WIN32)
        switch (m_fileLevel) {
        case 1U:
            setlogmask(LOG_UPTO(LOG_DEBUG));
            break;
        case 2U:
            setlogmask(LOG_UPTO(LOG_INFO));
            break;
        case 3U:
            setlogmask(LOG_UPTO(LOG_NOTICE));
            break;
        case 4U:
            setlogmask(LOG_UPTO(LOG_WARNING));
            break;
        case 5U:
        default:
            setlogmask(LOG_UPTO(LOG_ERR));
            break;
        }

        openlog(m_fileRoot.c_str(), LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
        return true;
#else
        return false;
#endif // !defined(_WIN32)
    }
}

/* Initializes the diagnostics log. */

bool LogInitialise(const std::string& filePath, const std::string& fileRoot, uint32_t fileLevel, uint32_t displayLevel, bool disableTimeDisplay, bool useSyslog)
{
    m_filePath = filePath;
    m_fileRoot = fileRoot;
    m_fileLevel = fileLevel;
    g_logDisplayLevel = displayLevel;
    g_disableTimeDisplay = disableTimeDisplay;
#if defined(_WIN32)
    g_useSyslog = false;
#else
    if (!g_useSyslog)
        g_useSyslog = useSyslog;
#endif // defined(_WIN32)
    return ::LogOpen();
}

/// <summary>
/// Finalizes the diagnostics log.
/// </summary>
void LogFinalise()
{
    if (m_fpLog != nullptr)
        ::fclose(m_fpLog);
#if !defined(_WIN32)
    if (g_useSyslog)
        closelog();
#endif // !defined(_WIN32)
}

/* Writes a new entry to the diagnostics log. */

void Log(uint32_t level, const char *module, const char* file, const int lineNo, const char* func, const char* fmt, ...)
{
    assert(fmt != nullptr);

    char buffer[LOG_BUFFER_LEN];
    if (!g_disableTimeDisplay && !g_useSyslog) {
        time_t now;
        ::time(&now);
        struct tm* tm = ::localtime(&now);

        struct timeval nowMillis;
        ::gettimeofday(&nowMillis, NULL);

        if (module != nullptr) {
            // level 1 is DEBUG
            if (level == 1U) {
                // if we have a file and line number -- add that to the log entry
                if (file != nullptr && lineNo > 0) {
                    // if we have a function name add that to the log entry
                    if (func != nullptr) {
                        ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu (%s)[%s:%u][%s] ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U, module, file, lineNo, func);
                    }
                    else {
                        ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu (%s)[%s:%u] ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U, module, file, lineNo);
                    }
                } else {
                    ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu (%s) ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U, module);
                }
            } else {
                ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu (%s) ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U, module);
            }
        }
        else {
            // level 1 is DEBUG
            if (level == 1U) {
                // if we have a file and line number -- add that to the log entry
                if (file != nullptr && lineNo > 0) {
                    // if we have a function name add that to the log entry
                    if (func != nullptr) {
                        ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu [%s:%u][%s] ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U, file, lineNo, func);
                    }
                    else {
                        ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu [%s:%u] ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U, file, lineNo);
                    }
                } else {
                    ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U);
                }
            } else {
                ::sprintf(buffer, "%c: %04d-%02d-%02d %02d:%02d:%02d.%03lu ", LEVELS[level], tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, nowMillis.tv_usec / 1000U);
            }
        }
    }
    else {
        if (module != nullptr) {
            // level 1 is DEBUG
            if (level == 1U) {
                // if we have a file and line number -- add that to the log entry
                if (file != nullptr && lineNo > 0) {
                    // if we have a function name add that to the log entry
                    if (func != nullptr) {
                        ::sprintf(buffer, "%c: (%s)[%s:%u][%s] ", LEVELS[level], module, file, lineNo, func);
                    }
                    else {
                        ::sprintf(buffer, "%c: (%s)[%s:%u] ", LEVELS[level], module, file, lineNo);
                    }
                }
                else {
                    ::sprintf(buffer, "%c: (%s) ", LEVELS[level], module);
                }
            } else {
                ::sprintf(buffer, "%c: (%s) ", LEVELS[level], module);
            }
        }
        else {
            if (level >= 9999U) {
                ::sprintf(buffer, "U: ");
            }
            else {
                 // if we have a file and line number -- add that to the log entry
                 if (file != nullptr && lineNo > 0) {
                    // if we have a function name add that to the log entry
                    if (func != nullptr) {
                        ::sprintf(buffer, "%c: [%s:%u][%s] ", LEVELS[level], file, lineNo, func);
                    }
                    else {
                        ::sprintf(buffer, "%c: [%s:%u] ", LEVELS[level], file, lineNo);
                    }
                }
                else {
                    ::sprintf(buffer, "%c: ", LEVELS[level]);
                }
            }
        }
    }

    va_list vl, vl_len;
    va_start(vl, fmt);
    va_copy(vl_len, vl);

    size_t len = ::vsnprintf(nullptr, 0U, fmt, vl_len);
    ::vsnprintf(buffer + ::strlen(buffer), len + 1U, fmt, vl);

    va_end(vl_len);
    va_end(vl);

    if (level >= m_fileLevel && m_fileLevel != 0U) {
        if (!g_useSyslog) {
            bool ret = ::LogOpen();
            if (!ret)
                return;

            if (m_fpLog != nullptr) {
                ::fprintf(m_fpLog, "%s\n", buffer);
                ::fflush(m_fpLog);
            }
        } else {
#if !defined(_WIN32)
            // convert our log level into syslog level
            int syslogLevel = LOG_INFO;
            switch (level) {
            case 1U:
                syslogLevel = LOG_DEBUG;
                break;
            case 2U:
                syslogLevel = LOG_NOTICE;
                break;
            case 3U:
            case 9999U: // in-band U: messages should also be info level
                syslogLevel = LOG_INFO;
                break;
            case 4U:
                syslogLevel = LOG_WARNING;
                break;
            case 5U:
                syslogLevel = LOG_ERR;
                break;
            default:
                syslogLevel = LOG_EMERG;
                break;
            }

            syslog(syslogLevel, "%s", buffer);
#endif // !defined(_WIN32)
        }
    }

    if (!g_useSyslog && level >= g_logDisplayLevel && g_logDisplayLevel != 0U) {
        ::fprintf(stdout, "%s" EOL, buffer);
        ::fflush(stdout);
    }

    // fatal error (specially allow any log levels above 9999)
    if (level >= 6U && level < 9999U) {
        if (m_fpLog != nullptr)
            ::fclose(m_fpLog);
#if !defined(_WIN32)
        if (g_useSyslog)
            ::closelog();
#endif // !defined(_WIN32)
        exit(1);
    }
}

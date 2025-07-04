/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2025  The Mana Developers
 *
 *  This file is part of The Mana Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <cstdarg>

#ifdef __GNUC__
#  define LOG_PRINTF_ATTR __attribute__((__format__(__printf__, 1, 2)))
#else
#  define LOG_PRINTF_ATTR
#endif

/**
 * The Log namespace: Useful to write debug or info messages to the log file
 * and/or console. The messages will be timestamped.
 */
namespace Log
{
    /**
     * Initializes the log system.
     */
    void init();

    /**
     * Sets the file to log to and opens it.
     */
    void setLogFile(const std::string &logFilename);

    /**
     * Sets whether the log should be written to standard output.
     */
    void setLogToStandardOut(bool value);

    void verbose(const char *log_text, ...) LOG_PRINTF_ATTR;
    void debug(const char *log_text, ...) LOG_PRINTF_ATTR;
    void info(const char *log_text, ...) LOG_PRINTF_ATTR;
    void warn(const char *log_text, ...) LOG_PRINTF_ATTR;
    void error(const char *log_text, ...) LOG_PRINTF_ATTR;

    void vinfo(const char *log_text, va_list ap);

    /**
     * Log an error and quit. The error will be printed to standard error
     * and shown in a simple message box.
     */
    __attribute__((noreturn))
    void critical(const std::string &error_text);

} // namespace Log

#undef LOG_PRINTF_ATTR

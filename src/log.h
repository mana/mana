/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2026  The Mana Developers
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

#include "utils/format.h"

#include <string>
#include <utility>

/**
 * The Log namespace: Useful to write debug or info messages to the log file
 * and/or console. The messages will be timestamped.
 */
namespace Log
{
    enum class Priority
    {
        Verbose,
        Debug,
        Info,
        Warn,
        Error,
    };

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

    /**
     * Logs an already formatted message with the given priority.
     */
    void message(Priority priority, const std::string &text);

    template<typename... Args>
    void verbose(fmt::format_string<Args...> format, Args &&...args)
    {
        message(Priority::Verbose, fmt::format(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void debug(fmt::format_string<Args...> format, Args &&...args)
    {
        message(Priority::Debug, fmt::format(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void info(fmt::format_string<Args...> format, Args &&...args)
    {
        message(Priority::Info, fmt::format(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void warn(fmt::format_string<Args...> format, Args &&...args)
    {
        message(Priority::Warn, fmt::format(format, std::forward<Args>(args)...));
    }

    template<typename... Args>
    void error(fmt::format_string<Args...> format, Args &&...args)
    {
        message(Priority::Error, fmt::format(format, std::forward<Args>(args)...));
    }

    /**
     * Logs an error and quits. The error will be printed to standard error
     * and shown in a simple message box.
     */
    [[noreturn]] void critical(const std::string &text);

    template<typename T, typename... Args>
    [[noreturn]] void critical(fmt::format_string<T, Args...> format,
                               T &&arg, Args &&...args)
    {
        critical(fmt::format(format, std::forward<T>(arg),
                             std::forward<Args>(args)...));
    }

} // namespace Log

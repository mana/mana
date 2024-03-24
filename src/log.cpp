/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#include "log.h"

#include <SDL.h>

#include <sys/time.h>
#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cstdio>

Logger::Logger() = default;
Logger::~Logger() = default;

void Logger::setLogFile(const std::string &logFilename)
{
    mLogFile.open(logFilename, std::ios_base::trunc);

    if (!mLogFile.is_open())
    {
        std::cout << "Warning: error while opening " << logFilename <<
            " for writing.\n";
    }
}

void Logger::log(const char *log_text, ...)
{
    const size_t bufSize = 1024;
    char* buf = new char[bufSize];
    va_list ap;

    // Use a temporary buffer to fill in the variables
    va_start(ap, log_text);
    vsnprintf(buf, bufSize, log_text, ap);
    va_end(ap);

    // Get the current system time
    timeval tv;
    gettimeofday(&tv, nullptr);

    // Print the log entry
    std::stringstream timeStr;
    timeStr << "["
        << ((((tv.tv_sec / 60) / 60) % 24 < 10) ? "0" : "")
        << (int)(((tv.tv_sec / 60) / 60) % 24)
        << ":"
        << (((tv.tv_sec / 60) % 60 < 10) ? "0" : "")
        << (int)((tv.tv_sec / 60) % 60)
        << ":"
        << ((tv.tv_sec % 60 < 10) ? "0" : "")
        << (int)(tv.tv_sec % 60)
        << "."
        << (((tv.tv_usec / 10000) % 100) < 10 ? "0" : "")
        << (int)((tv.tv_usec / 10000) % 100)
        << "] ";

    if (mLogFile.is_open())
    {
        mLogFile << timeStr.str() << buf << std::endl;
    }

    if (mLogToStandardOut)
    {
        std::cout << timeStr.str() << buf << std::endl;
    }

    // Delete temporary buffer
    delete[] buf;
}

void Logger::error(const std::string &error_text)
{
    log("Error: %s", error_text.c_str());
    std::cerr << "Error: " << error_text << std::endl;
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", error_text.c_str(), nullptr);
    exit(1);
}

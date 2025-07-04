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

#include "log.h"

#include <SDL.h>

#include <cstdarg>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/time.h>

static std::ofstream logFile;
static bool logToStandardOut = true;

static const char *getLogPriorityPrefix(SDL_LogPriority priority)
{
    switch (priority) {
    case SDL_LOG_PRIORITY_WARN:
        return "Warning: ";
    case SDL_LOG_PRIORITY_ERROR:
        return "Error: ";
    case SDL_LOG_PRIORITY_CRITICAL:
        return "Critical Error: ";
    default:
        return "";
    }
}

static void logOutputFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    // Get the current system time
    timeval tv;
    gettimeofday(&tv, nullptr);

    // Create timestamp string
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

    const char *prefix = getLogPriorityPrefix(priority);

    if (logToStandardOut)
    {
        std::cout << timeStr.str() << prefix << message << std::endl;
    }

    if (logFile.is_open())
    {
        logFile << timeStr.str() << prefix << message << std::endl;
    }
}

void Log::init()
{
    SDL_LogSetOutputFunction(logOutputFunction, nullptr);
}

void Log::setLogFile(const std::string &logFilename)
{
    logFile.open(logFilename, std::ios_base::trunc);

    if (!logFile.is_open())
    {
        std::cout << "Warning: error while opening " << logFilename
                  << " for writing.\n";
    }
}

void Log::setLogToStandardOut(bool value)
{
    logToStandardOut = value;
}

#define DEFINE_LOG_FUNCTION(name, priority) \
    void Log::name(const char *fmt, ...) \
    { \
        va_list ap; \
        va_start(ap, fmt); \
        SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, priority, fmt, ap); \
        va_end(ap); \
    }

DEFINE_LOG_FUNCTION(verbose, SDL_LOG_PRIORITY_VERBOSE)
DEFINE_LOG_FUNCTION(debug, SDL_LOG_PRIORITY_DEBUG)
DEFINE_LOG_FUNCTION(info, SDL_LOG_PRIORITY_INFO)
DEFINE_LOG_FUNCTION(warn, SDL_LOG_PRIORITY_WARN)
DEFINE_LOG_FUNCTION(error, SDL_LOG_PRIORITY_ERROR)

#undef DEFINE_LOG_FUNCTION

void Log::vinfo(const char *fmt, va_list ap)
{
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt, ap);
}

void Log::critical(const std::string &message)
{
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", message.c_str());

    if (!logToStandardOut)
    {
        std::cerr << getLogPriorityPrefix(SDL_LOG_PRIORITY_CRITICAL) << message << std::endl;
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", message.c_str(), nullptr);
    exit(1);
}

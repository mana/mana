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

#include "log.h"

#include <SDL.h>

#include <fstream>
#include <iostream>
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
    const std::string timeStr = fmt::format("[{:02}:{:02}:{:02}.{:02}] ",
                                            (tv.tv_sec / 3600) % 24,
                                            (tv.tv_sec / 60) % 60,
                                            tv.tv_sec % 60,
                                            (tv.tv_usec / 10000) % 100);

    const char *prefix = getLogPriorityPrefix(priority);

    if (logToStandardOut)
    {
        std::cout << timeStr << prefix << message << std::endl;
    }

    if (logFile.is_open())
    {
        logFile << timeStr << prefix << message << std::endl;
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

static SDL_LogPriority toSdlPriority(Log::Priority priority)
{
    switch (priority) {
    case Log::Priority::Verbose:
        return SDL_LOG_PRIORITY_VERBOSE;
    case Log::Priority::Debug:
        return SDL_LOG_PRIORITY_DEBUG;
    case Log::Priority::Info:
        return SDL_LOG_PRIORITY_INFO;
    case Log::Priority::Warn:
        return SDL_LOG_PRIORITY_WARN;
    case Log::Priority::Error:
        return SDL_LOG_PRIORITY_ERROR;
    }
    return SDL_LOG_PRIORITY_INFO;
}

void Log::message(Priority priority, const std::string &text)
{
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, toSdlPriority(priority),
                   "%s", text.c_str());
}

void Log::critical(const std::string &text)
{
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "%s", text.c_str());

    if (!logToStandardOut)
    {
        std::cerr << getLogPriorityPrefix(SDL_LOG_PRIORITY_CRITICAL) << text << std::endl;
    }

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", text.c_str(), nullptr);
    exit(1);
}

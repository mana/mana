/*
 *  The Mana World
 *  Copyright (C) 2009-2012  The Mana Developers
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "chatlogger.h"

#include <iostream>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#ifdef _WIN32
#include <windows.h>
#elif defined __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "configuration.h"

#include "utils/stringutils.h"

ChatLogger::ChatLogger()
{
}

ChatLogger::~ChatLogger()
{
    if (mLogFile.is_open())
        mLogFile.close();
}

void ChatLogger::setLogFile(const std::string &logFilename)
{
    if (mLogFile.is_open())
        mLogFile.close();

    mLogFile.open(logFilename, std::ios_base::app);

    if (!mLogFile.is_open())
    {
        std::cout << "Warning: error while opening " << logFilename <<
            " for writing.\n";
    }
}

void ChatLogger::setLogDir(const std::string &logDir)
{
    mLogDir = logDir;

    if (mLogFile.is_open())
        mLogFile.close();

    DIR *dir = opendir(mLogDir.c_str());
    if (!dir)
        makeDir(mLogDir);
    else
        closedir(dir);
}

void ChatLogger::log(std::string str)
{
    std::string dateStr = getDateString();
    if (!mLogFile.is_open() || dateStr != mLogDate)
    {
        mLogDate = dateStr;
        setLogFile(strprintf("%s/%s/#General_%s.log", mLogDir.c_str(),
                             mServerName.c_str(), dateStr.c_str()));
    }

    str = removeColors(str);
    writeTo(mLogFile, str);
}

void ChatLogger::log(std::string name, std::string str)
{
    std::ofstream logFile;
    logFile.open(strprintf("%s/%s/%s_%s.log", mLogDir.c_str(), mServerName.c_str(),
                          secureName(name).c_str(), getDateString().c_str()).c_str(),
                 std::ios_base::app);

    if (!logFile.is_open())
        return;

    str = removeColors(str);
    writeTo(logFile, str);

    if (logFile.is_open())
        logFile.close();
}

std::string ChatLogger::getDateString() const
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time (&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 79, "%y-%m-%d", timeinfo);
    return buffer;
}

std::string ChatLogger::secureName(std::string &name)
{
    const size_t sz = name.length();
    for (size_t f = 0; f < sz; f ++)
    {
        const unsigned char ch = name[f];
        if ((ch < '0' || ch > '9') &&
            (ch < 'a' || ch > 'z') &&
            (ch < 'A' || ch > 'Z') &&
            ch != '-' &&
            ch != '+' &&
            ch != '=' &&
            ch != '.' &&
            ch != ',' &&
            ch != ')' &&
            ch != '(' &&
            ch != '[' &&
            ch != ']' &&
            ch != '#')
        {
            name[f] = '_';
        }
    }
    return name;
}

void ChatLogger::writeTo(std::ofstream &file, const std::string &str) const
{
    file << str << std::endl;
}

void ChatLogger::setServerName(const std::string &serverName)
{
    mServerName = serverName;
    if (mServerName.empty() && !config.servers.empty())
        mServerName = config.servers.front().hostname;

    if (mLogFile.is_open())
        mLogFile.close();

    secureName(mServerName);
    if (!mLogDir.empty())
    {
        DIR *dir = opendir((mLogDir + "/" + mServerName).c_str());
        if (!dir)
            makeDir(mLogDir + "/" + mServerName);
        else
            closedir(dir);
    }
}

void ChatLogger::makeDir(const std::string &dir)
{
#ifdef _WIN32
    mkdir(dir.c_str());
#else
    mkdir(dir.c_str(), 0750);
#endif
}

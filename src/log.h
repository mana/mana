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

#pragma once

#include <fstream>

/**
 * The Log Class : Useful to write debug or info messages
 */
class Logger
{
    public:
        Logger();

        /**
         * Destructor, closes log file.
         */
        ~Logger();

        /**
         * Sets the file to log to and opens it
         */
        void setLogFile(const std::string &logFilename);

        /**
         * Sets whether the log should be written to standard output.
         */
        void setLogToStandardOut(bool value) { mLogToStandardOut = value; }

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log(const char *log_text, ...)
#ifdef __GNUC__
            __attribute__((__format__(__printf__, 2, 3)))
#endif
            ;

        /**
         * Log an error and quit. The error will be printed to standard error
         * and showm in a simple message box.
         */
        __attribute__((noreturn))
        void error(const std::string &error_text);

    private:
        std::ofstream mLogFile;
        bool mLogToStandardOut = true;
};

extern Logger *logger;

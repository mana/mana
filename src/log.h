/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef _LOG_H
#define _LOG_H

#include <fstream>

class ChatWindow;

/**
 * The Log Class : Useful to write debug or info messages
 */
class Logger
{
    public:
        /**
         * Constructor.
         */
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
         * Enables logging to chat window
         */
        void setChatWindow(ChatWindow *window) { mChatWindow = window; }

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log(const char *log_text, ...)
#ifdef __GNUC__
            __attribute__((__format__(__printf__, 2, 3)))
#endif
            ;

        /**
         * Log an error and quit. The error will pop-up on Windows and Mac, and
         * will be printed to standard error everywhere else.
         */
        void error(const std::string &error_text);

    private:
        std::ofstream mLogFile;
        bool mLogToStandardOut;
        ChatWindow *mChatWindow;
};

extern Logger *logger;

#endif

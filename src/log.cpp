/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "log.h"

FILE* logfile;


#define LOG_FILE "./docs/tmw.log"


/*
 * Initializes log file by opening it for writing.
 */
void init_log() {
    logfile = fopen(LOG_FILE, "w");
    if (!logfile) {
        printf("Warning: error while opening log file.\n");
    }
}

/*
 * Enter a message in the log with a certain category. The message will be
 * timestamped.
 */
void log(const char *category, const char *log_text, ...) {
    if (logfile) {
        char* buf = new char[1024];
        va_list ap;
        time_t t;

        // Use a temporary buffer to fill in the variables
        va_start(ap, log_text);
        vsprintf(buf, log_text, ap);
        va_end(ap);

        // Get the current system time
        time(&t);

        // Print the log entry
        fprintf(logfile,
                "[%s%d:%s%d:%s%d] %s: %s\n",
                (((t / 60) / 60) % 24 < 10) ? "0" : "",
                (int)(((t / 60) / 60) % 24),
                ((t / 60) % 60 < 10) ? "0" : "",
                (int)((t / 60) % 60),
                (t % 60 < 10) ? "0" : "",
                (int)(t % 60),
                category, buf
               );

        // Flush the log file
        fflush(logfile);

        // Delete temporary buffer
        delete[] buf;
    }
}

/*
 * Log an error and quit. The error will pop-up in Windows and will be printed
 * to standard error everywhere else. 
 */
void error(const std::string error_text) {
    log("Error", error_text.c_str());

#ifdef WIN32
    MessageBox(NULL, error_text.c_str(), "Error", MB_ICONERROR|MB_OK);
#else
    fprintf(stderr, "Error: %s\n", error_text.c_str());
#endif
    exit(1);
}

/*
 * Shortcut to log a warning.
 */
void warning(const char *warning_text) {
    log("Warning", warning_text);
}

/*
 * Shortcut to log a status update, will only really be logged in debug mode.
 */
void status(const char *status_text) {
    log("Status", status_text);
}

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

int warning_n; // Keep warning number
FILE* logfile;


#define LOG_FILE "./docs/tmw.log"



void init_log() {
    logfile = fopen(LOG_FILE, "w");
    if (!logfile) {
        printf("Warning: error while opening log file.\n");
    }
    warning_n = 0;
}

void log(const char *category, const char *log_text, ...) {
    if (logfile) {
        char* buf = new char[1024];

        va_list ap;
        va_start(ap, log_text);
        vsprintf(buf, log_text, ap);
        va_end(ap);

        time_t t;
        time(&t);

        fprintf(
                logfile,
                "[%s%d:%s%d:%s%d] %s: %s\n",
                (((t / 60) / 60) % 24 < 10) ? "0" : "",
                (int)(((t / 60) / 60) % 24),
                ((t / 60) % 60 < 10) ? "0" : "",
                (int)((t / 60) % 60),
                (t % 60 < 10) ? "0" : "",
                (int)(t % 60),
                category, buf
               );
        fflush(logfile);

        delete[] buf;
    }
}


void error(const char *error_text) {
    log("Error", error_text);

#ifdef WIN32
    MessageBox(NULL, error_text, "Error", MB_ICONERROR|MB_OK);
#else
    printf("Error: %s", error_text);
#endif
    exit(1);
}

void warning(const char *warning_text) {
    log("Warning", warning_text);
}

void status(const char *status_text) {
#ifdef DEBUG
    log("Status", status_text);
#endif
}

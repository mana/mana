/*
 *  The Mana Client
 *  Copyright (C) 2010  The Mana Developers
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

#include <climits>
#include <cstring>
#include <cerrno>

#if defined WIN32
#include <windows.h>
#endif

#include <sys/stat.h>

#ifdef _MKDIR_TEST_
// compile with -D_MKDIR_TEST_ to get a standalone binary
#include <cstdio>
#include <cstdlib>
#endif

#include "mkdir.h"

int mkdir_r(const char *pathname) {
    char tmp[PATH_MAX];
    char *p;

    if (strlen(pathname) >= PATH_MAX-2)
        return -1;

    strncpy(tmp, pathname, sizeof(tmp)-1);
    tmp[PATH_MAX-1] = '\0';

    int len=strlen(tmp);

    // terminate the pathname with '/'
    if (tmp[len-1] != '/') {
        tmp[len] = '/';
        tmp[len+1] = '\0';
    }

    for (p=tmp; *p; p++) {
#if defined WIN32
        if (*p == '/' || *p == '\\')
#else
        if (*p == '/')
#endif
        {
            *p = '\0';
            // ignore a slash at the beginning of a path
            if (strlen(tmp) == 0){
                *p = '/';
                continue;
            }

            // check if the name already exists, but not as directory
            struct stat statbuf;
            if (!stat(tmp, &statbuf))
            {
                if (S_ISDIR(statbuf.st_mode))
                {
                    *p = '/';
                    continue;
                }
                else
                    return -1;
            }

#if defined WIN32
            if (!CreateDirectory(tmp, 0))
#else
            if (mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
#endif
            {
#if defined WIN32
                // hack, hack. just assume that x: might be a drive
                // letter, and try again
                if (!(strlen(tmp) == 2 &&
                      !strcmp(tmp + 1, ":")))
#endif
                return -1;
            }

#ifdef _MKDIR_TEST_
            printf("%s\n", tmp);
#endif
            *p = '/';
        }
    }
    return 0;
}

#ifdef _MKDIR_TEST_
int main(int argc, char** argv) {
    if (argc < 2) exit(1);
    mkdir_r(argv[1]);
}
#endif

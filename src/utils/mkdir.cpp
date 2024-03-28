/*
 *  The Mana Client
 *  Copyright (C) 2010-2024  The Mana Developers
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
#include <filesystem>
#include <iostream>

#ifdef MKDIR_TEST
// compile with -DMKDIR_TEST to get a standalone binary
#include <cstdio>
#include <cstdlib>
#endif

#include "mkdir.h"

/// Create a directory, making leading components first if necessary
int mkdir_r(const char *pathname) {
    try {
        std::filesystem::create_directories(pathname);
        return 0;
    } catch (const std::exception&) {}

    return -1;
}

#ifdef MKDIR_TEST
int main(int argc, char** argv) {
    if (argc < 2) exit(1);
    mkdir_r(argv[1]);
}
#endif

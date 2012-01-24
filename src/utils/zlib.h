/*
 *  The Mana Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
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

#ifndef UTILS_ZLIB_H
#define UTILS_ZLIB_H

#include <string>

/**
 * Inflates either zlib or gzip deflated memory. The inflated memory is
 * expected to be freed by the caller.
 */
int inflateMemory(unsigned char *in, unsigned int inLength,
                  unsigned char *&out, unsigned int &outLength);

int inflateMemory(unsigned char *in, unsigned int inLength,
                  unsigned char *&out);

/**
 * Loads the given file from the filesystem, uncompressing if it ends in ".gz".
 *
 * @param filename The name of the file to be loaded and uncompressed
 * @param filesize The size of the file that was loaded and uncompressed.
 *
 * @return An allocated byte array containing the data that was loaded and
 *         uncompressed, or <code>NULL</code> on fail.
 */
void *loadCompressedFile(const std::string &filename, int &filesize);

#endif // UTILS_ZLIB_H

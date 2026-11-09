/*
 *  The Mana Client
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

#include "net/download.h"

#include <zlib.h>

#include <cassert>
#include <cstdlib>
#include <cstring>

namespace Net {

/**
 * Calculates the Alder-32 checksum for the given file.
 */
unsigned long Download::fadler32(FILE *file)
{
    if (!file || fseek(file, 0, SEEK_END) != 0)
        return 0;

    const long fileSize = ftell(file);
    if (fileSize < 0)
        return 0;

    if (fseek(file, 0, SEEK_SET) != 0)
        return 0;

    // Calculate Adler-32 checksum
    void *buffer = malloc(fileSize);
    const size_t read = fread(buffer, 1, fileSize, file);
    unsigned long adler = adler32_z(0L, Z_NULL, 0);
    adler = adler32_z(adler, static_cast<Bytef*>(buffer), read);
    free(buffer);

    return adler;
}

void Download::noCache()
{
    addHeader("pragma: no-cache");
    addHeader("Cache-Control: no-cache");
}

void Download::setFile(const std::string &filename,
                       std::optional<unsigned long> adler32)
{
    assert(!mStarted);   // Cannot set file after starting download

    mMemoryWrite = false;
    mFileName = filename;
    mAdler = adler32;
}

void Download::setUseBuffer()
{
    assert(!mStarted);   // Cannot set write function after starting download

    mMemoryWrite = true;
}

std::string_view Download::getBuffer() const
{
    assert(mMemoryWrite);   // Buffer not used
    return {mBuffer, mDownloadedBytes};
}

void Download::setError(const std::string &error)
{
    strncpy(mError, error.c_str(), sizeof(mError) - 1);
    mError[sizeof(mError) - 1] = 0;
}

} // namespace Net

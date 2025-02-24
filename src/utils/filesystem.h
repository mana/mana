/*
 *  The Mana Client
 *  Copyright (C) 2024  The Mana Developers
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

// Suppress deprecation warnings for PHYSFS_getUserDir
#define PHYSFS_DEPRECATED

#include "utils/bufferedrwops.h"
#include "utils/physfsrwops.h"

#include <optional>
#include <string>

/**
 * These functions wrap PHYSFS functions to provide a more user-friendly
 * interface and to limit the direct use of the PHYSFS API to a single file.
 */
namespace FS {

inline bool init(const char *argv0)
{
    return PHYSFS_init(argv0) != 0;
}

inline void deinit()
{
    PHYSFS_deinit();
}

inline const char *getDirSeparator()
{
    return PHYSFS_getDirSeparator();
}

inline const char *getBaseDir()
{
    return PHYSFS_getBaseDir();
}

inline const char *getUserDir()
{
    return PHYSFS_getUserDir();
}

inline const char *getPrefDir(const char *org, const char *app)
{
    return PHYSFS_getPrefDir(org, app);
}

/**
 * Sets the write directory.
 *
 * @param path The path of the directory to be added.
 * @return <code>true</code> on success, <code>false</code> otherwise.
 */
inline bool setWriteDir(const std::string &path)
{
    return PHYSFS_setWriteDir(path.c_str()) != 0;
}

/**
 * Adds a directory or archive to the search path. If append is true
 * then the directory is added to the end of the search path, otherwise
 * it is added at the front.
 *
 * @return <code>true</code> on success, <code>false</code> otherwise.
 */
inline bool addToSearchPath(const std::string &path, bool append)
{
    return PHYSFS_mount(path.c_str(), "/", append ? 1 : 0) != 0;
}

/**
 * Checks whether the given file or directory exists in the search path.
 */
inline bool exists(const std::string &path)
{
    return PHYSFS_exists(path.c_str()) != 0;
}

inline std::optional<const char *> getRealDir(const std::string &path)
{
    auto dir = PHYSFS_getRealDir(path.c_str());
    return dir ? std::optional<const char *>(dir) : std::nullopt;
}

/**
 * Checks whether the given path is a directory.
 */
inline bool isDirectory(const std::string &path)
{
    PHYSFS_Stat stat;
    if (PHYSFS_stat(path.c_str(), &stat) != 0)
    {
        return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
    }
    return false;
}

/**
 * Creates a directory in the write path.
 */
inline bool mkdir(const std::string &path)
{
    return PHYSFS_mkdir(path.c_str()) != 0;
}

/**
 * Helper class to iterate over the files in a directory.
 * Based on https://stackoverflow.com/a/79051293/355419.
 */
class Files
{
public:
    struct End {};
    friend bool operator!=(const char *const *files, End)
    { return *files != nullptr; }

    explicit Files(char **files) : mFiles(files) {}
    ~Files() { PHYSFS_freeList(mFiles); }

    Files(const Files &) = delete;
    Files &operator=(const Files &) = delete;

    // Relies on C++17 support for begin/end to not have the same return type
    const char* const *begin() const { return mFiles; }
    End end() const { return End(); }

private:
    char **mFiles;
};

/**
 * Returns a list of files in the given directory.
 */
inline Files enumerateFiles(const std::string &dir)
{
    return Files(PHYSFS_enumerateFiles(dir.c_str()));
}

/**
 * File wrapper class to provide a more convenient API and automatic closing.
 */
class File
{
public:
    explicit File(PHYSFS_file *file)
        : file(file)
    {}

    ~File()
    {
        if (isOpen())
            close();
    }

    bool isOpen() const
    {
        return file != nullptr;
    }

    operator bool() const
    {
        return isOpen();
    }

    bool close()
    {
        if (PHYSFS_close(file) != 0)
        {
            file = nullptr;
            return true;
        }
        return false;
    }

    std::optional<size_t> read(void *data, size_t size)
    {
        auto len = PHYSFS_readBytes(file, data, size);
        return len >= 0 ? std::optional<size_t>(len) : std::nullopt;
    }

    std::optional<size_t> write(const void *data, size_t size)
    {
        auto len = PHYSFS_writeBytes(file, data, size);
        return len >= 0 ? std::optional<size_t>(len) : std::nullopt;
    }

    bool flush()
    {
        return PHYSFS_flush(file) != 0;
    }

    bool seek(size_t pos)
    {
        return PHYSFS_seek(file, pos) != 0;
    }

    std::optional<size_t> fileLength() const
    {
        auto len = PHYSFS_fileLength(file);
        return len >= 0 ? std::optional<size_t>(len) : std::nullopt;
    }

    std::optional<size_t> tell() const
    {
        auto pos = PHYSFS_tell(file);
        return pos >= 0 ? std::optional<size_t>(pos) : std::nullopt;
    }

    bool eof() const
    {
        return PHYSFS_eof(file) != 0;
    }

private:
    PHYSFS_file *file;
};

inline File openWrite(const std::string &path)
{
    return File(PHYSFS_openWrite(path.c_str()));
}

inline File openAppend(const std::string &path)
{
    return File(PHYSFS_openAppend(path.c_str()));
}

inline File openRead(const std::string &path)
{
    return File(PHYSFS_openRead(path.c_str()));
}

inline const char *getLastError()
{
    return PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());
}

//
// Helper functions for loading files through SDL_RWops
//

inline SDL_RWops *openRWops(const std::string &path)
{
    return PHYSFSRWOPS_openRead(path.c_str());
}

/**
 * Creates a buffered SDL_RWops.
 *
 * Used to workaround a performance issue when SDL_mixer is using stb_vorbis.
 * The overhead of calling PHYSFS_readBytes each time is too high because
 * stb_vorbis requests the file one byte at a time.
 *
 * See https://github.com/libsdl-org/SDL_mixer/issues/670
 */
inline SDL_RWops *openBufferedRWops(const std::string &path)
{
    auto rw = PHYSFSRWOPS_openRead(path.c_str());
    if (auto buffered = createBufferedRWops(rw))
        return buffered;
    return rw;
}

inline void *loadFile(const std::string &path, size_t &datasize)
{
    auto file = openRWops(path);
    if (!file)
        return nullptr;

    return SDL_LoadFile_RW(file, &datasize, 1);
}

} // namespace FS

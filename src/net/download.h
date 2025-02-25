/*
 *  The Mana Client
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

#include <cstdio>
#include <string>
#include <optional>

#include <curl/curl.h>

#pragma once

enum DownloadStatus
{
    DOWNLOAD_STATUS_CANCELLED = -3,
    DOWNLOAD_STATUS_THREAD_ERROR = -2,
    DOWNLOAD_STATUS_ERROR = -1,
    DOWNLOAD_STATUS_STARTING = 0,
    DOWNLOAD_STATUS_IN_PROGRESS,
    DOWNLOAD_STATUS_COMPLETE
};

struct SDL_Thread;

namespace Net {

class Download
{
    public:
        /**
         * Callback function for download updates.
         *
         * @param ptr       Pointer passed to Download constructor
         * @param status    Current download status
         * @param dltotal   Total number of bytes to download
         * @param dlnow     Number of bytes downloaded so far
         */
        using DownloadUpdate = int (*)(void *, DownloadStatus, size_t, size_t);

        Download(void *ptr, const std::string &url, DownloadUpdate updateFunction);
        ~Download();

        void addHeader(const char *header);

        /**
         * Convience method for adding no-cache headers.
         */
        void noCache();

        void setFile(const std::string &filename,
                     std::optional<unsigned long> adler32 = {});

        void setWriteFunction(curl_write_callback write);

        /**
         * Starts the download thread.
         * @returns true  if thread was created
         *          false if the thread could not be made or download wasn't
         *                properly setup
         */
        bool start();

        /**
         * Cancels the download. Returns immediately, the cancelled status will
         * be noted in the next available update call.
         */
        void cancel();

        const char *getError() const;

        static unsigned long fadler32(FILE *file);

    private:
        static int downloadThread(void *ptr);
        static int downloadProgress(void *clientp,
                                    curl_off_t dltotal, curl_off_t dlnow,
                                    curl_off_t ultotal, curl_off_t ulnow);
        void *mPtr;
        std::string mUrl;
        struct {
            unsigned cancel : 1;
            unsigned memoryWrite: 1;
        } mOptions;
        std::string mFileName;
        curl_write_callback mWriteFunction = nullptr;
        std::optional<unsigned long> mAdler;
        DownloadUpdate mUpdateFunction;
        SDL_Thread *mThread = nullptr;
        curl_slist *mHeaders = nullptr;
        char *mError;
};

} // namespace Net

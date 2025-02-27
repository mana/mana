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

#include "utils/mutex.h"

#include <cstdio>
#include <optional>
#include <string>

#include <curl/curl.h>

#pragma once

enum class DownloadStatus
{
    IN_PROGRESS,
    CANCELED,
    ERROR,
    COMPLETE
};

struct SDL_Thread;

namespace Net {

class Download
{
    public:
        struct State
        {
            DownloadStatus status = DownloadStatus::IN_PROGRESS;
            float progress = 0.0f;
        };

        Download(const std::string &url);
        ~Download();

        void addHeader(const char *header);

        /**
         * Convience method for adding no-cache headers.
         */
        void noCache();

        void setFile(const std::string &filename,
                     std::optional<unsigned long> adler32 = {});

        void setUseBuffer();

        /**
         * Starts the download thread.
         * @returns whether the thread could be created
         */
        bool start();

        /**
         * Cancels the download. Returns immediately, the canceled status will
         * be noted in the next available update call.
         */
        void cancel();

        /**
         * Returns a view on the downloaded data.
         */
        std::string_view getBuffer() const;

        State getState();

        const char *getError() const;

        static unsigned long fadler32(FILE *file);

    private:
        static int downloadProgress(void *clientp,
                                    curl_off_t dltotal, curl_off_t dlnow,
                                    curl_off_t ultotal, curl_off_t ulnow);

        static size_t writeBuffer(char *ptr, size_t size, size_t nmemb,
                                  void *stream);

        static int downloadThread(void *ptr);

        ThreadSafe<State> mState;
        std::string mUrl;
        bool mCancel = false;
        bool mMemoryWrite = false;
        std::string mFileName;
        std::optional<unsigned long> mAdler;
        SDL_Thread *mThread = nullptr;
        curl_slist *mHeaders = nullptr;
        char mError[CURL_ERROR_SIZE];

        /** Byte count currently downloaded in mMemoryBuffer. */
        size_t mDownloadedBytes = 0;

        /** Buffer for files downloaded to memory. */
        char *mBuffer = nullptr;
};

inline Download::State Download::getState()
{
    return *mState.lock();
}

inline const char *Download::getError() const
{
    return mError;
}

} // namespace Net

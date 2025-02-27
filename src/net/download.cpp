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

#include "net/download.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "utils/stringutils.h"

#include <SDL.h>
#include <SDL_thread.h>

#include <zlib.h>

constexpr char DOWNLOAD_ERROR_MESSAGE_THREAD[] = "Could not create download thread!";

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

    rewind(file);

    // Calculate Adler-32 checksum
    void *buffer = malloc(fileSize);
    const size_t read = fread(buffer, 1, fileSize, file);
    unsigned long adler = adler32_z(0L, Z_NULL, 0);
    adler = adler32_z(adler, (Bytef*) buffer, read);
    free(buffer);

    return adler;
}

Download::Download(const std::string &url)
    : mUrl(url)
{
    mError[0] = 0;
}

Download::~Download()
{
    mCancel = true;
    SDL_WaitThread(mThread, nullptr);

    curl_slist_free_all(mHeaders);
    free(mBuffer);
}

void Download::addHeader(const char *header)
{
    assert(!mThread);   // Cannot add headers after starting download

    mHeaders = curl_slist_append(mHeaders, header);
}

void Download::noCache()
{
    addHeader("pragma: no-cache");
    addHeader("Cache-Control: no-cache");
}

void Download::setFile(const std::string &filename,
                       std::optional<unsigned long> adler32)
{
    assert(!mThread);   // Cannot set file after starting download

    mMemoryWrite = false;
    mFileName = filename;
    mAdler = adler32;
}

void Download::setUseBuffer()
{
    assert(!mThread);   // Cannot set write function after starting download

    mMemoryWrite = true;
}

bool Download::start()
{
    assert(!mThread);   // Download already started

    logger->log("Starting download: %s", mUrl.c_str());

    mThread = SDL_CreateThread(downloadThread, "Download", this);

    if (!mThread)
    {
        logger->log("%s", DOWNLOAD_ERROR_MESSAGE_THREAD);
        strncpy(mError, DOWNLOAD_ERROR_MESSAGE_THREAD, CURL_ERROR_SIZE - 1);
        mState.lock()->status = DownloadStatus::ERROR;
        return false;
    }

    return true;
}

void Download::cancel()
{
    logger->log("Canceling download: %s", mUrl.c_str());
    mCancel = true;
}

std::string_view Download::getBuffer() const
{
    assert(mMemoryWrite);   // Buffer not used
    return std::string_view(mBuffer, mDownloadedBytes);
}

/**
 * A libcurl callback for reporting progress.
 */
int Download::downloadProgress(void *clientp,
                               curl_off_t dltotal, curl_off_t dlnow,
                               curl_off_t ultotal, curl_off_t ulnow)
{
    auto *d = reinterpret_cast<Download*>(clientp);

    auto state = d->mState.lock();
    state->status = DownloadStatus::IN_PROGRESS;
    state->progress = 0.0f;
    if (dltotal > 0)
        state->progress = static_cast<float>(dlnow) / dltotal;

    return d->mCancel;
}

/**
 * A libcurl callback for writing to memory.
 */
size_t Download::writeBuffer(char *ptr, size_t size, size_t nmemb, void *stream)
{
    auto *d = reinterpret_cast<Download *>(stream);

    const size_t totalMem = size * nmemb;
    d->mBuffer = (char *) realloc(d->mBuffer, d->mDownloadedBytes + totalMem);
    if (d->mBuffer)
    {
        memcpy(d->mBuffer + d->mDownloadedBytes, ptr, totalMem);
        d->mDownloadedBytes += totalMem;
    }

    return totalMem;
}

int Download::downloadThread(void *ptr)
{
    auto *d = reinterpret_cast<Download*>(ptr);
    bool complete = false;
    std::string outFilename;

    if (!d->mMemoryWrite)
        outFilename = d->mFileName + ".part";

    for (int attempts = 0; attempts < 3 && !complete && !d->mCancel; ++attempts)
    {
        CURL *curl = curl_easy_init();
        if (!curl)
            break;

        logger->log("Downloading: %s", d->mUrl.c_str());

        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, d->mHeaders);

        FILE *file = nullptr;

        if (d->mMemoryWrite)
        {
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Download::writeBuffer);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, ptr);
        }
        else
        {
            file = fopen(outFilename.c_str(), "w+b");
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
        }

        const std::string appShort = branding.getStringValue("appShort");
        const std::string userAgent =
                strprintf(PACKAGE_EXTENDED_VERSION, appShort.c_str());

        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, d->mError);
        curl_easy_setopt(curl, CURLOPT_URL, d->mUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &Download::downloadProgress);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, ptr);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15);

        const CURLcode res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_ABORTED_BY_CALLBACK)
        {
            d->mCancel = true;

            if (file)
            {
                fclose(file);
                ::remove(outFilename.c_str());
            }

            break;
        }

        if (res != CURLE_OK)
        {
            logger->log("curl error %d: %s host: %s",
                        res, d->mError, d->mUrl.c_str());

            if (file)
            {
                fclose(file);
                ::remove(outFilename.c_str());
            }

            break;
        }

        if (!d->mMemoryWrite)
        {
            // Check the checksum if available
            if (d->mAdler)
            {
                unsigned long adler = fadler32(file);

                if (d->mAdler != adler)
                {
                    if (file)
                        fclose(file);

                    // Remove the corrupted file
                    ::remove(outFilename.c_str());
                    logger->log("Checksum for file %s failed: (%lx/%lx)",
                        d->mFileName.c_str(),
                        adler, *d->mAdler);

                    continue; // Bail out here to avoid the renaming
                }
            }

            if (file)
                fclose(file);

            // Any existing file with this name is deleted first, otherwise
            // the rename will fail on Windows.
            ::remove(d->mFileName.c_str());
            ::rename(outFilename.c_str(), d->mFileName.c_str());

            // Check if we can open it and no errors were encountered
            // during renaming
            file = fopen(d->mFileName.c_str(), "rb");
            if (file)
            {
                fclose(file);
                file = nullptr;
                complete = true;
            }
        }
        else
        {
            // It's stored in memory, we're done
            complete = true;
        }

        if (file)
            fclose(file);
    }

    auto state = d->mState.lock();
    if (d->mCancel)
        state->status = DownloadStatus::CANCELED;
    else if (complete)
        state->status = DownloadStatus::COMPLETE;
    else
        state->status = DownloadStatus::ERROR;

    return 0;
}

} // namespace Net

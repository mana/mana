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

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "utils/stringutils.h"

#include <SDL.h>
#include <SDL_thread.h>

#include <zlib.h>

#include <cassert>
#include <cstdarg>
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

    rewind(file);

    // Calculate Adler-32 checksum
    void *buffer = malloc(fileSize);
    const size_t read = fread(buffer, 1, fileSize, file);
    unsigned long adler = adler32_z(0L, Z_NULL, 0);
    adler = adler32_z(adler, (Bytef*) buffer, read);
    free(buffer);

    return adler;
}

#ifdef __EMSCRIPTEN__

/*
 * In the browser there are no threads to spare and no libcurl. The download is
 * an emscripten_fetch, whose callbacks run on the main thread between frames.
 * The public interface is unchanged: callers still poll getState() once per
 * frame.
 */

Download::Download(const std::string &url)
    : mUrl(url)
{
    mError[0] = 0;
}

Download::~Download()
{
    mCancel = true;
    closeFetch();

    free(mBuffer);
}

void Download::addHeader(const char *header)
{
    assert(!mStarted);   // Cannot add headers after starting download

    // emscripten_fetch wants the name and the value as separate strings
    const char *colon = strchr(header, ':');
    if (!colon)
    {
        mHeaders.emplace_back(header);
        mHeaders.emplace_back();
        return;
    }

    mHeaders.emplace_back(header, colon - header);

    ++colon;
    while (*colon == ' ')
        ++colon;
    mHeaders.emplace_back(colon);
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

bool Download::start()
{
    assert(!mStarted);   // Download already started
    mStarted = true;

    Log::info("Starting download: %s", mUrl.c_str());

    // No more headers can be added from here on, so the strings are stable and
    // it is safe to hand out pointers into them.
    if (!mHeaders.empty())
    {
        mHeaderPointers.reserve(mHeaders.size() + 1);
        for (const std::string &header : mHeaders)
            mHeaderPointers.push_back(header.c_str());
        mHeaderPointers.push_back(nullptr);
    }

    if (!startFetch())
    {
        setError("Could not start download of %s", mUrl.c_str());
        Log::info("%s", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    return true;
}

void Download::cancel()
{
    Log::info("Canceling download: %s", mUrl.c_str());

    mCancel = true;
    closeFetch();
    mState.lock()->status = DownloadStatus::Canceled;
}

std::string_view Download::getBuffer() const
{
    assert(mMemoryWrite);   // Buffer not used
    return std::string_view(mBuffer, mDownloadedBytes);
}

bool Download::startFetch()
{
    ++mAttempts;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = &Download::fetchSuccess;
    attr.onerror = &Download::fetchError;
    attr.onprogress = &Download::fetchProgress;
    attr.userData = this;
    if (!mHeaderPointers.empty())
        attr.requestHeaders = mHeaderPointers.data();

    Log::info("Downloading: %s", mUrl.c_str());

    mFetch = emscripten_fetch(&attr, mUrl.c_str());
    return mFetch != nullptr;
}

void Download::closeFetch()
{
    if (!mFetch)
        return;

    emscripten_fetch_t *fetch = mFetch;
    mFetch = nullptr;

    // Detach first, so a callback fired while closing cannot reach this object
    fetch->userData = nullptr;
    emscripten_fetch_close(fetch);
}

void Download::setError(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vsnprintf(mError, sizeof(mError), format, ap);
    va_end(ap);
}

/**
 * Called on the main thread once the whole response has been received.
 */
void Download::fetchSuccess(emscripten_fetch_t *fetch)
{
    auto *d = static_cast<Download *>(fetch->userData);
    if (!d)
        return;     // Canceled or deleted, the handle is no longer ours

    d->mFetch = nullptr;
    d->handleSuccess(fetch);
    emscripten_fetch_close(fetch);
}

void Download::fetchError(emscripten_fetch_t *fetch)
{
    auto *d = static_cast<Download *>(fetch->userData);
    if (!d)
        return;

    d->mFetch = nullptr;
    d->handleError(fetch);
    emscripten_fetch_close(fetch);
}

void Download::fetchProgress(emscripten_fetch_t *fetch)
{
    auto *d = static_cast<Download *>(fetch->userData);
    if (!d)
        return;

    auto state = d->mState.lock();
    state->status = DownloadStatus::InProgress;
    state->progress = 0.0f;

    if (fetch->totalBytes > 0)
    {
        const double received = static_cast<double>(fetch->dataOffset) +
                                static_cast<double>(fetch->numBytes);
        state->progress = static_cast<float>(received / fetch->totalBytes);
    }
}

void Download::handleSuccess(emscripten_fetch_t *fetch)
{
    if (mCancel)
    {
        mState.lock()->status = DownloadStatus::Canceled;
        return;
    }

    const size_t size = static_cast<size_t>(fetch->numBytes);

    if (mMemoryWrite)
    {
        // Same ownership as the libcurl version: a malloc'd buffer owned by
        // this object, freed in the destructor and read through getBuffer().
        free(mBuffer);
        mBuffer = nullptr;
        mDownloadedBytes = 0;

        if (size > 0)
        {
            mBuffer = (char *) malloc(size);
            if (!mBuffer)
            {
                setError("Out of memory while downloading %s", mUrl.c_str());
                Log::info("%s", mError);
                mState.lock()->status = DownloadStatus::Error;
                return;
            }

            memcpy(mBuffer, fetch->data, size);
            mDownloadedBytes = size;
        }
    }
    else if (!writeToFile(fetch->data, size))
    {
        // Either another attempt is in flight, or the error state is set
        return;
    }

    auto state = mState.lock();
    state->progress = 1.0f;
    state->status = DownloadStatus::Complete;
}

void Download::handleError(emscripten_fetch_t *fetch)
{
    if (mCancel)
    {
        mState.lock()->status = DownloadStatus::Canceled;
        return;
    }

    setError("HTTP %d: %s", fetch->status, fetch->statusText);
    Log::info("fetch error %d: %s host: %s",
              fetch->status, fetch->statusText, mUrl.c_str());

    mState.lock()->status = DownloadStatus::Error;
}

/**
 * Writes the received bytes to "<file>.part", verifies the checksum and moves
 * the result into place. Returns whether the download is complete. On failure
 * the state is set to Error, unless another attempt was started.
 */
bool Download::writeToFile(const char *data, size_t size)
{
    const std::string partialName = mFileName + ".part";

    FILE *file = fopen(partialName.c_str(), "w+b");
    if (!file)
    {
        setError("Could not open %s for writing", partialName.c_str());
        Log::info("%s", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    if (size > 0 && fwrite(data, 1, size, file) != size)
    {
        fclose(file);
        ::remove(partialName.c_str());

        setError("Could not write %s", partialName.c_str());
        Log::info("%s", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    // Check the checksum if available
    if (mAdler)
    {
        const unsigned long adler = fadler32(file);

        if (*mAdler != adler)
        {
            fclose(file);

            // Remove the corrupted file
            ::remove(partialName.c_str());
            Log::info("Checksum for file %s failed: (%lx/%lx)",
                      mFileName.c_str(), adler, *mAdler);

            // Like the libcurl version, a failed checksum is retried up to
            // three times in total before giving up.
            if (mAttempts < 3 && !mCancel && startFetch())
                return false;

            setError("Checksum for file %s failed", mFileName.c_str());
            mState.lock()->status = DownloadStatus::Error;
            return false;
        }
    }

    fclose(file);

    // Any existing file with this name is deleted first, otherwise the rename
    // will fail on Windows.
    ::remove(mFileName.c_str());
    if (::rename(partialName.c_str(), mFileName.c_str()) != 0)
    {
        setError("Could not rename %s to %s",
                 partialName.c_str(), mFileName.c_str());
        Log::info("%s", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    return true;
}

#else // __EMSCRIPTEN__

constexpr char DOWNLOAD_ERROR_MESSAGE_THREAD[] = "Could not create download thread!";

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

    Log::info("Starting download: %s", mUrl.c_str());

    mThread = SDL_CreateThread(downloadThread, "Download", this);

    if (!mThread)
    {
        Log::info("%s", DOWNLOAD_ERROR_MESSAGE_THREAD);
        strncpy(mError, DOWNLOAD_ERROR_MESSAGE_THREAD, CURL_ERROR_SIZE - 1);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    return true;
}

void Download::cancel()
{
    Log::info("Canceling download: %s", mUrl.c_str());
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
    state->status = DownloadStatus::InProgress;
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

        Log::info("Downloading: %s", d->mUrl.c_str());

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

        const std::string appShort = branding.shortName();
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

#if defined(_WIN32) && defined(CURLSSLOPT_NATIVE_CA)
        // Verify against the certificate store of Windows. Without this, an
        // OpenSSL based libcurl looks for a CA bundle at the path it was
        // compiled with, which does not exist on the user's machine.
        curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NATIVE_CA);
#endif

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
            Log::info("curl error %d: %s host: %s",
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
                    Log::info("Checksum for file %s failed: (%lx/%lx)",
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
        state->status = DownloadStatus::Canceled;
    else if (complete)
        state->status = DownloadStatus::Complete;
    else
        state->status = DownloadStatus::Error;

    return 0;
}

#endif // __EMSCRIPTEN__

} // namespace Net

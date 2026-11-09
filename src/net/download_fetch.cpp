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

#include "log.h"

#include "utils/format.h"

#include <emscripten/emscripten.h>
#include <emscripten/fetch.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>

/*
 * In the browser there are no threads to spare and no libcurl. The download is
 * an emscripten_fetch, whose callbacks run on the main thread between frames.
 * The public interface is unchanged: callers still poll getState() once per
 * frame.
 */

/**
 * Returns whether the page was loaded over HTTPS, in which case browsers
 * refuse to fetch plain HTTP resources.
 */
static bool pageIsSecure()
{
    static const bool secure = EM_ASM_INT({
        return location.protocol === "https:" ? 1 : 0;
    }) != 0;
    return secure;
}

namespace Net {


Download::Download(std::string url)
    : mUrl(std::move(url))
{
    mError[0] = 0;

    // Game servers hand out plain http:// update hosts for the sake of old
    // clients. From a secure page such a download would be blocked as mixed
    // content, so it is upgraded to https:// instead.
    if (pageIsSecure() && mUrl.compare(0, 7, "http://") == 0)
        mUrl.replace(0, 7, "https://");
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

bool Download::start()
{
    assert(!mStarted);   // Download already started
    mStarted = true;

    Log::info("Starting download: {}", mUrl);

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
        setError(fmt::format("Could not start download of {}", mUrl));
        Log::info("{}", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    return true;
}

void Download::cancel()
{
    Log::info("Canceling download: {}", mUrl);

    mCancel = true;
    closeFetch();
    mState.lock()->status = DownloadStatus::Canceled;
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

    Log::info("Downloading: {}", mUrl);

    mFetch = emscripten_fetch(&attr, mUrl.c_str());
    return mFetch != nullptr;
}

void Download::closeFetch()
{
    if (!mFetch)
        return;

    emscripten_fetch_t *closing = mFetch;
    mFetch = nullptr;

    // Detach first, so a callback fired while closing cannot reach this object
    closing->userData = nullptr;
    emscripten_fetch_close(closing);
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
                setError(fmt::format(
                        "Out of memory while downloading {}", mUrl));
                Log::info("{}", mError);
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

    setError(fmt::format("HTTP {}: {}",
                                   fetch->status, fetch->statusText));
    Log::info("fetch error {}: {} host: {}",
              fetch->status, fetch->statusText, mUrl);

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
        setError(fmt::format("Could not open {} for writing",
                                       partialName));
        Log::info("{}", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    if (size > 0 && fwrite(data, 1, size, file) != size)
    {
        fclose(file);
        ::remove(partialName.c_str());

        setError(fmt::format("Could not write {}", partialName));
        Log::info("{}", mError);
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
            Log::info("Checksum for file {} failed: ({:x}/{:x})",
                      mFileName, adler, *mAdler);

            // Like the libcurl version, a failed checksum is retried up to
            // three times in total before giving up.
            if (mAttempts < 3 && !mCancel && startFetch())
                return false;

            setError(fmt::format("Checksum for file {} failed",
                                           mFileName));
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
        setError(fmt::format("Could not rename {} to {}",
                                       partialName, mFileName));
        Log::info("{}", mError);
        mState.lock()->status = DownloadStatus::Error;
        return false;
    }

    return true;
}

} // namespace Net

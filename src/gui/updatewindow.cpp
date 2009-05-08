/*
 *  The Mana World
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "gui/updatewindow.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <iostream>
#include <SDL.h>
#include <SDL_thread.h>
#include <zlib.h>

#include <curl/curl.h>

/**
 * Calculates the Alder-32 checksum for the given file.
 */
static unsigned long fadler32(FILE *file)
{
    // Obtain file size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    // Calculate Adler-32 checksum
    char *buffer = (char*) malloc(fileSize);
    const size_t read = fread(buffer, 1, fileSize, file);
    unsigned long adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, (Bytef*) buffer, read);
    free(buffer);

    return adler;
}

/**
 * Load the given file into a vector of strings.
 */
std::vector<std::string> loadTextFile(const std::string &fileName)
{
    std::vector<std::string> lines;
    std::ifstream fin(fileName.c_str());

    if (!fin) {
        logger->log("Couldn't load text file: %s", fileName.c_str());
        return lines;
    }

    std::string line;

    while (getline(fin, line))
        lines.push_back(line);

    return lines;
}


UpdaterWindow::UpdaterWindow(const std::string &updateHost,
                             const std::string &updatesDir):
    Window(_("Updating...")),
    mThread(NULL),
    mDownloadStatus(UPDATE_NEWS),
    mUpdateHost(updateHost),
    mUpdatesDir(updatesDir),
    mCurrentFile("news.txt"),
    mCurrentChecksum(0),
    mStoreInMemory(true),
    mDownloadComplete(true),
    mUserCancel(false),
    mDownloadedBytes(0),
    mMemoryBuffer(NULL),
    mCurlError(new char[CURL_ERROR_SIZE]),
    mLineIndex(0)
{
    mCurlError[0] = 0;

    mBrowserBox = new BrowserBox;
    mScrollArea = new ScrollArea(mBrowserBox);
    mLabel = new Label(_("Connecting..."));
    mProgressBar = new ProgressBar(0.0, 310, 20, gcn::Color(168, 116, 31));
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mPlayButton = new Button(_("Play"), "play", this);

    mBrowserBox->setOpaque(false);
    mPlayButton->setEnabled(false);

    ContainerPlacer place;
    place = getPlacer(0, 0);

    place(0, 0, mScrollArea, 5, 3).setPadding(3);
    place(0, 3, mLabel, 5);
    place(0, 4, mProgressBar, 5);
    place(3, 5, mCancelButton);
    place(4, 5, mPlayButton);

    reflowLayout(320, 240);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    center();
    setVisible(true);
    mCancelButton->requestFocus();

    // Try to download the updates list
    download();
}

UpdaterWindow::~UpdaterWindow()
{
    if (mThread)
        SDL_WaitThread(mThread, NULL);

    free(mMemoryBuffer);

    // Remove possibly leftover temporary download
    ::remove((mUpdatesDir + "/download.temp").c_str());

    delete[] mCurlError;
}

void UpdaterWindow::setProgress(float p)
{
    mProgressBar->setProgress(p);
}

void UpdaterWindow::setLabel(const std::string &str)
{
    // Do delayed label text update, since Guichan isn't thread-safe
    MutexLocker lock(&mLabelMutex);
    mNewLabelCaption = str;
}

void UpdaterWindow::enable()
{
    mCancelButton->setEnabled(false);
    mPlayButton->setEnabled(true);
    mPlayButton->requestFocus();
}

void UpdaterWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
    {
        // Register the user cancel
        mUserCancel = true;
        // Skip the updating process
        if (mDownloadStatus != UPDATE_COMPLETE)
        {
            mDownloadStatus = UPDATE_ERROR;
        }
    }
    else if (event.getId() == "play")
    {
        state = STATE_LOADDATA;
    }
}

void UpdaterWindow::loadNews()
{
    if (!mMemoryBuffer)
    {
        logger->log("Couldn't load news");
        return;
    }

    // Reallocate and include terminating 0 character
    mMemoryBuffer = (char*)realloc(mMemoryBuffer, mDownloadedBytes + 1);
    mMemoryBuffer[mDownloadedBytes] = '\0';

    mBrowserBox->clearRows();

    // Tokenize and add each line separately
    char *line = strtok(mMemoryBuffer, "\n");
    while (line)
    {
        mBrowserBox->addRow(line);
        line = strtok(NULL, "\n");
    }

    // Free the memory buffer now that we don't need it anymore
    free(mMemoryBuffer);
    mMemoryBuffer = NULL;

    mScrollArea->setVerticalScrollAmount(0);
}

int UpdaterWindow::updateProgress(void *ptr,
                                  double dt, double dn, double ut, double un)
{
    float progress = dn / dt;
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(ptr);

    if (progress != progress) progress = 0.0f; // check for NaN
    if (progress < 0.0f) progress = 0.0f; // no idea how this could ever happen, but why not check for it anyway.
    if (progress > 1.0f) progress = 1.0f;

    uw->setLabel(
            uw->mCurrentFile + " (" + toString((int) (progress * 100)) + "%)");
    uw->setProgress(progress);

    if (state != STATE_UPDATE || uw->mDownloadStatus == UPDATE_ERROR)
    {
        // If the action was canceled return an error code to stop the mThread
        return -1;
    }

    return 0;
}

size_t UpdaterWindow::memoryWrite(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(stream);
    size_t totalMem = size * nmemb;
    uw->mMemoryBuffer = (char*) realloc(uw->mMemoryBuffer,
                                        uw->mDownloadedBytes + totalMem);
    if (uw->mMemoryBuffer)
    {
        memcpy(&(uw->mMemoryBuffer[uw->mDownloadedBytes]), ptr, totalMem);
        uw->mDownloadedBytes += totalMem;
    }

    return totalMem;
}

int UpdaterWindow::downloadThread(void *ptr)
{
    int attempts = 0;
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(ptr);
    CURL *curl;
    CURLcode res;
    std::string outFilename;
    std::string url(uw->mUpdateHost + "/" + uw->mCurrentFile);

    while (attempts < 3 && !uw->mDownloadComplete)
    {
        FILE *outfile = NULL;
        FILE *newfile = NULL;
        uw->setLabel(uw->mCurrentFile + " (0%)");

        curl = curl_easy_init();

        if (curl)
        {
            logger->log("Downloading: %s", url.c_str());

            if (uw->mStoreInMemory)
            {
                uw->mDownloadedBytes = 0;
                curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                                       UpdaterWindow::memoryWrite);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, ptr);
            }
            else
            {
                outFilename =  uw->mUpdatesDir + "/download.temp";
                outfile = fopen(outFilename.c_str(), "w+b");
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
            }

#ifdef PACKAGE_VERSION
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "TMW/" PACKAGE_VERSION);
#else
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "TMW");
#endif
            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, uw->mCurlError);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,
                                   UpdaterWindow::updateProgress);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15);

            struct curl_slist *pHeaders = NULL;
            if (uw->mDownloadStatus != UPDATE_RESOURCES)
            {
                // Make sure the resources2.txt and news.txt aren't cached,
                // in order to always get the latest version.
                pHeaders = curl_slist_append(pHeaders, "pragma: no-cache");
                pHeaders =
                    curl_slist_append(pHeaders, "Cache-Control: no-cache");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);
            }

            if ((res = curl_easy_perform(curl)) != 0)
            {
                uw->mDownloadStatus = UPDATE_ERROR;
                switch (res)
                {
                case CURLE_COULDNT_CONNECT:
                default:
                    std::cerr << _("curl error ") << res << ": "
                              << uw->mCurlError << _(" host: ") << url.c_str()
                              << std::endl;
                    break;
                }

                if (!uw->mStoreInMemory)
                {
                    fclose(outfile);
                    ::remove(outFilename.c_str());
                }
                attempts++;
                continue;
            }

            curl_easy_cleanup(curl);

            if (uw->mDownloadStatus != UPDATE_RESOURCES)
            {
                curl_slist_free_all(pHeaders);
            }

            if (!uw->mStoreInMemory)
            {
                // Don't check resources2.txt checksum
                if (uw->mDownloadStatus == UPDATE_RESOURCES)
                {
                    unsigned long adler = fadler32(outfile);

                    if (uw->mCurrentChecksum != adler)
                    {
                        fclose(outfile);

                        // Remove the corrupted file
                        ::remove(outFilename.c_str());
                        logger->log(
                            "Checksum for file %s failed: (%lx/%lx)",
                            uw->mCurrentFile.c_str(),
                            adler, uw->mCurrentChecksum);
                        attempts++;
                        continue; // Bail out here to avoid the renaming
                    }
                }
                fclose(outfile);

                // Give the file the proper name
                const std::string newName =
                    uw->mUpdatesDir + "/" + uw->mCurrentFile;

                // Any existing file with this name is deleted first, otherwise
                // the rename will fail on Windows.
                ::remove(newName.c_str());
                ::rename(outFilename.c_str(), newName.c_str());

                // Check if we can open it and no errors were encountered
                // during renaming
                newfile = fopen(newName.c_str(), "rb");
                if (newfile)
                {
                    fclose(newfile);
                    uw->mDownloadComplete = true;
                }
            }
            else
            {
                // It's stored in memory, we're done
                uw->mDownloadComplete = true;
            }
        }
        attempts++;
    }

    if (!uw->mDownloadComplete) {
        uw->mDownloadStatus = UPDATE_ERROR;
    }

    return 0;
}

void UpdaterWindow::download()
{
    mDownloadComplete = false;
    mThread = SDL_CreateThread(UpdaterWindow::downloadThread, this);

    if (!mThread)
    {
        logger->log("Unable to create mThread");
        mDownloadStatus = UPDATE_ERROR;
    }
}

void UpdaterWindow::logic()
{
    // Update Scroll logic
    mScrollArea->logic();

    // Synchronize label caption when necessary
    {
        MutexLocker lock(&mLabelMutex);

        if (mLabel->getCaption() != mNewLabelCaption)
        {
            mLabel->setCaption(mNewLabelCaption);
            mLabel->adjustSize();
        }
    }

    switch (mDownloadStatus)
    {
        case UPDATE_ERROR:
            if (mThread)
            {
                if (mUserCancel) {
                    // Kill the thread, because user has canceled
                    SDL_KillThread(mThread);
                    // Set the flag to false again
                    mUserCancel = false;
                }
                else {
                    SDL_WaitThread(mThread, NULL);
                }
                mThread = NULL;
            }
            mBrowserBox->addRow("");
            mBrowserBox->addRow(_("##1  The update process is incomplete."));
            mBrowserBox->addRow(_("##1  It is strongly recommended that"));
            mBrowserBox->addRow(_("##1  you try again later"));
            mBrowserBox->addRow(mCurlError);
            mScrollArea->setVerticalScrollAmount(
                    mScrollArea->getVerticalMaxScroll());
            mDownloadStatus = UPDATE_COMPLETE;
            break;
        case UPDATE_NEWS:
            if (mDownloadComplete)
            {
                // Parse current memory buffer as news and dispose of the data
                loadNews();

                mCurrentFile = "resources2.txt";
                mStoreInMemory = false;
                mDownloadStatus = UPDATE_LIST;
                download(); // download() changes mDownloadComplete to false
            }
            break;
        case UPDATE_LIST:
            if (mDownloadComplete)
            {
                mLines = loadTextFile(mUpdatesDir + "/resources2.txt");
                mStoreInMemory = false;
                mDownloadStatus = UPDATE_RESOURCES;
            }
            break;
        case UPDATE_RESOURCES:
            if (mDownloadComplete)
            {
                if (mThread)
                {
                    SDL_WaitThread(mThread, NULL);
                    mThread = NULL;
                }

                if (mLineIndex < mLines.size())
                {
                    std::stringstream line(mLines[mLineIndex]);
                    line >> mCurrentFile;
                    std::string checksum;
                    line >> checksum;
                    std::stringstream ss(checksum);
                    ss >> std::hex >> mCurrentChecksum;

                    std::ifstream temp(
                            (mUpdatesDir + "/" + mCurrentFile).c_str());

                    if (!temp.is_open())
                    {
                        temp.close();
                        download();
                    }
                    else
                    {
                        logger->log("%s already here", mCurrentFile.c_str());
                    }
                    mLineIndex++;
                }
                else
                {
                    // Download of updates completed
                    mDownloadStatus = UPDATE_COMPLETE;
                }
            }
            break;
        case UPDATE_COMPLETE:
            enable();
            setLabel(_("Completed"));
            break;
        case UPDATE_IDLE:
            break;
    }
}

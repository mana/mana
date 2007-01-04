/*
 *  The Mana World
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

#include "updatewindow.h"

#include <iostream>
#include <SDL.h>
#include <SDL_thread.h>
#include <zlib.h>

#include <curl/curl.h>

#include <guichan/widgets/label.hpp>

#include "browserbox.h"
#include "button.h"
#include "progressbar.h"
#include "scrollarea.h"

#include "../configuration.h"
#include "../log.h"
#include "../main.h"

#include "../utils/tostring.h"

#include "../resources/resourcemanager.h"

UpdaterWindow::UpdaterWindow():
    Window("Updating..."),
    mThread(NULL), mMutex(NULL), mDownloadStatus(UPDATE_NEWS),
    mUpdateHost(""), mCurrentFile("news.txt"), mBasePath(""),
    mStoreInMemory(true), mDownloadComplete(true), mUserCancel(false),
    mDownloadedBytes(0), mMemoryBuffer(NULL),
    mCurlError(new char[CURL_ERROR_SIZE]), mLineIndex(0)
{
    mCurlError[0] = 0;

    const int h = 240;
    const int w = 320;
    setContentSize(w, h);

    mBrowserBox = new BrowserBox();
    mScrollArea = new ScrollArea(mBrowserBox);
    mLabel = new gcn::Label("Connecting...");
    mProgressBar = new ProgressBar(0.0, w - 10, 20, 37, 70, 200);
    mCancelButton = new Button("Cancel", "cancel", this);
    mPlayButton = new Button("Play", "play", this);

    mBrowserBox->setOpaque(false);
    mPlayButton->setEnabled(false);

    mCancelButton->setPosition(5, h - 5 - mCancelButton->getHeight());
    mPlayButton->setPosition(
            mCancelButton->getX() + mCancelButton->getWidth() + 5,
            h - 5 - mPlayButton->getHeight());
    mProgressBar->setPosition(5, mCancelButton->getY() - 20 - 5);
    mLabel->setPosition(5, mProgressBar->getY() - mLabel->getHeight() - 5);

    mScrollArea->setDimension(gcn::Rectangle(5, 5, 310, mLabel->getY() - 12));

    add(mScrollArea);
    add(mLabel);
    add(mProgressBar);
    add(mCancelButton);
    add(mPlayButton);

    setLocationRelativeTo(getParent());
    setVisible(true);
    mCancelButton->requestFocus();

    mUpdateHost =
        config.getValue("updatehost", "http://updates.themanaworld.org");
    mBasePath = config.getValue("homeDir", ".");

    // Try to download the updates list
    download();
}

UpdaterWindow::~UpdaterWindow()
{
    if (mThread)
    {
         SDL_WaitThread(mThread, NULL);
         mThread = NULL;
    }

    if (mMemoryBuffer)
    {
        free(mMemoryBuffer);
    }

    // Remove possibly leftover temporary download
    ::remove((mBasePath + "/updates/download.temp").c_str());

    delete[] mCurlError;
}

void UpdaterWindow::setProgress(float p)
{
    mProgressBar->setProgress(p);
}

void UpdaterWindow::setLabel(const std::string &str)
{
    mLabel->setCaption(str);
    mLabel->adjustSize();
}

void UpdaterWindow::enable()
{
    mPlayButton->setEnabled(true);
    mPlayButton->requestFocus();
}

void UpdaterWindow::action(const std::string &eventId, gcn::Widget *widget)
{
    if (eventId == "cancel")
    {
        // Register the user cancel
        mUserCancel=true;
        // Skip the updating process
        if (mDownloadStatus == UPDATE_COMPLETE)
        {
            state = STATE_EXIT;
        }
        else
        {
            mDownloadStatus = UPDATE_ERROR;
        }
    }
    else if (eventId == "play")
    {
        state = STATE_LOGIN;
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
    while (line != NULL)
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

    if (progress < 0) progress = 0.0f;
    if (progress > 1) progress = 1.0f;

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

size_t UpdaterWindow::memoryWrite(void *ptr,
                                  size_t size, size_t nmemb, FILE *stream)
{
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(stream);
    size_t totalMem = size * nmemb;
    uw->mMemoryBuffer = (char*)realloc(uw->mMemoryBuffer,
                                       uw->mDownloadedBytes + totalMem + 1);
    if (uw->mMemoryBuffer)
    {
        memcpy(&(uw->mMemoryBuffer[uw->mDownloadedBytes]), ptr, totalMem);
        uw->mDownloadedBytes += totalMem;

        // Make sure the memory buffer is NULL terminated, because this
        // function is used to download text files that are later parsed as a
        // string.
        uw->mMemoryBuffer[uw->mDownloadedBytes] = 0;
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

    while (attempts < 3 && !uw->mDownloadComplete) {
        FILE *outfile = NULL;
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
                // Download in the proper folder : ./updates under win,
                // /home/user/.tmw/updates for unices
                outFilename =  uw->mBasePath + "/updates/download.temp";
                outfile = fopen(outFilename.c_str(), "w+b");
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
            }

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, uw->mCurlError);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION,
                                   UpdaterWindow::updateProgress);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 15);

            if ((res = curl_easy_perform(curl)) != 0)
            {
                uw->mDownloadStatus = UPDATE_ERROR;
                switch (res)
                {
                case CURLE_COULDNT_CONNECT: // give more debug info on that error
                    std::cerr << "curl error " << res << " : " << uw->mCurlError << " " << url.c_str()
                    << std::endl;
                    break;

                default:
                    std::cerr << "curl error " << res << " : " << uw->mCurlError << " host: " << url.c_str()
                    << std::endl;
                }
            }

            curl_easy_cleanup(curl);

            uw->mDownloadComplete = true;

            if (!uw->mStoreInMemory)
            {
                long fileSize;
                char *buffer;
                // Obtain file size.
                fseek(outfile, 0, SEEK_END);
                fileSize = ftell(outfile);
                rewind(outfile);
                buffer = (char*)malloc(fileSize);
                fread(buffer, 1, fileSize, outfile);
                fclose(outfile);

                // Give the file the proper name
                std::string newName(uw->mBasePath + "/updates/" +
                                    uw->mCurrentFile.c_str());

                // Any existing file with this name is deleted first, otherwise the
                // rename will fail on Windows.
                ::remove(newName.c_str());
                ::rename(outFilename.c_str(), newName.c_str());

                // Don't check resources2.txt checksum
                if (uw->mDownloadStatus == UPDATE_RESOURCES)
                {
                    // Calculate Adler-32 checksum
                    unsigned long adler = adler32(0L, Z_NULL, 0);
                    adler = adler32(adler, (Bytef *)buffer, fileSize);
                    free(buffer);

                    if (uw->mCurrentChecksum != adler) {
                        uw->mDownloadComplete = false;
                        // Remove the corrupted file
                        ::remove(newName.c_str());
                        logger->log(
                            "Checksum for file %s failed: (%lx/%lx)",
                            uw->mCurrentFile.c_str(),
                            adler, uw->mCurrentChecksum);
                    }
                }

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

    if (mThread == NULL)
    {
        logger->log("Unable to create mThread");
        mDownloadStatus = UPDATE_ERROR;
    }
}

void UpdaterWindow::logic()
{
    // Update Scroll logic
    mScrollArea->logic();

    switch (mDownloadStatus)
    {
        case UPDATE_ERROR:
            if (mThread)
            {
                if(mUserCancel){
                    // Kill the thread, because user has canceled
                    SDL_KillThread(mThread);
                    // Set the flag to false again
                    mUserCancel = false;
                }
                else{
                    SDL_WaitThread(mThread, NULL);
                }
                mThread = NULL;
            }
            mBrowserBox->addRow("");
            mBrowserBox->addRow("##1  The update process is incomplete.");
            mBrowserBox->addRow("##1  It is strongly recommended that");
            mBrowserBox->addRow("##1  you try again later");
            mBrowserBox->addRow(mCurlError);
            mScrollArea->setVerticalScrollAmount(mScrollArea->getVerticalMaxScroll());
            mDownloadStatus = UPDATE_COMPLETE;
            break;
        case UPDATE_NEWS:
            if (mDownloadComplete)
            {
                // Parse current memory buffer as news and dispose of the data
                loadNews();

                mCurrentFile = "resources2.txt";
                mStoreInMemory = false;
                download();
                mDownloadStatus = UPDATE_LIST;
            }
            break;
        case UPDATE_LIST:
            if (mDownloadComplete)
            {
                ResourceManager *resman = ResourceManager::getInstance();
                mLines = resman->loadTextFile("updates/resources2.txt");
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
                            (mBasePath + "/updates/" + mCurrentFile).c_str());
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
            setLabel("Completed");
            break;
        case UPDATE_IDLE:
            break;
    }
}

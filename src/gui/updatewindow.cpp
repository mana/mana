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

#include "gui/sdlinput.h"

#include "configuration.h"
#include "log.h"
#include "main.h"

#include "net/download.h"

#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"

#include <iostream>

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
    mDownloadStatus(UPDATE_NEWS),
    mUpdateHost(updateHost),
    mUpdatesDir(updatesDir),
    mCurrentFile("news.txt"),
    mDownloadProgress(0.0f),
    mCurrentChecksum(0),
    mStoreInMemory(true),
    mDownloadComplete(true),
    mUserCancel(false),
    mDownloadedBytes(0),
    mMemoryBuffer(NULL),
    mDownload(NULL),
    mLineIndex(0)
{
    mBrowserBox = new BrowserBox;
    mScrollArea = new ScrollArea(mBrowserBox);
    mLabel = new Label(_("Connecting..."));
    mProgressBar = new ProgressBar(0.0, 310, 20, gcn::Color(168, 116, 31));
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mPlayButton = new Button(_("Play"), "play", this);

    mProgressBar->setSmoothProgress(false);
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

    addKeyListener(this);

    center();
    setVisible(true);
    mCancelButton->requestFocus();

    // Try to download the updates list
    download();
}

UpdaterWindow::~UpdaterWindow()
{
    if (mDownload)
    {
        mDownload->cancel();
    }
    free(mMemoryBuffer);
}

void UpdaterWindow::setProgress(float p)
{
    // Do delayed progress bar update, since Guichan isn't thread-safe
    MutexLocker lock(&mDownloadMutex);
    mDownloadProgress = p;
}

void UpdaterWindow::setLabel(const std::string &str)
{
    // Do delayed label text update, since Guichan isn't thread-safe
    MutexLocker lock(&mDownloadMutex);
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
            mDownload->cancel();
            mDownloadStatus = UPDATE_ERROR;
        }
    }
    else if (event.getId() == "play")
    {
        state = STATE_LOAD_DATA;
    }
}

void UpdaterWindow::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        action(gcn::ActionEvent(NULL, mCancelButton->getActionEventId()));
        state = STATE_WORLD_SELECT;
    }
    else if (key.getValue() == Key::ENTER)
    {
        if (mDownloadStatus == UPDATE_COMPLETE ||
                mDownloadStatus == UPDATE_ERROR)
        {
            action(gcn::ActionEvent(NULL, mPlayButton->getActionEventId()));
        }
        else
        {
            action(gcn::ActionEvent(NULL, mCancelButton->getActionEventId()));
        }
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

int UpdaterWindow::updateProgress(void *ptr, DownloadStatus status,
                                  size_t dt, size_t dn)
{
    UpdaterWindow *uw = reinterpret_cast<UpdaterWindow *>(ptr);

    if (status == DOWNLOAD_STATUS_COMPLETE)
    {
        uw->mDownloadComplete = true;
    }
    else if (status == DOWNLOAD_STATUS_ERROR ||
             status == DOWNLOAD_STATUS_CANCELLED)
    {
        uw->mDownloadStatus = UPDATE_ERROR;
    }

    float progress = (float) dn / dt;

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

size_t UpdaterWindow::memoryWrite(void *ptr, size_t size, size_t nmemb, void *stream)
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

void UpdaterWindow::download()
{
    mDownload = new Net::Download(this, mUpdateHost + "/" + mCurrentFile,
                                  updateProgress);

    if (mStoreInMemory)
    {
        mDownload->setWriteFunction(UpdaterWindow::memoryWrite);
    }
    else
    {
        if (mDownloadStatus == UPDATE_RESOURCES)
        {
            mDownload->setFile(mUpdatesDir + "/" + mCurrentFile, mCurrentChecksum);
        }
        else
        {
            mDownload->setFile(mUpdatesDir + "/" + mCurrentFile);
        }
    }

    if (mDownloadStatus != UPDATE_RESOURCES)
    {
        mDownload->noCache();
    }

    setLabel(mCurrentFile + " (0%)");
    mDownloadComplete = false;

    // TODO: check return
    mDownload->start();
}

void UpdaterWindow::logic()
{
    // Update Scroll logic
    mScrollArea->logic();

    // Synchronize label caption when necessary
    {
        MutexLocker lock(&mDownloadMutex);

        if (mLabel->getCaption() != mNewLabelCaption)
        {
            mLabel->setCaption(mNewLabelCaption);
            mLabel->adjustSize();
        }

        mProgressBar->setProgress(mDownloadProgress);
    }

    std::string filename = mUpdatesDir + "/" + mCurrentFile;

    switch (mDownloadStatus)
    {
        case UPDATE_ERROR:
            // TODO: Only send complete sentences to gettext
            mBrowserBox->addRow("");
            mBrowserBox->addRow(_("##1  The update process is incomplete."));
            // TRANSLATORS: Continues "you try again later.".
            mBrowserBox->addRow(_("##1  It is strongly recommended that"));
            // TRANSLATORS: Begins "It is strongly recommended that".
            mBrowserBox->addRow(_("##1  you try again later."));

            mBrowserBox->addRow(mDownload->getError());
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

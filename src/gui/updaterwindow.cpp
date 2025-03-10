/*
 *  The Mana Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#include "gui/updaterwindow.h"

#include "client.h"
#include "configuration.h"
#include "log.h"

#include "gui/sdlinput.h"

#include "gui/widgets/browserbox.h"
#include "gui/widgets/button.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layout.h"
#include "gui/widgets/progressbar.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/itemlinkhandler.h"

#include "net/download.h"

#include "resources/resourcemanager.h"

#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/xml.h"

#include <iostream>
#include <fstream>

constexpr char xmlUpdateFile[] = "resources.xml";
constexpr char txtUpdateFile[] = "resources2.txt";

/**
 * Load the given file into a vector of updateFiles.
 */
std::vector<UpdateFile> loadXMLFile(const std::string &fileName)
{
    std::vector<UpdateFile> files;
    XML::Document doc(fileName, false);
    XML::Node rootNode = doc.rootNode();

    if (!rootNode || rootNode.name() != "updates")
    {
        logger->log("Error loading update file: %s", fileName.c_str());
        return files;
    }

    for (auto fileNode : rootNode.children())
    {
        // Ignore all tags except for the "update" tags
        if (fileNode.name() != "update")
            continue;

        UpdateFile file;
        file.name = fileNode.getProperty("file", std::string());
        file.hash = fileNode.getProperty("hash", std::string());
        file.type = fileNode.getProperty("type", "data");
        file.desc = fileNode.getProperty("description", std::string());
        file.required = fileNode.getProperty("required", "yes") == "yes";

        files.push_back(file);
    }

    return files;
}

std::vector<UpdateFile> loadTxtFile(const std::string &fileName)
{
    std::vector<UpdateFile> files;
    std::ifstream fileHandler;
    fileHandler.open(fileName, std::ios::in);

    if (fileHandler.is_open())
    {
        while (fileHandler.good())
        {
            char name[256];
            char hash[50];
            fileHandler.getline(name, 256, ' ');
            fileHandler.getline(hash, 50);

            UpdateFile thisFile;
            thisFile.name = name;
            thisFile.hash = hash;
            thisFile.type = "data";
            thisFile.required = true;

            if (!thisFile.name.empty())
                files.push_back(thisFile);
        }
    }
    else
    {
        logger->log("Error loading update file: %s", fileName.c_str());
    }
    fileHandler.close();

    return files;
}

UpdaterWindow::UpdaterWindow(const std::string &updateHost,
                             const std::string &updatesDir,
                             bool applyUpdates):
    Window(_("Updating...")),
    mUpdateHost(updateHost),
    mUpdatesDir(updatesDir),
    mCurrentFile("news.txt"),
    mLoadUpdates(applyUpdates),
    mLinkHandler(std::make_unique<ItemLinkHandler>(this))
{
    setWindowName("UpdaterWindow");
    setResizable(true);
    setDefaultSize(450, 400, ImageRect::CENTER);
    setMinWidth(320);
    setMinHeight(240);

    mBrowserBox = new BrowserBox(BrowserBox::AUTO_WRAP);
    mScrollArea = new ScrollArea(mBrowserBox);
    mLabel = new Label(_("Connecting..."));
    mProgressBar = new ProgressBar(0.0, 310, 20);
    mCancelButton = new Button(_("Cancel"), "cancel", this);
    mPlayButton = new Button(_("Play"), "play", this);

    mBrowserBox->setLinkHandler(mLinkHandler.get());
    mBrowserBox->setFrameSize(4);
    mProgressBar->setSmoothProgress(false);
    mScrollArea->setHorizontalScrollPolicy(gcn::ScrollArea::SHOW_NEVER);
    mPlayButton->setEnabled(false);

    place(0, 0, mScrollArea, 5, 3).setPadding(3);
    place(0, 3, mLabel, 5);
    place(0, 4, mProgressBar, 5);
    place(3, 5, mCancelButton);
    place(4, 5, mPlayButton);

    Layout &layout = getLayout();
    layout.setRowHeight(0, Layout::AUTO_SET);

    addKeyListener(this);

    loadWindowState();
    setVisible(true);
    mCancelButton->requestFocus();

    // Try to download the updates list
    download();
}

UpdaterWindow::~UpdaterWindow()
{
    if (mLoadUpdates)
        loadUpdates();

    if (mDownload)
    {
        mDownload->cancel();

        // Make sure thread is gone before freeing the memory buffer
        mDownload.reset();
    }

    free(mMemoryBuffer);
}

void UpdaterWindow::setProgress(float progress)
{
    // Do delayed progress bar update, since Guichan isn't thread-safe
    MutexLocker lock(&mDownloadMutex);
    mDownloadProgress = progress;
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
        Client::setState(STATE_LOAD_DATA);
    }
}

void UpdaterWindow::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        action(gcn::ActionEvent(nullptr, mCancelButton->getActionEventId()));
        Client::setState(STATE_WORLD_SELECT);
    }
    else if (key.getValue() == Key::ENTER)
    {
        if (mDownloadStatus == UPDATE_COMPLETE ||
                mDownloadStatus == UPDATE_ERROR)
        {
            action(gcn::ActionEvent(nullptr, mPlayButton->getActionEventId()));
        }
        else
        {
            action(gcn::ActionEvent(nullptr, mCancelButton->getActionEventId()));
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

    mBrowserBox->clearRows();
    mBrowserBox->addRows(std::string_view(mMemoryBuffer, mDownloadedBytes));

    // Free the memory buffer now that we don't need it anymore
    free(mMemoryBuffer);
    mMemoryBuffer = nullptr;

    mScrollArea->setVerticalScrollAmount(0);
}

int UpdaterWindow::updateProgress(void *ptr, DownloadStatus status,
                                  size_t dltotal, size_t dlnow)
{
    auto *uw = reinterpret_cast<UpdaterWindow *>(ptr);

    if (status == DOWNLOAD_STATUS_COMPLETE)
    {
        uw->mDownloadComplete = true;
    }
    else if (status == DOWNLOAD_STATUS_ERROR ||
             status == DOWNLOAD_STATUS_CANCELLED)
    {
        uw->mDownloadStatus = UPDATE_ERROR;
    }

    float progress = 0.0f;
    if (dltotal > 0)
        progress = static_cast<float>(dlnow) / dltotal;

    uw->setLabel(
            uw->mCurrentFile + " (" + toString((int) (progress * 100)) + "%)");
    uw->setProgress(progress);

    if (Client::getState() != STATE_UPDATE)
    {
        // If the action was canceled return an error code to stop the mThread
        return -1;
    }

    return 0;
}

size_t UpdaterWindow::memoryWrite(char *ptr, size_t size, size_t nmemb, void *stream)
{
    auto *uw = reinterpret_cast<UpdaterWindow *>(stream);
    const size_t totalMem = size * nmemb;
    uw->mMemoryBuffer = (char*) realloc(uw->mMemoryBuffer,
                                        uw->mDownloadedBytes + totalMem);
    if (uw->mMemoryBuffer)
    {
        memcpy(uw->mMemoryBuffer + uw->mDownloadedBytes, ptr, totalMem);
        uw->mDownloadedBytes += totalMem;
    }

    return totalMem;
}

void UpdaterWindow::download()
{
    mDownload = std::make_unique<Net::Download>(this,
                                                mUpdateHost + "/" + mCurrentFile,
                                                &UpdaterWindow::updateProgress);

    if (mStoreInMemory)
    {
        mDownload->setWriteFunction(UpdaterWindow::memoryWrite);
    }
    else
    {
        std::optional<unsigned long> adler32;
        if (mDownloadStatus == UPDATE_RESOURCES)
            adler32 = mCurrentChecksum;

        mDownload->setFile(mUpdatesDir + "/" + mCurrentFile, adler32);
    }

    if (mDownloadStatus != UPDATE_RESOURCES)
        mDownload->noCache();

    setLabel(mCurrentFile + " (0%)");
    mDownloadComplete = false;

    // TODO: check return
    mDownload->start();
}

void UpdaterWindow::loadUpdates()
{
    if (mUpdateFiles.empty())
    {
        // updates not downloaded
        mUpdateFiles = loadXMLFile(mUpdatesDir + "/" + xmlUpdateFile);
        if (mUpdateFiles.empty())
        {
            logger->log("Warning this server does not have a"
                        " %s file falling back to %s", xmlUpdateFile,
                        txtUpdateFile);
            mUpdateFiles = loadTxtFile(mUpdatesDir + "/" + txtUpdateFile);
        }
    }

    for (const UpdateFile &file : mUpdateFiles)
        ResourceManager::addToSearchPath(mUpdatesDir + "/" + file.name, false);
}

void UpdaterWindow::logic()
{
    const std::string xmlUpdateFile = "resources.xml";
    const std::string txtUpdateFile = "resources2.txt";

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

    switch (mDownloadStatus)
    {
        case UPDATE_ERROR: {
            std::string error = "##1";
            error += mDownload->getError();
            error += "\n\n";
            error += _("The update process is incomplete. "
                       "It is strongly recommended that you try again later.");
            mBrowserBox->addRows(error);

            mScrollArea->setVerticalScrollAmount(
                    mScrollArea->getVerticalMaxScroll());
            mDownloadStatus = UPDATE_COMPLETE;
            break;
        }
        case UPDATE_NEWS:
            if (mDownloadComplete)
            {
                // Parse current memory buffer as news and dispose of the data
                loadNews();

                mCurrentFile = xmlUpdateFile;
                mStoreInMemory = false;
                mDownloadStatus = UPDATE_LIST;
                download(); // download() changes mDownloadComplete to false
            }
            break;
        case UPDATE_LIST:
            if (mDownloadComplete)
            {
                if (mCurrentFile == xmlUpdateFile)
                {
                    mUpdateFiles = loadXMLFile(mUpdatesDir + "/" + xmlUpdateFile);
                    if (mUpdateFiles.empty())
                    {
                        logger->log("Warning this server does not have a %s"
                                    " file falling back to %s",
                                    xmlUpdateFile.c_str(), txtUpdateFile.c_str());

                        // If the resources.xml file fails, fall back onto a older version
                        mCurrentFile = txtUpdateFile;
                        mStoreInMemory = false;
                        mDownloadStatus = UPDATE_LIST;
                        download();
                        break;
                    }
                }
                else if (mCurrentFile == txtUpdateFile)
                {
                    mUpdateFiles = loadTxtFile(mUpdatesDir + "/" + txtUpdateFile);
                }
                mStoreInMemory = false;
                mDownloadStatus = UPDATE_RESOURCES;
            }
            break;
        case UPDATE_RESOURCES:
            if (mDownloadComplete)
            {
                if (mUpdateIndex < mUpdateFiles.size())
                {
                    const UpdateFile &thisFile = mUpdateFiles[mUpdateIndex];
                    if (!thisFile.required)
                    {
                        if (!(thisFile.type == "music" && config.downloadMusic))
                        {
                            mUpdateIndex++;
                            break;
                        }
                    }
                    mCurrentFile = thisFile.name;
                    std::stringstream ss(thisFile.hash);
                    ss >> std::hex >> mCurrentChecksum;

                    std::string filename = mUpdatesDir + "/" + mCurrentFile;
                    FILE *file = fopen(filename.c_str(), "r+b");

                    if (!file || Net::Download::fadler32(file) != mCurrentChecksum)
                    {
                        if (file)
                            fclose(file);
                        download();
                    }
                    else
                    {
                        fclose(file);
                        logger->log("%s already here", mCurrentFile.c_str());
                    }
                    mUpdateIndex++;
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
            mDownloadStatus = UPDATE_IDLE;
            break;
        case UPDATE_IDLE:
            break;
    }
}

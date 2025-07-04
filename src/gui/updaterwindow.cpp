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
        Log::info("Error loading update file: %s", fileName.c_str());
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
        Log::info("Error loading update file: %s", fileName.c_str());
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

    startDownload("news.txt", true);
}

UpdaterWindow::~UpdaterWindow()
{
    if (mLoadUpdates)
        loadUpdates();
}

void UpdaterWindow::setLabel(const std::string &str)
{
    mLabel->setCaption(str);
    mLabel->adjustSize();
}

void UpdaterWindow::enablePlay()
{
    mCancelButton->setEnabled(false);
    mPlayButton->setEnabled(true);
    mPlayButton->requestFocus();
}

void UpdaterWindow::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "cancel")
        cancel();
    else if (event.getId() == "play")
        play();
}

void UpdaterWindow::keyPressed(gcn::KeyEvent &keyEvent)
{
    gcn::Key key = keyEvent.getKey();

    if (key.getValue() == Key::ESCAPE)
    {
        if (!cancel())
        {
            mLoadUpdates = false;
            Client::setState(STATE_WORLD_SELECT);
        }
    }
    else if (key.getValue() == Key::ENTER)
    {
        play();
    }
}

bool UpdaterWindow::cancel()
{
    // Skip the updating process
    if (mDialogState != DialogState::Done)
    {
        mDownload->cancel();
        return true;
    }
    return false;
}

void UpdaterWindow::play()
{
    if (mPlayButton->isEnabled())
        Client::setState(STATE_LOAD_DATA);
}

void UpdaterWindow::loadNews()
{
    mBrowserBox->clearRows();
    mBrowserBox->addRows(mDownload->getBuffer());

    mScrollArea->setVerticalScrollAmount(0);
}

void UpdaterWindow::startDownload(const std::string &fileName,
                                  bool storeInMemory,
                                  std::optional<unsigned long> adler32)
{
    mDownload = std::make_unique<Net::Download>(mUpdateHost + "/" + fileName);
    mCurrentFile = fileName;

    if (storeInMemory)
        mDownload->setUseBuffer();
    else
        mDownload->setFile(mUpdatesDir + "/" + fileName, adler32);

    if (mDialogState != DialogState::DownloadResources)
        mDownload->noCache();

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
            Log::warn("This server does not have a"
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
    Window::logic();

    if (mDialogState == DialogState::Done)
        return;

    const auto state = mDownload->getState();
    float progress = 0.0f;

    switch (state.status) {
    case DownloadStatus::InProgress: {
        setLabel(mCurrentFile + " (" + toString((int) (state.progress * 100)) + "%)");
        progress = state.progress;
        break;
    }

    case DownloadStatus::Canceled:
        mDialogState = DialogState::Done;

        enablePlay();
        setLabel(_("Download canceled"));
        break;

    case DownloadStatus::Error: {
        mDialogState = DialogState::Done;

        std::string error = "##1";
        error += mDownload->getError();
        error += "\n\n##1";
        error += _("The update process is incomplete. "
                   "It is strongly recommended that you try again later.");
        mBrowserBox->addRows(error);

        int maxScroll = mScrollArea->getVerticalMaxScroll();
        mScrollArea->setVerticalScrollAmount(maxScroll);

        enablePlay();
        setLabel(_("Error while downloading"));
        break;
    }

    case DownloadStatus::Complete:
        downloadCompleted();
        break;
    }

    mProgressBar->setProgress(progress);
}

void UpdaterWindow::downloadCompleted()
{
    switch (mDialogState)
    {
    case DialogState::DownloadNews:
        loadNews();

        mDialogState = DialogState::DownloadList;
        startDownload(xmlUpdateFile, false);
        break;

    case DialogState::DownloadList:
        if (mCurrentFile == xmlUpdateFile)
        {
            mUpdateFiles = loadXMLFile(mUpdatesDir + "/" + xmlUpdateFile);
            if (mUpdateFiles.empty())
            {
                Log::warn("This server does not have a %s"
                          " file falling back to %s",
                          xmlUpdateFile, txtUpdateFile);

                // If the resources.xml file fails, fall back onto a older version
                mDialogState = DialogState::DownloadList;
                startDownload(txtUpdateFile, false);
                break;
            }
        }
        else if (mCurrentFile == txtUpdateFile)
        {
            mUpdateFiles = loadTxtFile(mUpdatesDir + "/" + txtUpdateFile);
        }

        mDialogState = DialogState::DownloadResources;
        break;

    case DialogState::DownloadResources:
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

            unsigned long checksum;
            std::stringstream ss(thisFile.hash);
            ss >> std::hex >> checksum;

            std::string filename = mUpdatesDir + "/" + thisFile.name;
            FILE *file = fopen(filename.c_str(), "r+b");

            if (!file || Net::Download::fadler32(file) != checksum)
            {
                if (file)
                    fclose(file);
                startDownload(thisFile.name, false, checksum);
            }
            else
            {
                fclose(file);
                Log::info("%s already here", thisFile.name.c_str());
            }
            mUpdateIndex++;
        }
        else
        {
            // Download of updates completed
            mDialogState = DialogState::Done;
            enablePlay();
            setLabel(_("Completed"));
        }
        break;

    case DialogState::Done:
        break;
    }
}

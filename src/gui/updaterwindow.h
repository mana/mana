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

#pragma once

#include "gui/widgets/window.h"

#include "net/download.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include <memory>
#include <string>
#include <vector>

class LinkHandler;
class BrowserBox;
class Button;
class ProgressBar;
class ScrollArea;

struct UpdateFile
{
    std::string name;
    std::string hash;
    std::string type;
    bool required;
    std::string desc;
};

/**
 * Update progress window GUI
 *
 * \ingroup GUI
 */
class UpdaterWindow : public Window, public gcn::ActionListener,
                      public gcn::KeyListener
{
 public:
    /**
     * Constructor.
     *
     * @param updateHost Host where to get the updated files.
     * @param updatesDir Directory where to store updates (should be absolute
     *                   and already created).
     * @param applyUpdates If true, the update window will pass the updates to the
     *                     resource manager
     */
    UpdaterWindow(const std::string &updateHost,
                  const std::string &updatesDir,
                  bool applyUpdates);

    ~UpdaterWindow() override;

    void action(const gcn::ActionEvent &event) override;

    void keyPressed(gcn::KeyEvent &keyEvent) override;

    void logic() override;

private:
    bool cancel();
    void play();

    void setLabel(const std::string &);
    void enablePlay();

    void startDownload(const std::string &fileName,
                       bool storeInMemory,
                       std::optional<unsigned long> adler32 = {});
    void downloadCompleted();

    /**
     * Loads and display news. Assumes the news file contents have been loaded
     * into the memory buffer.
     */
    void loadNews();

    /**
     * Loads the updates this window has gotten into the resource manager
     */
    void loadUpdates();

    enum class DialogState
    {
        DOWNLOAD_NEWS,
        DOWNLOAD_LIST,
        DOWNLOAD_RESOURCES,
        DONE,
    };

    /** Status of the current download. */
    DialogState mDialogState = DialogState::DOWNLOAD_NEWS;

    /** Host where we get the updated files. */
    std::string mUpdateHost;

    /** Place where the updates are stored (absolute path). */
    std::string mUpdatesDir;

    /** The file currently downloading. */
    std::string mCurrentFile;

    /** Download handle. */
    std::unique_ptr<Net::Download> mDownload;

    /** List of files to download. */
    std::vector<UpdateFile> mUpdateFiles;

    /** Index of the file to be downloaded. */
    unsigned int mUpdateIndex = 0;

    /** Tells ~UpdaterWindow() if it should load updates */
    bool mLoadUpdates;

    gcn::Label *mLabel;           /**< Progress bar caption. */
    Button *mCancelButton;        /**< Button to stop the update process. */
    Button *mPlayButton;          /**< Button to start playing. */
    ProgressBar *mProgressBar;    /**< Update progress bar. */
    BrowserBox *mBrowserBox;      /**< Box to display news. */
    ScrollArea *mScrollArea;      /**< Used to scroll news box. */
    std::unique_ptr<LinkHandler> mLinkHandler;
};

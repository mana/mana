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

#ifndef UPDATERWINDOW_H
#define UPDATERWINDOW_H

#include "gui/widgets/window.h"

#include "net/download.h"

#include "utils/mutex.h"

#include <guichan/actionlistener.hpp>
#include <guichan/keylistener.hpp>

#include <string>
#include <vector>

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
     * @param applyUpdates If true, the update window will pass the updates to teh
     *                     resource manager
     */
    UpdaterWindow(const std::string &updateHost,
                  const std::string &updatesDir,
                  bool applyUpdates);

    ~UpdaterWindow() override;

    /**
     * Set's progress bar status
     */
    void setProgress(float progress);

    /**
     * Set's label above progress
     */
    void setLabel(const std::string &);

    /**
     * Enables play button
     */
    void enable();

    /**
     * Loads and display news. Assumes the news file contents have been loaded
     * into the memory buffer.
     */
    void loadNews();

    void action(const gcn::ActionEvent &event) override;

    void keyPressed(gcn::KeyEvent &keyEvent) override;

    void logic() override;

    int updateState;

private:
    void download();

    /**
     * Loads the updates this window has gotten into the resource manager
     */
    void loadUpdates();


    /**
     * A download callback for progress updates.
     */
    static int updateProgress(void *ptr, DownloadStatus status,
                              size_t dt, size_t dn);

    /**
     * A libcurl callback for writing to memory.
     */
    static size_t memoryWrite(void *ptr, size_t size, size_t nmemb,
                              void *stream);

    enum UpdateDownloadStatus
    {
        UPDATE_ERROR,
        UPDATE_IDLE,
        UPDATE_LIST,
        UPDATE_COMPLETE,
        UPDATE_NEWS,
        UPDATE_RESOURCES
    };

    /** Status of the current download. */
    UpdateDownloadStatus mDownloadStatus = UPDATE_NEWS;

    /** Host where we get the updated files. */
    std::string mUpdateHost;

    /** Place where the updates are stored (absolute path). */
    std::string mUpdatesDir;

    /** The file currently downloading. */
    std::string mCurrentFile;

    /** The new label caption to be set in the logic method. */
    std::string mNewLabelCaption;

    /** The new progress value to be set in the logic method. */
    float mDownloadProgress = 0.0f;

    /** The mutex used to guard access to mNewLabelCaption and mDownloadProgress. */
    Mutex mDownloadMutex;

    /** The Adler32 checksum of the file currently downloading. */
    unsigned long mCurrentChecksum = 0;

    /** A flag to indicate whether to use a memory buffer or a regular file. */
    bool mStoreInMemory = true;

    /** Flag that show if current download is complete. */
    bool mDownloadComplete = true;

    /** Flag that show if the user has canceled the update. */
    bool mUserCancel = false;

    /** Byte count currently downloaded in mMemoryBuffer. */
    int mDownloadedBytes = 0;

    /** Buffer for files downloaded to memory. */
    char *mMemoryBuffer = nullptr;

    /** Download handle. */
    Net::Download *mDownload = nullptr;

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
};

#endif

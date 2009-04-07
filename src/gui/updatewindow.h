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

#ifndef _UPDATERWINDOW_H
#define _UPDATERWINDOW_H

#include "gui/widgets/window.h"

#include "utils/mutex.h"

#include <guichan/actionlistener.hpp>

#include <string>
#include <vector>

class BrowserBox;
class Button;
class ProgressBar;
class ScrollArea;

struct SDL_Thread;

/**
 * Update progress window GUI
 *
 * \ingroup GUI
 */
class UpdaterWindow : public Window, public gcn::ActionListener
{
 public:
    /**
     * Constructor.
     *
     * @param updateHost Host where to get the updated files.
     * @param updatesDir Directory where to store updates (should be absolute
     *                   and already created).
     */
    UpdaterWindow(const std::string &updateHost,
                  const std::string &updatesDir);

    /**
     * Destructor
     */
    ~UpdaterWindow();

    /**
     * Set's progress bar status
     */
    void setProgress(float p);

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

    void action(const gcn::ActionEvent &event);

    void logic();

    int updateState;

private:
    void download();

    /**
     * The thread function that download the files.
     */
    static int downloadThread(void *ptr);

    /**
     * A libcurl callback for progress updates.
     */
    static int updateProgress(void *ptr,
                              double dt, double dn, double ut, double un);

    /**
     * A libcurl callback for writing to memory.
     */
    static size_t memoryWrite(void *ptr, size_t size, size_t nmemb,
                              FILE *stream);

    enum DownloadStatus
    {
        UPDATE_ERROR,
        UPDATE_IDLE,
        UPDATE_LIST,
        UPDATE_COMPLETE,
        UPDATE_NEWS,
        UPDATE_RESOURCES
    };

    /** A thread that use libcurl to download updates. */
    SDL_Thread *mThread;

    /** Status of the current download. */
    DownloadStatus mDownloadStatus;

    /** Host where we get the updated files. */
    std::string mUpdateHost;

    /** Place where the updates are stored (absolute path). */
    std::string mUpdatesDir;

    /** The file currently downloading. */
    std::string mCurrentFile;

    /** The new label caption to be set in the logic method. */
    std::string mNewLabelCaption;

    /** The mutex used to guard access to mNewLabelCaption. */
    Mutex mLabelMutex;

    /** The Adler32 checksum of the file currently downloading. */
    unsigned long mCurrentChecksum;

    /** A flag to indicate whether to use a memory buffer or a regular file. */
    bool mStoreInMemory;

    /** Flag that show if current download is complete. */
    bool mDownloadComplete;

    /** Flag that show if the user has canceled the update. */
    bool mUserCancel;

    /** Byte count currently downloaded in mMemoryBuffer. */
    int mDownloadedBytes;

    /** Buffer for files downloaded to memory. */
    char *mMemoryBuffer;

    /** Buffer to handler human readable error provided by curl. */
    char *mCurlError;

    /** List of files to download. */
    std::vector<std::string> mLines;

    /** Index of the file to be downloaded. */
    unsigned int mLineIndex;

    gcn::Label *mLabel;           /**< Progress bar caption. */
    Button *mCancelButton;        /**< Button to stop the update process. */
    Button *mPlayButton;          /**< Button to start playing. */
    ProgressBar *mProgressBar;    /**< Update progress bar. */
    BrowserBox *mBrowserBox;      /**< Box to display news. */
    ScrollArea *mScrollArea;      /**< Used to scroll news box. */
};

#endif

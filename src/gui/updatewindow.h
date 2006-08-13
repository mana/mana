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

#ifndef _UPDATERWINDOW_H
#define _UPDATERWINDOW_H

#include <guichan/actionlistener.hpp>
#include <string>
#include <vector>

#include "window.h"

#include "../guichanfwd.h"

class BrowserBox;
class Button;
class ProgressBar;
class ScrollArea;

struct SDL_mutex;
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
     * Constructor
     */
    UpdaterWindow();

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

    void action(const std::string& eventId, gcn::Widget* widget);

    /**
     * Add a row to the message field.
     */
    void addRow(const std::string &row);

    void logic();

    int updateState;

 protected:
    void download();

    /**
     * The tread function that download the files.
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

    /**
     * A thread that use libcurl to download updates.
     */
    SDL_Thread *mThread;

    /**
     * A mutex to protect shared data between the threads.
     */
    SDL_mutex *mMutex;

    /**
     * Status of the current download.
     */
    DownloadStatus mDownloadStatus;

    /**
     * Host where we get the updated files.
     */
    std::string mUpdateHost;

    /**
     * The file currently downloading.
     */
    std::string mCurrentFile;

    /**
     * Absolute path to locally save downloaded files.
     */
    std::string mBasePath;

    /**
     * A flag to know if we must write the downloaded file to a memory buffer
     * instead of a regular file.
     */
    bool mStoreInMemory;

    /**
     * Flag that show if current download is complete.
     */
    bool mDownloadComplete;

    /**
     * Byte count currently downloaded in mMemoryBuffer.
     */
    int mDownloadedBytes;

    /**
     * Buffer where to put downloaded file which are not stored in file system.
     */
    char *mMemoryBuffer;

    /**
     * Buffer to handler human readable error provided by curl.
     */
    char *mCurlError;

    /**
     * List of files to download
     */
    std::vector<std::string> mFiles;

    /**
     * Index of the file to be downloaded
     */
    unsigned int mFileIndex;

    gcn::Label *mLabel;           /**< Progress bar caption. */
    Button *mCancelButton;        /**< Button to stop the update process. */
    Button *mPlayButton;          /**< Button to start playing. */
    ProgressBar *mProgressBar;    /**< Update progress bar. */
    BrowserBox* mBrowserBox;      /**< Box to display news. */
    ScrollArea *mScrollArea;      /**< Used to scroll news box. */
};

#endif

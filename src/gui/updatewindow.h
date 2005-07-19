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

#include "window.h"
#include "vbox.h"
#include "progressbar.h"
#include "button.h"
#include "browserbox.h"
#include "scrollarea.h"

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
    UpdaterWindow(const std::string& updateHost = "themanaworld.org/files");

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
     * Loads and display news
     */
    void loadNews();

    void action(const std::string& eventId);

    /**
     * Add a row to the message field.
     */
    void addRow(const std::string &row);

    void updateData();

    int updateState;

 protected:
    
    void download();

    /*
     * The tread function that download the files
     */
    static int downloadThread(void *ptr);

    /*
     * A libcurl callback
     */
    static int updateProgress(void *ptr, double dt, double dn, double ut, double un);

    /*
     * A libcurl callback
     */
    static size_t memoryWrite(void *ptr, size_t size, size_t nmemb, FILE *stream);

    enum DownloadStatus
    {
        UPDATE_ERROR,
        UPDATE_IDLE,
        UPDATE_LIST,
        UPDATE_COMPLETE,
        UPDATE_NEWS,
        UPDATE_RESOURCES
    };

    /*
     * A thread that use libcurl to download updates
     */
    class SDL_Thread *m_thread;

    /*
     * A mutex to protect shared data betwed the threads
     */
    class SDL_mutex *m_mutex;


    /*
     * Status of the current download
     */
    DownloadStatus m_downloadStatus;

    /*
     * host where we get the updated files
     */
    std::string m_updateHost;

    /*
     * the file currently downloading
     */
    std::string m_currentFile;

    /*
     * Absolute path to locally save downloaded files
     */
    std::string m_basePath;

    /*
     * A flag to know if we must write the downloaded file
     * in m_memoryBuffer instead of a regular file
     */
    bool m_storeInMemory;

    /*
     * flag that show if current download is complete
     */
    bool m_downloadComplete;

    /*
     * byte count currently downloaded in m_memoryBuffer
     */
    int m_downloadedBytes;

    /*
     * buffer where to put downloaded file which are
     * not stored in file system
     */
    char *m_memoryBuffer;

    std::string labelText;       /**< Text for caption label */

    gcn::Label *label;           /**< Progress bar caption */
    Button *cancelButton;        /**< Button to stop the update process */
    Button *playButton;          /**< Button to start playing */
    ProgressBar *progressBar;    /**< Update progress bar */
    BrowserBox* browserBox;      /**< Box to display news */
    ScrollArea *scrollArea;      /**< Used to scroll news box */

};

void updateData();

#endif

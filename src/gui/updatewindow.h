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

enum {
    UPDATE_ERROR,
    UPDATE_IDLE,
    UPDATE_RUN,
    UPDATE_COMPLETE,
    UPDATE_NEWS,
    UPDATE_RESOURCES
};

/**
 * Update progress window GUI
 *
 * \ingroup GUI
 */
class UpdaterWindow : public Window, public gcn::ActionListener
{
 protected:
    std::string labelText;       /**< Text for caption label */

    gcn::Label *label;           /**< Progress bar caption */
    Button *cancelButton;        /**< Button to stop the update process */
    Button *playButton;          /**< Button to start playing */
    ProgressBar *progressBar;    /**< Update progress bar */
    BrowserBox* browserBox;      /**< Box to display news */
    ScrollArea *scrollArea;      /**< Used to scroll news box */

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
     * Loads and display news
     */
    void loadNews();
    
    void action(const std::string& eventId);

    void draw(gcn::Graphics *);
    
    int updateState;
};

void updateData();
#endif

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

#ifndef _TMW_HELP_H
#define _TMW_HELP_H

#include <guichan/actionlistener.hpp>

#include "window.h"
#include "linkhandler.h"

#include "../guichanfwd.h"

class BrowserBox;

/**
 * The help dialog.
 */
class HelpWindow : public Window, public LinkHandler,
                   public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         */
        HelpWindow();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const std::string& eventId);

        /**
         * Handles link action.
         */
        void handleLink(const std::string& link);

        /**
         * Loads help in the dialog.
         */
        void loadHelp(const std::string &helpFile);

    private:
        void loadFile(const std::string &file);

        BrowserBox* mBrowserBox;
        gcn::ScrollArea *mScrollArea;
};

extern HelpWindow *helpWindow;

#endif

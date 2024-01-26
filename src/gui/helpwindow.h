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

#ifndef HELP_H
#define HELP_H

#include "gui/widgets/linkhandler.h"
#include "gui/widgets/window.h"

#include <guichan/actionlistener.hpp>

class BrowserBox;
class LinkHandler;

/**
 * The help dialog.
 */
class HelpWindow : public Window, public LinkHandler,
                   public gcn::ActionListener
{
    public:
        HelpWindow();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event) override;

        /**
         * Handles link action.
         */
        void handleLink(const std::string &link) override;

        /**
         * Loads help in the dialog.
         */
        void loadHelp(const std::string &helpFile);

    private:
        void loadFile(const std::string &file);

        BrowserBox *mBrowserBox;
        gcn::ScrollArea *mScrollArea;
};

extern HelpWindow *helpWindow;

#endif

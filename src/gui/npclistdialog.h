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
 */

#ifndef _TMW_GUI_NPCLISTDIALOG_H
#define _TMW_GUI_NPCLISTDIALOG_H

#include <iosfwd>
#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include "window.h"

#include "../guichanfwd.h"

/**
 * The npc list dialog.
 *
 * \ingroup Interface
 */
class NpcListDialog : public Window, public gcn::ActionListener,
                      public gcn::ListModel
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        NpcListDialog();

        /**
         * Called when resizing the window
         *
         * @param event The calling event
         */
        void widgetResized(const gcn::Event &event);

        /**
         * Redraws the window
         */
        void draw();

        /**
         * Called when receiving actions from the widgets.
         */
        void
        action(const gcn::ActionEvent &event);

        /**
         * Returns the number of items in the choices list.
         */
        int
        getNumberOfElements();

        /**
         * Returns the name of item number i of the choices list.
         */
        std::string
        getElementAt(int i);

        /**
         * Fills the options list for an NPC dialog.
         *
         * @param itemString A string with the options separated with colons.
         */
        void
        parseItems(const std::string &itemString);

        /**
         * Resets the list by removing all items.
         */
        void
        reset();

    private:
        gcn::ListBox *mItemList;
        gcn::ScrollArea *scrollArea;
        gcn::Button *okButton;
        gcn::Button *cancelButton;

        std::vector<std::string> mItems;
};

#endif

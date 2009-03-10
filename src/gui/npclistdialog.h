/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
 *  from The Mana World.
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

#ifndef GUI_NPCLISTDIALOG_H
#define GUI_NPCLISTDIALOG_H

#include <vector>

#include <guichan/actionlistener.hpp>
#include <guichan/listmodel.hpp>

#include "window.h"

class Network;

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
        NpcListDialog(Network *network);

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the number of items in the choices list.
         */
        int getNumberOfElements();

        /**
         * Returns the name of item number i of the choices list.
         */
        std::string getElementAt(int i);

        /**
         * Fills the options list for an NPC dialog.
         *
         * @param itemString A string with the options separated with colons.
         */
        void parseItems(const std::string &itemString);

        /**
         * Resets the list by removing all items.
         */
        void reset();

        /**
         * Requests the listbox to take focus for input and sets window width
         * to the last known setting.
         */
        void requestFocus();

    private:
        Network *mNetwork;
        gcn::ListBox *mItemList;
        gcn::ScrollArea *scrollArea;
        gcn::Button *okButton;
        gcn::Button *cancelButton;

        std::vector<std::string> mItems;
};

extern NpcListDialog *npcListDialog;

#endif // GUI_NPCLISTDIALOG_H

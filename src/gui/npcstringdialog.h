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

#ifndef GUI_NPCSTRINGDIALOG_H
#define GUI_NPCSTRINGDIALOG_H

#include <guichan/actionlistener.hpp>

#include "window.h"

/**
 * The npc integer input dialog.
 *
 * \ingroup Interface
 */
class NpcStringDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        NpcStringDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the current value.
         */
        std::string getValue();

        /**
         * Chnages the current value.
         *
         * @param value The new value
         */
        void setValue(const std::string &value);

        /**
         * Checks whether NpcStringDialog is Focused or not.
         */
        bool isInputFocused();

        /**
         * Requests the textfield to take focus for input.
         */
        void requestFocus();

    private:
        gcn::TextField *mValueField;
        gcn::Button *okButton;
        gcn::Button *cancelButton;
};

extern NpcStringDialog *npcStringDialog;

#endif // GUI_NPCSTRINGDIALOG_H

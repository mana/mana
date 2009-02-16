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

#ifndef GUI_NPCINTEGERDIALOG_H
#define GUI_NPCINTEGERDIALOG_H

#include <guichan/actionlistener.hpp>

#include "window.h"

class IntTextField;

/**
 * The npc integer input dialog.
 *
 * \ingroup Interface
 */
class NpcIntegerDialog : public Window, public gcn::ActionListener
{
    public:
        /**
         * Constructor.
         *
         * @see Window::Window
         */
        NpcIntegerDialog();

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const gcn::ActionEvent &event);

        /**
         * Returns the current value.
         */
        int getValue();

        /**
         * Prepares the NPC dialog.
         *
         * @param min The minimum value to allow
         * @param max The maximum value to allow
         */
        void setRange(const int min, const int max);

        /**
         * Sets the default value.
         *
         * @param value The new default value
         */
        void setDefaultValue(const int value);

        /**
         * Checks whether NpcStringDialog is Focused or not.
         */
        bool isInputFocused();

        /**
         * Requests the textfield to take focus for input.
         */
        void requestFocus();

    private:
        gcn::Button *mDecButton;
        gcn::Button *mIncButton;
        IntTextField *mValueField;
};

#endif // GUI_NPCINTEGERDIALOG_H

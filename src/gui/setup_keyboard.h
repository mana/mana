/*
 *  The Mana World
 *  Copyright 2007 The Mana World Development Team
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
 *  $Id: setup_keyboard.h 4255 2008-05-21 21:44:27Z crush_tmw $
 */

#ifndef _TMW_GUI_SETUP_KEYBOARD_H
#define _TMW_GUI_SETUP_KEYBOARD_H

#include "setuptab.h"
#include "button.h"
#include "../guichanfwd.h"

#include <guichan/actionlistener.hpp>


#include <string>

class Setup_Keyboard : public SetupTab, public gcn::ActionListener
{
    public:
        /**
         * Constructor
         */
        Setup_Keyboard();

        /**
         * Destructor
         */
        ~Setup_Keyboard();

        void apply();
        void cancel();

        void action(const gcn::ActionEvent &event);

        /**
         * Get an update on the assigned key.
         */
        void refreshAssignedKey(int index);

        /**
         * The callback function when a new key has been pressed.
         */
        void newKeyCallback(int index);

        /**
         * Shorthand method to update all the keys.
         */
        void refreshKeys();

        /**
         * If a key function is unresolved, then this reverts it.
         */
        void keyUnresolved();

    private:
        class KeyListModel *mKeyListModel;
        gcn::ListBox *mKeyList;

        gcn::Button *mAssignKeyButton;
        gcn::Button *mMakeDefaultButton;

        bool mKeySetting; /**< flag to check if key being set. */
};

#endif

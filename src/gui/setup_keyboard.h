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
 *  $Id$
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
         * Easy way to disable/enable all the set buttons.
         */
        void enableSetButtons(bool bValue);

        /**
         * Get an update on the assigned key.
         */
        void refreshAssignedKey(const int index);

        /**
         * The callback function when a new key has been pressed.
         */
        void newKeyCallback(const int index);

        /**
         * Shorthand method to update all the keys.
         */
        void refreshKeys();

    private:
        gcn::Label *mKeyLabel;

        Button *mKeyButton;

        gcn::Button *mMakeDefaultButton;
};

#endif

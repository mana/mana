/*
 *  The Mana Client
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
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

#ifndef BEINGPOPUP_H
#define BEINGPOPUP_H

#include "gui/widgets/popup.h"

#include <guichan/mouselistener.hpp>

class TextBox;
class Being;

/**
 * A popup that displays information about an item.
 */
class BeingPopup : public Popup
{
    public:
        /**
         * Constructor. Initializes the item popup.
         */
        BeingPopup();

        /**
         * Destructor. Cleans up the item popup on deletion.
         */
        ~BeingPopup();

        /**
         * Sets the info to be displayed given a particular item.
         */
        void setBeing(int x, int y, Being *b);

        /**
         * Sets the location to display the item popup.
         */
        void view(int x, int y);

    private:
        TextBox *mBeingName;
        TextBox *mBeingParty;

        static gcn::Color getColor();
};

#endif // BEINGPOPUP_H

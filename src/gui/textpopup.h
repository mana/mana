/*
 *  The Mana World
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2008-2009  The Mana World Development Team
 *  Copyright (C) 2009-2012  The Mana Developers
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

#ifndef TEXTPOPUP_H
#define TEXTPOPUP_H

#include "gui/widgets/popup.h"

#include <guichan/mouselistener.hpp>

class TextBox;

/**
 * A popup that displays information about an item.
 */
class TextPopup : public Popup
{
    public:
        TextPopup();

        ~TextPopup();

        /**
         * Sets the text to be displayed.
         */
        void show(int x, int y, const std::string &str1)
        { show(x, y, str1, (const char*)""); }

        /**
         * Sets the text to be displayed.
         */
        void show(int x, int y, const std::string &str1, const std::string &str2);

        void mouseMoved(gcn::MouseEvent &mouseEvent);

    private:
        gcn::Label *mText1;
        gcn::Label *mText2;
};

#endif // TEXTPOPUP_H

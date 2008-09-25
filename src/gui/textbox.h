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
 *  $Id: textbox.h 4096 2008-04-16 08:39:59Z b_lindeijer $
 */

#ifndef __TMW_TEXTBOX_H__
#define __TMW_TEXTBOX_H__

#include <guichan/widgets/textbox.hpp>

/**
 * A text box, meant to be used inside a scroll area. Same as the Guichan text
 * box except this one doesn't have a background or border, instead completely
 * relying on the scroll area.
 *
 * \ingroup GUI
 */
class TextBox : public gcn::TextBox {
    public:
        /**
         * Constructor.
         */
        TextBox();

        /**
         * Sets the text after wrapping it to the current width of the widget.
         */
        void setTextWrapped(const std::string &text);

        /**
         * Get the minimum text width for the text box.
         */
        int getMinWidth() { return mMinWidth; }

        /**
         * Set the minimum text width for the text box.
         */
        void setMinWidth(int width) { mMinWidth = width; }

    private:
        int mMinWidth;
};

#endif

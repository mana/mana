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
 *  $Id$
 */

#ifndef __TMW_TEXTFIELD_H__
#define __TMW_TEXTFIELD_H__

#include <guichan.hpp>
#include "../graphics.h"

/**
 * A text field.
 *
 * \ingroup GUI
 */
class TextField : public gcn::TextField {
    public:
        /**
         * Constructor, initializes the text field with the given string.
         */
        TextField(const std::string& text = "");

        /**
         * Draws the text field.
         */
        virtual void draw(gcn::Graphics *graphics);

        /**
         * Draws the background and border.
         */
        void drawBorder(gcn::Graphics *graphics);

    private:
        ImageRect skin;
};

#endif
